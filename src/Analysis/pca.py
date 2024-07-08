import numpy.typing as npt

from datapaths import FeatureData
from feature_dataset import Array2D, FeatureDataset
from sklearn import decomposition


def calculate(feature_data: FeatureData, dimensions: int = 2) -> Array2D:
    dataset = FeatureDataset([feature_data])

    pca_params = {
        "n_components": dimensions,
    }
    pca = decomposition.PCA(**pca_params)

    result: npt.NDArray = pca.fit_transform(dataset.data)

    list_result: Array2D = result.tolist()

    return list_result
