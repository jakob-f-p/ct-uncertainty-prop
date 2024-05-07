#pragma once

#include "ImageArtifactFilter.h"

#include <vtkImageResample.h>

class StairStepArtifactFilter : public ImageArtifactFilter {
public:
    StairStepArtifactFilter(const StairStepArtifactFilter&) = delete;
    void operator=(const StairStepArtifactFilter&) = delete;

    static StairStepArtifactFilter* New();
    vtkTypeMacro(StairStepArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(SamplingRate, float, 0.01, 1.0);
    vtkGetMacro(SamplingRate, float);

protected:
    StairStepArtifactFilter() = default;
    ~StairStepArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto
    ExecuteDataWithImageInformation(vtkImageData* input, vtkImageData* output, vtkInformation* outInfo) -> void override;

    float SamplingRate = 0.0F;
};
