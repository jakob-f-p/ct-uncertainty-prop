import csv
import json
import logging.config
import logging.handlers
import numpy
import radiomics
import threading

from datapaths import DataPaths, extraction_params_file, feature_directory
from functools import partial
from multiprocessing import cpu_count, Manager, Pool
from multiprocessing.pool import AsyncResult
from pathlib import Path
from radiomics import featureextractor
from radiomics.scripts import segment
from typing import Dict, Callable, List, OrderedDict, Tuple, cast

logger = logging.getLogger(__file__)

FeatureExtractionResult = List[OrderedDict[str, float]]


class FeatureExtraction:
    id: int
    params_file: Path
    data_files: DataPaths
    number_of_images: int
    is_multiprocessing: bool
    out_dir: Path
    log_file: Path
    logging_config: Dict
    logging_queue_listener: logging.handlers.QueueListener
    progress_callback: Callable[[float], None]

    def __init__(self, id: int, data_files: DataPaths, progress_callback: Callable[[float], None]):
        self.id = id
        self.params_file = extraction_params_file
        self.data_files = data_files
        self.number_of_images = len(data_files)
        self.is_multiprocessing = self.number_of_images > 1
        self.out_dir = feature_directory
        self.log_file = self.out_dir / "log.txt"
        self.logging_config, self.logging_queue_listener = self.get_logging_config()
        self.progress_callback = progress_callback

    def run(self) -> FeatureExtractionResult:
        results = []
        try:
            logger.info("Starting PyRadiomics (version: %s)", radiomics.__version__)
            cases = self.get_cases()
            results = self.extract_features(cases)
            self.write_outputs(results)
            logger.info("Finished extraction successfully...")
        except (KeyboardInterrupt, SystemExit):
            logger.info("Cancelling Extraction")
        finally:
            if self.logging_queue_listener is not None:
                for item in self.logging_queue_listener.handlers:
                    if isinstance(item, logging.Handler):
                        handler = cast(logging.Handler, item)
                        handler.close()
                self.logging_queue_listener.stop()

        result_feature_dicts: FeatureExtractionResult = []
        for res_dict in results:
            res_feature_dict = dict(res_dict)
            for key in list(res_feature_dict):
                if not cast(str, key).startswith("original"):
                    del res_feature_dict[key]
            result_feature_dicts.append(cast(OrderedDict[str, float], res_feature_dict))
        return result_feature_dicts

    def get_logging_config(self) -> (Dict, logging.handlers.QueueListener):
        queue_listener = None

        verbose_level = 30
        logger_level = 20

        self.log_file.unlink(missing_ok=True)

        logging_config = {
            "version": 1,
            "disable_existing_loggers": False,
            "formatters": {
                "default": {
                    "format": "[%(asctime)s] %(levelname)-.1s: %(name)s: %(message)s",
                    "datefmt": "%Y-%m-%d %H:%M:%S"
                }
            },
            "handlers": {
                "console": {
                    "class": "logging.StreamHandler",
                    "level": verbose_level,
                    "formatter": "default"
                }
            },
            "loggers": {
                "radiomics": {
                    "level": logger_level,
                    "handlers": ["console"]
                }
            }
        }

        # Set up logging to file
        if self.is_multiprocessing:
            q = Manager().Queue(-1)
            threading.current_thread().setName("Main")

            logging_config["formatters"]["default"]["format"] = ("[%(asctime)s] %(levelname)-.1s:"
                                                                 "(%(threadName)s) %(name)s: %(message)s")

            logging_config["handlers"]["logfile"] = {
                "class": "logging.handlers.QueueHandler",
                "queue": q,
                "level": logger_level,
                "formatter": "default"
            }

            file_handler = logging.FileHandler(filename=self.log_file, mode="a")
            file_handler.setFormatter(logging.Formatter(fmt=logging_config["formatters"]["default"].get("format"),
                                                        datefmt=logging_config["formatters"]["default"].get("datefmt")))

            queue_listener = logging.handlers.QueueListener(q, file_handler)
            queue_listener.start()
        else:
            logging_config["handlers"]["logfile"] = {
                "class": "logging.FileHandler",
                "filename": self.log_file,
                "mode": "a",
                "level": logger_level,
                "formatter": "default"
            }
        logging_config["loggers"]["radiomics"]["handlers"].append("logfile")

        logging.config.dictConfig(logging_config)

        return logging_config, queue_listener

    def get_cases(self) -> List[Tuple[int, Dict[str, str]]]:
        generator = []
        for i in range(0, self.number_of_images):
            data_path = self.data_files[i]
            generator.append((i + 1, {"Image": str(data_path.image), "Mask": str(data_path.mask)}))

        return generator

    def extract_features(self, cases) -> List[OrderedDict]:
        extractor = featureextractor.RadiomicsFeatureExtractor(str(self.params_file))

        results = []
        number_of_workers = min(cpu_count() - 1, self.number_of_images)
        if self.is_multiprocessing and number_of_workers > 1:
            async_results: List[AsyncResult] = []
            with Pool(number_of_workers) as pool:
                for case in cases:
                    async_results.append(pool.apply_async(partial(segment.extractSegment_parallel,
                                                                  case,
                                                                  extractor=extractor,
                                                                  out_dir=self.out_dir,
                                                                  logging_config=self.logging_config)))

                done: bool = False
                last_successful = -1
                while not done:
                    running, successful, error = 0, 0, 0
                    for result in async_results:
                        try:
                            if result.successful():
                                successful += 1
                            else:
                                error += 1
                        except ValueError:
                            running += 1

                    if error > 0:
                        raise ValueError

                    if successful > last_successful:
                        self.progress_callback(successful / self.number_of_images)
                        last_successful = successful

                    if successful == len(cases):
                        done = True

            results = [res.get() for res in async_results]
        else:
            for case in cases:
                results.append(segment.extractSegment(*case, extractor=extractor, out_dir=self.out_dir))

        for feature_filepath in self.out_dir.glob("features_*.csv"):
            feature_filepath.unlink()  # delete

        return results

    def write_outputs(self, results):
        logger.info("Processing results...")

        # Store the header of all calculated features
        # By checking all headers of cases > 1, and subtracting those already in case 1, original ordering of case 1 is
        # preserved. Additional headers are by definition generated by pyradiomics, and therefore can be appended at
        # the end.
        additional_headers = set()
        for case in results[1:]:
            additional_headers.update(set(case.keys()))
            additional_headers -= set(results[0].keys())  # Subtract all headers found in the first case

        headers = list(results[0].keys()) + sorted(additional_headers)

        csv_filepath: Path = self.out_dir / "results-{}.csv".format(self.id)
        txt_filepath: Path = self.out_dir / "results-{}.txt".format(self.id)
        json_filepath: Path = self.out_dir / "results-{}.json".format(self.id)
        for filepath in [csv_filepath, txt_filepath, json_filepath]:
            open(str(filepath), "w").close()  # clear

        for case_idx, case in enumerate(results, start=1):
            with open(csv_filepath, "a") as csv_file:
                writer = csv.DictWriter(csv_file, headers, lineterminator="\n", extrasaction="ignore")
                if case_idx == 1:
                    writer.writeheader()
                writer.writerow(case)

            with open(txt_filepath, "a") as txt_file:
                for image, mask in case.items():
                    txt_file.write("Case-{}_{}: {}\n".format(case_idx, image, mask))

        class NumpyEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, numpy.ndarray):
                    return obj.tolist()
                return json.JSONEncoder.default(self, obj)

        with open(json_filepath, "w+") as json_file:
            json.dump(results, json_file, cls=NumpyEncoder, indent=2)


def extract(id: int, data_files: DataPaths, progress_callback: Callable[[float], None]) -> FeatureExtractionResult:
    extraction = FeatureExtraction(id, data_files, progress_callback)

    result: FeatureExtractionResult = extraction.run()

    return result
