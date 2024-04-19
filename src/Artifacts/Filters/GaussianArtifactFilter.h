#pragma once

#include "ImageArtifactFilter.h"

class GaussianArtifactFilter : public ImageArtifactFilter {
public:
    static GaussianArtifactFilter* New();
    vtkTypeMacro(GaussianArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(Mean, double, -150.0, 150.0);
    vtkGetMacro(Mean, double);

    vtkSetClampMacro(Sd, double, -150.0, 100.0);
    vtkGetMacro(Sd, double);

    GaussianArtifactFilter(const GaussianArtifactFilter&) = delete;
    void operator=(const GaussianArtifactFilter&) = delete;

protected:
    GaussianArtifactFilter() = default;
    ~GaussianArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto ExecuteDataWithImageInformation(vtkImageData* output, vtkInformation* outInfo) -> void override;

    double Mean;
    double Sd;
};
