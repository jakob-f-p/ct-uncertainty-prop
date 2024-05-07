#pragma once

#include "ImageArtifactFilter.h"

#include <vtkObjectFactory.h>

class PassThroughImageArtifactFilter : public ImageArtifactFilter {
public:
    PassThroughImageArtifactFilter(const PassThroughImageArtifactFilter&) = delete;
    void operator=(const PassThroughImageArtifactFilter&) = delete;

    static PassThroughImageArtifactFilter* New() { VTK_STANDARD_NEW_BODY(PassThroughImageArtifactFilter); }
    vtkTypeMacro(PassThroughImageArtifactFilter, ImageArtifactFilter);

protected:
    PassThroughImageArtifactFilter() = default;
    ~PassThroughImageArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override {
        return ImageArtifactFilter::RequestInformation(request, inputVector, outputVector);
    }

    auto
    ExecuteDataWithImageInformation(vtkImageData* input, vtkImageData* output, vtkInformation* outInfo) -> void override {}
};
