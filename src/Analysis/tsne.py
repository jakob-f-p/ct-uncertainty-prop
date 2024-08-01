import numpy as np
import numpy.typing as npt

from feature_extraction_cpp import FeatureData, SampleId
from feature_dataset import Array2D, Array2DList, FeatureDataset
from sklearn import manifold
from typing import cast


def calculate(feature_data_list: list[FeatureData], dimensions: int = 2) -> Array2DList:
    if len(feature_data_list) == 0 or len(feature_data_list[0].values) == 0:
        return Array2DList()

    dataset = FeatureDataset(feature_data_list)

    if np.all(dataset.data == 0):
        arr_1d_0: list[float] = []
        for j in range(0, dimensions):
            arr_1d_0.append(0)

        result: list[Array2D] = []
        for feature_data in feature_data_list:
            arr_2d: Array2D = []
            for i in range(0, len(feature_data.values)):
                arr_2d.append(list(arr_1d_0))
            result.append(arr_2d)

        return result

    tsne_params = {
        "n_components": dimensions,
        "perplexity": min(30.0, dataset.dimensions[0] - 1),
        "n_iter_without_progress": 300,
        "max_iter": 1250,
    }
    tsne = manifold.TSNE(**tsne_params)

    result: npt.NDArray = tsne.fit_transform(dataset.data)

    list_result: Array2D = result.tolist()

    if len(list_result) != len(dataset.sample_ids):
        raise ValueError("Result has different number of samples compared to input dataset")

    list_result_list: Array2DList = []
    current_list_result: Array2D = []
    current_group_idx: int = dataset.sample_ids[0].group_idx
    for i, (sample_id, result_row) in enumerate(zip(dataset.sample_ids, list_result)):
        s_id = cast(SampleId, sample_id)

        group_idx_has_changed: bool = current_group_idx != s_id.group_idx
        is_last_iteration: bool = i == len(dataset.sample_ids) - 1

        if group_idx_has_changed:
            list_result_list.append(list(current_list_result))
            current_list_result = []

        current_list_result.append(list(result_row))

        if is_last_iteration:
            list_result_list.append(list(current_list_result))
            current_list_result = []

        current_group_idx = s_id.group_idx

    return list_result_list
