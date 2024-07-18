#pragma once

#include "ImageArtifactFilter.h"

class CuppingArtifactFilter : public ImageArtifactFilter {
public:
    CuppingArtifactFilter(const CuppingArtifactFilter&) = delete;
    void operator=(const CuppingArtifactFilter&) = delete;

    static CuppingArtifactFilter* New();
    vtkTypeMacro(CuppingArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(DarkIntensityValue, float, -1000.0, 0.0);
    vtkGetMacro(DarkIntensityValue, float);

    auto
    SetCenterPoint(std::array<float, 3> center) -> void { if (center != Center) Center = center; Modified(); }

    [[nodiscard]] auto
    GetCenterPoint() -> std::array<float, 3> { return Center; }

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
        float* ArtifactValues;
        float DarkIntensityValue;
        float xyMaxDistance;
        FloatPoint Center;

        Algorithm(CuppingArtifactFilter* self, vtkImageData* volumeData, float* artifactValues);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    float DarkIntensityValue = 0.0F;

    FloatPoint Center = { 0.0F, 0.0F, 0.0F };
};
