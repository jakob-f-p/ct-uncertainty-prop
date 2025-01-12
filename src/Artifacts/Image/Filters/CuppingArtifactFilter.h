#pragma once

#include "ImageArtifactFilter.h"

class CuppingArtifactFilter : public ImageArtifactFilter {
public:
    CuppingArtifactFilter(const CuppingArtifactFilter&) = delete;
    void operator=(const CuppingArtifactFilter&) = delete;

    static CuppingArtifactFilter* New();
    vtkTypeMacro(CuppingArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(MinRadiodensityFactor, float, 0.0, 1.0);
    vtkGetMacro(MinRadiodensityFactor, float);

    auto
    SetCenterPoint(std::array<float, 3> center) -> void {
        if (center != Center)
            Center = center;
        Modified();
    }

    [[nodiscard]] auto
    GetCenterPoint() const -> std::array<float, 3> { return Center; }

protected:
    CuppingArtifactFilter() = default;
    ~CuppingArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto
    ExecuteDataWithImageInformation(vtkImageData* input, vtkImageData* output, vtkInformation* outInfo) -> void override;

    struct Algorithm {
        CuppingArtifactFilter* Self;
        vtkImageData* VolumeData;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
        DoublePoint StartPoint;
        float const* Radiodensities;
        float* ArtifactValues;
        float MinRadiodensityFactor;
        float RadiodensityFactorRange;
        FloatPoint Center;
        float xyMaxDistance;

        Algorithm(CuppingArtifactFilter* self,
                  vtkImageData* volumeData,
                  float const* radiodensities,
                  float* artifactValues);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    float MinRadiodensityFactor = 0.0F;

    FloatPoint Center = { 0.0F, 0.0F, 0.0F };
};
