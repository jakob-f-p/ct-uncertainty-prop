from collections import OrderedDict
from collections.abc import Callable
from functools import partial
from multiprocessing import cpu_count, Pool, Value
from multiprocessing.pool import AsyncResult
from radiomics.scripts import segment


def do_extraction(extractor, logging_config, case):
    try:
        return segment.extractSegment_parallel(case, extractor=extractor, logging_config=logging_config)
    except ValueError:
        return OrderedDict()


def do_parallel_feature_extraction(cases,
                                   extractor,
                                   logging_config,
                                   progress_callback: Callable[[int], None]) -> list[OrderedDict]:
    number_of_workers = min(cpu_count() - 1, len(cases))

    results: list[OrderedDict] = []
    with Pool(number_of_workers) as pool:
        done_counter = Value("i", 0)

        def callback(res):
            val = 0
            with done_counter.get_lock():
                val = done_counter.value
                done_counter.value += 1
            progress_callback(val)

        do_extraction_helper = partial(do_extraction, extractor, logging_config)

        async_results: list[AsyncResult] = [pool.apply_async(partial(do_extraction_helper, case),
                                                             (), {}, callback)
                                            for case in cases]

        results = [res.get() for res in async_results]

    return results
