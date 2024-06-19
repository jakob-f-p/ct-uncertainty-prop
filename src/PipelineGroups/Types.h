#pragma once

#include <vtkType.h>

#include <string>
#include <vector>


using SampleCoordinateData = std::vector<std::vector<double>>;
using GroupCoordinateData = std::vector<SampleCoordinateData>;

struct FeatureData {
    using StringVector = std::vector<std::string>;
    using Vector2DDouble = std::vector<std::vector<double>>;

    StringVector Names;
    Vector2DDouble Values;
};

struct DataStatus {
    DataStatus() = default;
    DataStatus(vtkMTimeType image, vtkMTimeType feature, vtkMTimeType pca, vtkMTimeType tsne) :
            Image(image), Feature(feature), Pca(pca), Tsne(tsne),
            Total(std::min({ Image, Feature, Pca, Tsne })) {}

    [[nodiscard]] auto
    IsComplete() const noexcept -> bool { return Image > 0 && Feature > 0 && Pca > 0 && Tsne > 0 && Total > 0; }

    auto
    Update(DataStatus const& other) noexcept -> void {
        Image   = std::max(other.Image, Image);
        Feature = std::max(other.Feature, Feature);
        Pca     = std::max(other.Pca, Pca);
        Tsne    = std::max(other.Tsne, Tsne);
        Total   = std::max(other.Total, Total);
    }

    vtkMTimeType Image = 0;
    vtkMTimeType Feature = 0;
    vtkMTimeType Pca = 0;
    vtkMTimeType Tsne = 0;
    vtkMTimeType Total = 0;
};

struct SampleId {
    uint16_t GroupIdx;
    uint16_t StateIdx;
};

