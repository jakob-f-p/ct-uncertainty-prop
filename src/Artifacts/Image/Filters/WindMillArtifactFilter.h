#pragma once

#include "ImageArtifactFilter.h"

class WindMillArtifactFilter : public ImageArtifactFilter {
public:
    WindMillArtifactFilter(const WindMillArtifactFilter&) = delete;
    void operator=(const WindMillArtifactFilter&) = delete;

    static WindMillArtifactFilter* New();
    vtkTypeMacro(WindMillArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(BrightAngularWidth, float, 0.0, 360.0);
    vtkGetMacro(BrightAngularWidth, float);

    vtkSetClampMacro(DarkAngularWidth, float, 0.0, 360.0);
    vtkGetMacro(DarkAngularWidth, float);

    vtkSetClampMacro(BrightIntensityValue, float, 0.0, 1000.0);
    vtkGetMacro(BrightIntensityValue, float);

    vtkSetClampMacro(DarkIntensityValue, float, -1000.0, 0.0);
    vtkGetMacro(DarkIntensityValue, float);

    auto
    SetCenterPoint(std::array<float, 3> center) -> void { if (center != Center) Center = center; Modified(); }

    [[nodiscard]] auto
    GetCenterPoint() -> std::array<float, 3> { return Center; }

protected:
    WindMillArtifactFilter() = default;
    ~WindMillArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto
    ExecuteDataWithImageInformation(vtkImageData* input,
                                    vtkImageData* output,
                                    vtkInformation* outInfo) -> void override;

    struct Algorithm {
        WindMillArtifactFilter* Self;
        vtkImageData* VolumeData;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
        float* ArtifactValues;
        float BrightAngularWidth;
        float DarkAngularWidth;
        float CombinedAngularWidth;
        float BrightDarkThreshold;
        float BrightIntensityValue;
        float DarkIntensityValue;
        FloatPoint Center;

        Algorithm(WindMillArtifactFilter* self, vtkImageData* volumeData, float* artifactValues);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    float BrightAngularWidth = 0.0F;
    float DarkAngularWidth = 0.0F;

    float BrightIntensityValue = 0.0F;
    float DarkIntensityValue = 0.0F;

    FloatPoint Center = { 0.0F, 0.0F, 0.0F };
};
