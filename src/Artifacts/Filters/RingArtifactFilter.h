#pragma once

#include "ImageArtifactFilter.h"

class RingArtifactFilter : public ImageArtifactFilter {
public:
    RingArtifactFilter(const RingArtifactFilter&) = delete;
    void operator=(const RingArtifactFilter&) = delete;

    static RingArtifactFilter* New();
    vtkTypeMacro(RingArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(BrightRingWidth, float, 0.0, 100.0);
    vtkGetMacro(BrightRingWidth, float);

    vtkSetClampMacro(DarkRingWidth, float, 0.0, 100.0);
    vtkGetMacro(DarkRingWidth, float);

    vtkSetClampMacro(BrightIntensityValue, float, 0.0, 1000.0);
    vtkGetMacro(BrightIntensityValue, float);

    vtkSetClampMacro(DarkIntensityValue, float, -1000.0, 0.0);
    vtkGetMacro(DarkIntensityValue, float);

    auto
    SetCenterPoint(std::array<float, 3> center) -> void { if (center != Center) Center = center; Modified(); }

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
        float* ArtifactValues;
        float BrightRingWidth;
        float DarkRingWidth;
        float CombinedRingWidth;
        float BrightDarkThreshold;
        float BrightIntensityValue;
        float DarkIntensityValue;
        FloatPoint Center;

        Algorithm(RingArtifactFilter* self, vtkImageData* volumeData, float* artifactValues);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    float BrightRingWidth = 0.0F;
    float DarkRingWidth = 0.0F;

    float BrightIntensityValue = 0.0F;
    float DarkIntensityValue = 0.0F;

    FloatPoint Center = { 0.0F, 0.0F, 0.0F };
};
