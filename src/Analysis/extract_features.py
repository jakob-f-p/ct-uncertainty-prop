import csv
import json
import logging.config
import logging.handlers
import numpy as np
import radiomics
import SimpleITK as sitk
import threading
import warnings

from collections import OrderedDict
from collections.abc import Callable
from datetime import datetime
from extract_features_parallel import do_parallel_feature_extraction
from feature_extraction_cpp import (extraction_params_file, FeatureData, feature_directory,
                                    VtkImageMaskPair, VtkImageData, VtkType_Short, VtkType_Float)
from multiprocessing import Manager
from pathlib import Path
from radiomics import featureextractor
from radiomics.scripts import segment
from typing import cast

# warnings.filterwarnings(action="ignore", category=RuntimeWarning)
logging.captureWarnings(True)

logger = logging.getLogger(__file__)

FeatureExtractionResult = list[OrderedDict[str, float]]


class SitkImageMaskPair:
    def __init__(self, image: sitk.Image, mask: sitk.Mask):
        self.Image = image
        self.Mask = mask


def vtk_to_sitk_image(vtk_image: VtkImageData) -> sitk.Image:
    dims = list(vtk_image.get_dimensions())
    origin = vtk_image.get_origin()
    spacing = vtk_image.get_spacing()

    vtk_data_type: int = vtk_image.get_data_type()
    vtk_to_np_type_dict = {VtkType_Short: np.short, VtkType_Float: np.float32}
    np_data_type = vtk_to_np_type_dict[vtk_data_type]

    np_data = np.frombuffer(vtk_image, dtype=np_data_type)

    dims.reverse()
    np_data.shape = tuple(dims)

    sitk_image = sitk.GetImageFromArray(np_data)
    sitk_image.SetSpacing(spacing)
    sitk_image.SetOrigin(origin)

    return sitk_image


def vtk_pairs_to_sitk_pairs(vtk_image_mask_pairs: list[VtkImageMaskPair]) -> list[SitkImageMaskPair]:
    sitk_pairs: list[SitkImageMaskPair] = []

    for vtk_image_mask_pair in vtk_image_mask_pairs:
        vtk_image: VtkImageData = vtk_image_mask_pair.image
        vtk_mask: VtkImageData = vtk_image_mask_pair.mask

        sitk_pair = SitkImageMaskPair(vtk_to_sitk_image(vtk_image), vtk_to_sitk_image(vtk_mask))
        sitk_pairs.append(sitk_pair)

    return sitk_pairs


class FeatureExtraction:
    params_file: Path
    vtk_image_mask_pairs: list[VtkImageMaskPair]
    sitk_image_mask_pairs: list[SitkImageMaskPair]
    number_of_images: int
    is_multiprocessing: bool
    out_dir: Path
    log_file: Path
    logging_config: dict
    logging_queue_listener: logging.handlers.QueueListener
    progress_callback: Callable[[float], None]

    debug: bool = False

    def __init__(self, vtk_image_mask_pairs: list[VtkImageMaskPair], progress_callback: Callable[[float], None]):
        self.params_file = extraction_params_file
        self.vtk_image_mask_pairs = vtk_image_mask_pairs
        self.sitk_image_mask_pairs = vtk_pairs_to_sitk_pairs(self.vtk_image_mask_pairs)
        self.number_of_images = len(vtk_image_mask_pairs)
        self.is_multiprocessing = False  # self.number_of_images > 1
        self.out_dir = feature_directory
        self.log_file = self.out_dir / "extraction_log.txt"
        self.logging_config, self.logging_queue_listener = self.get_logging_config()
        self.progress_callback = progress_callback

    def run(self) -> FeatureExtractionResult:
        results = []
        try:
            logger.info("Starting PyRadiomics (version: %s)", radiomics.__version__)

            cases = self.get_cases()

            results = self.extract_features(cases)

            if self.debug:
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
            radiomics_logger = logging.getLogger("radiomics")
            radiomics_logger.handlers.clear()

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

    def get_logging_config(self) -> (dict, logging.handlers.QueueListener):
        queue_listener = None

        file_handler_level = 30
        console_handler_level = 50
        logger_level = min(file_handler_level, console_handler_level)

        try:
            self.log_file.unlink(missing_ok=True)
        except Exception as e:
            print(e)

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
                    "level": console_handler_level,
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
                "level": file_handler_level,
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
                "level": file_handler_level,
                "formatter": "default"
            }
        logging_config["loggers"]["radiomics"]["handlers"].append("logfile")

        logging.config.dictConfig(logging_config)

        return logging_config, queue_listener

    def get_cases(self) -> list[tuple[int, dict[str, str]]]:
        generator = []
        for i in range(0, self.number_of_images):
            pair = self.sitk_image_mask_pairs[i]
            generator.append((i + 1, {"Image": pair.Image, "Mask": pair.Mask}))

        return generator

    def extract_features(self, cases) -> list[OrderedDict]:
        extractor = featureextractor.RadiomicsFeatureExtractor(str(self.params_file))

        if self.is_multiprocessing:
            results = do_parallel_feature_extraction(cases, extractor, self.logging_config, self.progress_callback)

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

        else:
            results = []
            for (i, case) in enumerate(cases):
                self.progress_callback(i)
                results.append(segment.extractSegment(*case, extractor=extractor))

        return results

    def write_outputs(self, results):
        logger.info("Processing results...")

        for result in results[1:]:
            if list(result.keys()) != list(results[0].keys()):
                raise ValueError("Results do not have the same keys")

        headers = list(results[0].keys())

        time_stamp = datetime.now()
        time_stamp_string = time_stamp.strftime("%Y-%m-%d-%H-%M-%S")
        csv_filepath: Path = self.out_dir / "results-debug-{}.csv".format(time_stamp_string)
        json_filepath: Path = self.out_dir / "results-debug-{}.json".format(time_stamp_string)

        for filepath in [csv_filepath, json_filepath]:
            open(str(filepath), "w").close()  # clear

        for case_idx, case in enumerate(results, start=1):
            with open(csv_filepath, "a") as csv_file:
                writer = csv.DictWriter(csv_file, headers, lineterminator="\n", extrasaction="ignore")
                if case_idx == 1:
                    writer.writeheader()
                writer.writerow(case)

        class NumpyEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, np.ndarray):
                    return obj.tolist()

                if isinstance(obj, sitk.Image):
                    return id(obj)

                return json.JSONEncoder.default(self, obj)

        with open(json_filepath, "w+") as json_file:
            json.dump(results, json_file, cls=NumpyEncoder, indent=2)


def extract(image_mask_pairs: list[VtkImageMaskPair],
            progress_callback: Callable[[float], None] = lambda f: None) -> FeatureExtractionResult:
    extraction = FeatureExtraction(image_mask_pairs, progress_callback)

    result: FeatureExtractionResult = extraction.run()

    feature_names: list[str] = list(result[0].keys())
    feature_values: list[list[float]] = []
    for ordered_dict in result:
        feature_values.append(list(ordered_dict.values()))

    feature_data = FeatureData(feature_names, feature_values)

    return feature_data
