import numpy as np
import numpy.typing as npt

from feature_extraction_cpp import FeatureData, SampleId
from sklearn import preprocessing
from typing import List


Array2D = List[List[float]]
Array2DList = List[Array2D]


class FeatureDataset:
    original_data: npt.NDArray
    data: npt.NDArray  # normalized
    feature_names: List[str]
    sample_ids: List[SampleId]
    dimensions: tuple[int, int]

    def __init__(self, feature_data_list: List[FeatureData], scale: bool = True):
        if (len(feature_data_list) == 0 or len(feature_data_list[0].values) == 0
                or len(feature_data_list[0].values[0]) == 0):
            raise ValueError("Feature data list is empty")

        sample_values: List[List[float]] = []
        self.feature_names: List[str] = feature_data_list[0].names
        self.sample_ids: List[SampleId] = []

        for group_idx, feature_data in enumerate(feature_data_list):
            for state_idx, feature_row in enumerate(feature_data.values):
                sample_values.append(list(feature_row))

                self.sample_ids.append(SampleId(group_idx, state_idx))

                if feature_data.names != self.feature_names:
                    raise ValueError("Differing number of features, feature order, or feature names")

        self.dimensions = (len(self.sample_ids), len(self.feature_names))
        self.original_data = np.array(sample_values)
        self.original_data = np.nan_to_num(self.original_data)

        scaler = preprocessing.StandardScaler(with_std=scale)
        self.data = scaler.fit_transform(self.original_data)

    def __repr__(self) -> str:
        return ("TsneInputDataSet(\n\tdata: {}\n\tfeature_names: {}\n\tsample_ids: {}\n\tdimensions: {})"
                .format(self.data, self.feature_names, self.sample_ids, self.dimensions))
