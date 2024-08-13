#pragma once

#include "ImageArtifactFilter.h"

class RingArtifactFilter : public ImageArtifactFilter {
public:
    RingArtifactFilter(const RingArtifactFilter&) = delete;
    void operator=(const RingArtifactFilter&) = delete;

    static RingArtifactFilter* New();
    vtkTypeMacro(RingArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(InnerRadius, float, 0.0, 100.0);
    vtkGetMacro(InnerRadius, float);

    vtkSetClampMacro(RingWidth, float, 0.0, 100.0);
    vtkGetMacro(RingWidth, float);

    vtkSetClampMacro(RadiodensityFactor, float, 0.0, 100.0);
    vtkGetMacro(RadiodensityFactor, float);

    auto
    SetCenterPoint(std::array<float, 3> center) -> void {
        if (center != Center)
            Center = center;

        Modified();
    }

    [[nodiscard]] auto
    GetCenterPoint() -> std::array<float, 3> { return Center; }

protected:
    RingArtifactFilter() = default;
    ~RingArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto
    ExecuteDataWithImageInformation(vtkImageData* input, vtkImageData* output, vtkInformation* outInfo) -> void override;

    struct Algorithm {
        RingArtifactFilter* Self;
        vtkImageData* VolumeData;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
        DoublePoint StartPoint;
        float const* Radiodensities;
        float* ArtifactValues;
        float InnerRadius;
        float RingWidth;
        float OuterRadius;
        float RadiodensityChangeFactor;
        FloatPoint Center;

        Algorithm(RingArtifactFilter* self,
                  vtkImageData* volumeData,
                  float const* radiodensities,
                  float* artifactValues);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    float InnerRadius = 0.0F;
    float RingWidth = 0.0F;
    float RadiodensityFactor = 0.0F;

    FloatPoint Center = { 0.0F, 0.0F, 0.0F };
};
