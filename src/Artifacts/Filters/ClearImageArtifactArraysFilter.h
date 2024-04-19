#pragma once

#include "ImageArtifactFilter.h"

class ClearImageArtifactArraysFilter : public ImageArtifactFilter {
public:
    ClearImageArtifactArraysFilter(const ClearImageArtifactArraysFilter&) = delete;
    void operator=(const ClearImageArtifactArraysFilter&) = delete;

    static ClearImageArtifactArraysFilter* New();
    vtkTypeMacro(ClearImageArtifactArraysFilter, ImageArtifactFilter);

protected:
    ClearImageArtifactArraysFilter() = default;
    ~ClearImageArtifactArraysFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto RequestData(vtkInformation* request, vtkInformationVector** inputVector,
                     vtkInformationVector* outputVector) -> int override;
};
