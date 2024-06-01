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
from typing import List, Tuple


logger = logging.getLogger(__file__)


def run(params_file: Path, data_files: DataPaths, out_dir: Path):
    number_of_images = len(data_files)
    is_multiprocessing = number_of_images > 0

    log_file: Path = out_dir / 'log.txt'
    logging_config, queue_listener = get_logging_config(is_multiprocessing, log_file)

    try:
        logger.info('Starting PyRadiomics (version: %s)', radiomics.__version__)
        case_generator = get_case_generator(data_files)
        results = extract_features(params_file, data_files, out_dir, number_of_images, is_multiprocessing,
                                   case_generator, logging_config)
        write_outputs(results, out_dir, True, True, True)
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

    logfile_level = logging.NOTSET
    verbosity = 4
    verbose_level = (6 - verbosity) * 10  # convert to python logging level
    logger_level = min(logfile_level, verbose_level)

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

    f = open(str(log_file), 'w+')
    f.close()

    # Set up logging to file
    if is_multiprocessing:
        q = Manager().Queue(-1)
        threading.current_thread().setName('Main')

        logging_config['handlers']['logfile'] = {
            'class': 'logging.handlers.QueueHandler',
            'queue': q,
            'level': logfile_level,
            'formatter': 'default'
        }

        file_handler = logging.FileHandler(filename=log_file)
        file_handler.setFormatter(logging.Formatter(fmt=logging_config['formatters']['default'].get('format'),
                                                    datefmt=logging_config['formatters']['default'].get('datefmt')))

        queue_listener = logging.handlers.QueueListener(q, file_handler)
        queue_listener.start()
    else:
        logging_config['handlers']['logfile'] = {
            'class': 'logging.FileHandler',
            'filename': log_file,
            'mode': 'a',
            'level': logfile_level,
            'formatter': 'default'
        }
    logging_config['loggers']['radiomics']['handlers'].append('logfile')

    logging.config.dictConfig(logging_config)

    return logging_config, queue_listener


def get_case_generator(data_paths) -> List[Tuple[int, dict]]:
    generator = []
    for i in range(0, len(data_paths)):
        data_path = data_paths[i]
        generator.append((i + 1, {"Image": data_path.image.name, "Mask": data_path.mask.name}))
    return generator


def extract_features(params_path: Path, data_paths, out_path: Path, number_of_images,
                     is_multiprocessing: bool, case_generator, logging_config: dict):
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
                                          out_dir=out_path,
                                          logging_config=logging_config),
                                  case_generator,
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
        for case in case_generator:
            results.append(segment.extractSegment(*case,
                                                  extractor=extractor,
                                                  out_dir=out_path))
    return results


def write_outputs(results, out_dir: Path, write_csv: bool, write_txt: bool, write_json: bool):
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

    for case_idx, case in enumerate(results, start=1):
        case['Image'] = case['Image']
        case['Mask'] = case['Mask']

        if write_csv:
            file_path: Path = out_dir / "results.csv"
            with open(file_path, 'w+') as csv_file:
                writer = csv.DictWriter(csv_file, headers, lineterminator='\n', extrasaction='ignore')
                if case_idx == 1:
                    writer.writeheader()
                writer.writerow(case)  # if skip_nans is enabled, nan-values are written as empty strings

        if write_txt:
            file_path: Path = out_dir / "results.txt"
            with open(file_path, 'w+') as txt_file:
                for image, mask in case.items():
                    txt_file.write("Case-{}_{}: {}\n".format(case_idx, image, mask))

    if write_json:
        class NumpyEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, numpy.ndarray):
                    return obj.tolist()
                return json.JSONEncoder.default(self, obj)

        file_path: Path = out_dir / "results.json"
        with open(file_path, 'w+') as json_file:
            json.dump(results, json_file, cls=NumpyEncoder, indent=2)
