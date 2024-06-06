import csv
import json
import logging.config
import logging.handlers
import numpy
import radiomics
import threading

from datapaths import DataPaths
from functools import partial
from multiprocessing import cpu_count, Manager, Pool
from pathlib import Path
from radiomics import featureextractor
from radiomics.scripts import segment
from typing import List, Tuple, Dict

logger = logging.getLogger(__file__)


def run(params_file: Path, data_files: DataPaths, out_dir: Path):
    number_of_images = len(data_files)
    is_multiprocessing = number_of_images > 1

    log_file: Path = out_dir / 'log.txt'
    logging_config, queue_listener = get_logging_config(is_multiprocessing, log_file)

    try:
        logger.info('Starting PyRadiomics (version: %s)', radiomics.__version__)
        cases = get_cases(data_files)
        results = extract_features(params_file, data_files, out_dir, number_of_images, is_multiprocessing,
                                   cases, logging_config)
        write_outputs(results, out_dir)
        logger.info('Finished extraction successfully...')
    except (KeyboardInterrupt, SystemExit):
        logger.info('Cancelling Extraction')
        return -1
    finally:
        if queue_listener is not None:
            queue_listener.stop()
    return 0  # success


def get_logging_config(is_multiprocessing: bool, log_file: Path):
    queue_listener = None

    verbose_level = 30
    logger_level = 20

    log_file.unlink()

    logging_config = {
        'version': 1,
        'disable_existing_loggers': False,
        'formatters': {
            'default': {
                'format': '[%(asctime)s] %(levelname)-.1s: %(name)s: %(message)s',
                'datefmt': '%Y-%m-%d %H:%M:%S'
            }
        },
        'handlers': {
            'console': {
                'class': 'logging.StreamHandler',
                'level': verbose_level,
                'formatter': 'default'
            }
        },
        'loggers': {
            'radiomics': {
                'level': logger_level,
                'handlers': ['console']
            }
        }
    }

    if is_multiprocessing:
        # Update the logger format to include the threadname if multiprocessing
        # is enabled
        logging_config['formatters']['default']['format'] = \
            '[%(asctime)s] %(levelname)-.1s: (%(threadName)s) %(name)s: %(message)s'

    # Set up logging to file
    if is_multiprocessing:
        q = Manager().Queue(-1)
        threading.current_thread().setName('Main')

        logging_config['handlers']['logfile'] = {
            'class': 'logging.handlers.QueueHandler',
            'queue': q,
            'level': logger_level,
            'formatter': 'default'
        }

        file_handler = logging.FileHandler(filename=log_file, mode='a')
        file_handler.setFormatter(logging.Formatter(fmt=logging_config['formatters']['default'].get('format'),
                                                    datefmt=logging_config['formatters']['default'].get('datefmt')))

        queue_listener = logging.handlers.QueueListener(q, file_handler)
        queue_listener.start()
    else:
        logging_config['handlers']['logfile'] = {
            'class': 'logging.FileHandler',
            'filename': log_file,
            'mode': 'a',
            'level': logger_level,
            'formatter': 'default'
        }
    logging_config['loggers']['radiomics']['handlers'].append('logfile')

    logging.config.dictConfig(logging_config)

    return logging_config, queue_listener


def get_cases(data_paths) -> List[Tuple[int, Dict[str, str]]]:
    generator = []
    for i in range(0, len(data_paths)):
        data_path = data_paths[i]
        generator.append((i + 1, {"Image": str(data_path.image), "Mask": str(data_path.mask)}))

    return generator


def extract_features(params_path: Path, data_paths, out_dir: Path, number_of_images,
                     is_multiprocessing: bool, cases, logging_config: dict):
    extractor = featureextractor.RadiomicsFeatureExtractor(str(params_path))

    if not is_multiprocessing:
        return []

    results = []
    number_of_workers = min(cpu_count() - 1, len(data_paths))
    if number_of_workers > 1:  # multiple cases, parallel processing enabled
        pool = Pool(number_of_workers)
        try:
            task = pool.map_async(partial(segment.extractSegment_parallel,
                                          extractor=extractor,
                                          out_dir=out_dir,
                                          logging_config=logging_config),
                                  cases,
                                  chunksize=min(10, number_of_images))
            # Wait for the results to be done. task.get() without timeout performs a blocking call, which prevents
            # the program from processing the KeyboardInterrupt if it occurs
            while not task.ready():
                pass
            results = task.get()
            pool.close()
        except (KeyboardInterrupt, SystemExit):
            pool.terminate()
            raise
        finally:
            pool.join()
    else:
        for case in cases:
            results.append(segment.extractSegment(*case,
                                                  extractor=extractor,
                                                  out_dir=out_dir))

    for feature_filepath in out_dir.glob("features_*.csv"):
        feature_filepath.unlink() # delete

    return results


def write_outputs(results, out_dir: Path):
    logger.info('Processing results...')

    # Store the header of all calculated features
    # By checking all headers of cases > 1, and subtracting those already in case 1, original ordering of case 1 is
    # preserved. Additional headers are by definition generated by pyradiomics, and therefore can be appended at
    # the end.
    additional_headers = set()
    for case in results[1:]:
        additional_headers.update(set(case.keys()))
        additional_headers -= set(results[0].keys())  # Subtract all headers found in the first case

    headers = list(results[0].keys()) + sorted(additional_headers)

    csv_filepath: Path = out_dir / "results.csv"
    txt_filepath: Path = out_dir / "results.txt"
    json_filepath: Path = out_dir / "results.json"
    for filepath in [csv_filepath, txt_filepath, json_filepath]:
        open(str(filepath), 'w').close()  # clear

    for case_idx, case in enumerate(results, start=1):
        with open(csv_filepath, 'a') as csv_file:
            writer = csv.DictWriter(csv_file, headers, lineterminator='\n', extrasaction='ignore')
            if case_idx == 1:
                writer.writeheader()
            writer.writerow(case)

        with open(txt_filepath, 'a') as txt_file:
            for image, mask in case.items():
                txt_file.write("Case-{}_{}: {}\n".format(case_idx, image, mask))

    class NumpyEncoder(json.JSONEncoder):
        def default(self, obj):
            if isinstance(obj, numpy.ndarray):
                return obj.tolist()
            return json.JSONEncoder.default(self, obj)

    json_filepath: Path = out_dir / "results.json"
    with open(json_filepath, 'w+') as json_file:
        json.dump(results, json_file, cls=NumpyEncoder, indent=2)
