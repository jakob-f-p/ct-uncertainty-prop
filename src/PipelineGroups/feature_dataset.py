import numpy as np
import numpy.typing as npt

from datapaths import FeatureData
from sklearn import preprocessing
from typing import List


Array2D = List[List[float]]
Array2DList = List[Array2D]


class SampleId:
    def __init__(self, group_id: int, state_id: int):
        self.groupId: int = group_id
        self.stateId: int = state_id

    def __repr__(self) -> str:
        return "({}, {})".format(self.groupId, self.stateId)


class FeatureDataset:
    original_data: npt.NDArray
    data: npt.NDArray  # normalized
    feature_names: List[str]
    sample_ids: List[SampleId]
    dimensions: tuple[int, int]

    def __init__(self, feature_data_list: List[FeatureData]):
        if (len(feature_data_list) == 0 or len(feature_data_list[0].values) == 0
                or len(feature_data_list[0].values[0]) == 0):
            raise ValueError("Feature data list is empty")

        sample_values: List[List[float]] = []
        self.feature_names: List[str] = feature_data_list[0].names
        self.sample_ids: List[SampleId] = []

        for groupId, feature_data in enumerate(feature_data_list):
            for stateId, feature_row in enumerate(feature_data.values):
                sample_values.append(list(feature_row))

                self.sample_ids.append(SampleId(groupId, stateId))

                if feature_data.names != self.feature_names:
                    raise ValueError("Differing number of features, feature order, or feature names")

        self.dimensions = (len(self.sample_ids), len(self.feature_names))
        self.original_data = np.array(sample_values)

        scaler = preprocessing.StandardScaler()
        self.data = scaler.fit_transform(self.original_data)

    def __repr__(self) -> str:
        return ("TsneInputDataSet(\n\tdata: {}\n\tfeature_names: {}\n\tsample_ids: {}\n\tdimensions: {})"
                .format(self.data, self.feature_names, self.sample_ids, self.dimensions))
