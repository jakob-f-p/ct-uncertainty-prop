#pragma once

#include "ImageArtifactFilter.h"

class SaltPepperArtifactFilter : public ImageArtifactFilter {
public:
    SaltPepperArtifactFilter(const SaltPepperArtifactFilter&) = delete;
    void operator=(const SaltPepperArtifactFilter&) = delete;

    static SaltPepperArtifactFilter* New();
    vtkTypeMacro(SaltPepperArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(SaltAmount, float, 0.0, 1.0);
    vtkGetMacro(SaltAmount, float);

    vtkSetClampMacro(PepperAmount, float, 0.0, 1.0);
    vtkGetMacro(PepperAmount, float);

    vtkSetClampMacro(SaltIntensityValue, float, 0.0, 2000.0);
    vtkGetMacro(SaltIntensityValue, float);

    vtkSetClampMacro(PepperIntensityValue, float, -2000.0, 0.0);
    vtkGetMacro(PepperIntensityValue, float);


protected:
    SaltPepperArtifactFilter() = default;
    ~SaltPepperArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto
    ExecuteDataWithImageInformation(vtkImageData* input, vtkImageData* output, vtkInformation* outInfo) -> void override;

    float SaltAmount;
    float PepperAmount;

    float SaltIntensityValue;
    float PepperIntensityValue;
};
