import csv
import json
import logging.config
import logging.handlers
import numpy as np
import radiomics
import threading
import warnings

from collections import OrderedDict
from datapaths import DataPaths, extraction_params_file, FeatureData, feature_directory
from functools import partial
from multiprocessing import cpu_count, Manager, Pool
from multiprocessing.pool import AsyncResult
from pathlib import Path
from radiomics import featureextractor
from radiomics.scripts import segment
from typing import cast, Dict, Callable, List, Tuple

warnings.filterwarnings(action="ignore", category=RuntimeWarning)

logger = logging.getLogger(__file__)

FeatureExtractionResult = List[OrderedDict[str, float]]


def do_extraction(case, extractor, out_dir, logging_config) -> OrderedDict:
    try:
        return segment.extractSegment_parallel(case,
                                               extractor=extractor,
                                               out_dir=out_dir,
                                               logging_config=logging_config)
    except ValueError:
        return OrderedDict()


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
            print(1)
            logger.info("Starting PyRadiomics (version: %s)", radiomics.__version__)
            print(2)
            cases = self.get_cases()
            print(3)
            results = self.extract_features(cases)
            print(4)
            self.write_outputs(results)
            print(5)
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
            res_feature_dict = OrderedDict(res_dict)
            for key in list(res_feature_dict):
                if not cast(str, key).startswith("original"):
                    del res_feature_dict[key]
            result_feature_dicts.append(cast(OrderedDict[str, float], res_feature_dict))

        for res_feature_dict in result_feature_dicts:
            for key, value in res_feature_dict.items():
                res_feature_dict[key] = np.float64(value)

        return result_feature_dicts

    def get_logging_config(self) -> (Dict, logging.handlers.QueueListener):
        queue_listener = None

        verbose_level = 30
        logger_level = 50

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
                    # async_results.append(pool.apply_async(partial(do_extraction, case, extractor,
                    #                                               self.out_dir, self.logging_config)))

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

                    # if error > 0:
                    #     raise ValueError

                    if successful + error > last_successful:
                        self.progress_callback(successful / self.number_of_images)
                        last_successful = successful + error

                    if successful + error == len(cases):
                        done = True

            results = [res.get() if res.successful() else OrderedDict() for res in async_results]
        else:
            for (i, case) in enumerate(cases):
                self.progress_callback(i / self.number_of_images)
                results.append(segment.extractSegment(*case, extractor=extractor, out_dir=self.out_dir))

        for feature_filepath in self.out_dir.glob("features_*.csv"):
            feature_filepath.unlink()  # delete

        # set feature values of empty results to 0
        lengths = [len(result) for result in results]
        max_length = max(lengths)
        max_idx = lengths.index(max_length)
        max_element = results[max_idx]
        for result in results:
            if len(result) < max_length:
                for max_key in max_element:
                    if max_key not in result:
                        result[max_key] = max_element[max_key] if not max_key.startswith("original") else 0.0

        return results

    def write_outputs(self, results):
        logger.info("Processing results...")

        for result in results[1:]:
            if list(result.keys()) != list(results[0].keys()):
                raise ValueError("Results do not have the same keys")

        headers = list(results[0].keys())

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
                if isinstance(obj, np.ndarray):
                    return obj.tolist()
                return json.JSONEncoder.default(self, obj)

        with open(json_filepath, "w+") as json_file:
            json.dump(results, json_file, cls=NumpyEncoder, indent=2)


def extract(id: int, data_files: DataPaths, progress_callback: Callable[[float], None]) -> FeatureExtractionResult:
    extraction = FeatureExtraction(id, data_files, progress_callback)

    result: FeatureExtractionResult = extraction.run()

    feature_names: List[str] = list(result[0].keys())
    feature_values: List[List[float]] = []
    for ordered_dict in result:
        feature_values.append(list(ordered_dict.values()))

    feature_data = FeatureData(feature_names, feature_values)

    return feature_data
