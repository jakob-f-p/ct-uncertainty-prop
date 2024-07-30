from collections import OrderedDict
from collections.abc import Callable
from functools import partial
from multiprocessing import cpu_count, Pool, Value
from multiprocessing.pool import AsyncResult
from radiomics.scripts import segment

import logging


def do_extraction(extractor, logging_config, case, sample_id):
    class InfoWarningFilter(logging.Filter):
        def __init__(self, name=""):
            super().__init__(name)
            self.name = name

        def filter(self, record) -> bool:
            # return (record.levelno >= logging.WARNING
            #         or record.levelno >= logging.INFO and (record.name == "uncertainty_propagation"
            #                                                or record.name == "radiomics.script"))
            return record.levelno >= logging.DEBUG

    logging_config["filters"] = {
        "info_warning_filter": {
            "()": InfoWarningFilter,
            "name": "info_warning_filter"
        }
    }
    logging_config["handlers"]["logfile"]["filters"] = ["info_warning_filter"]

    try:
        result = segment.extractSegment_parallel(case, extractor=extractor, logging_config=logging_config)
    except ValueError:
        result = OrderedDict()

    logger = logging.getLogger("radiomics.script")
    logger.info(f"Processed sample with id ({sample_id[0]}, {sample_id[1]})")

    return result


def do_parallel_feature_extraction(cases,
                                   sample_ids,
                                   extractor,
                                   logging_config,
                                   progress_callback: Callable[[int], None]) -> list[OrderedDict]:
    number_of_workers = min(cpu_count() - 1, len(cases))

    pickleable_sample_ids: list[tuple[int, int]] = [(s_id.group_idx, s_id.state_idx) for s_id in sample_ids]

    with Pool(number_of_workers) as pool:
        done_counter = Value("i", sample_ids[0].state_idx)

        def callback(res):
            with done_counter.get_lock():
                val = done_counter.value
                done_counter.value += 1
            progress_callback(val)

        async_results: list[AsyncResult] = [pool.apply_async(partial(do_extraction,
                                                                     extractor, logging_config, case, sample_id),
                                                             (), {}, callback)
                                            for case, sample_id in zip(cases, pickleable_sample_ids)]

        results: list[OrderedDict] = [res.get() for res in async_results]

    return results
