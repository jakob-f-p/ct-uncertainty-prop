import numpy.typing as npt

from feature_extraction_cpp import FeatureData, PcaData
from feature_dataset import Array2D, FeatureDataset
from sklearn import decomposition


def calculate(feature_data: FeatureData, dimensions: int = 2) -> PcaData:
    dataset = FeatureDataset([feature_data])

    pca_params = {
        "n_components": dimensions,
    }
    pca = decomposition.PCA(**pca_params)

    values_np: npt.NDArray = pca.fit_transform(dataset.data)
    values: Array2D = values_np.tolist()

    explained_variance_ratios_np: npt.NDArray = pca.explained_variance_ratio_
    explained_variance_ratios: list[float] = explained_variance_ratios_np.tolist()

    pca_components_np: npt.NDArray = pca.components_
    pca_components: Array2D = pca_components_np.tolist()

    pca_data = PcaData(explained_variance_ratios, pca_components, values)

    return pca_data
