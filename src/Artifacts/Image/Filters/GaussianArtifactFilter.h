#pragma once

#include "ImageArtifactFilter.h"

class GaussianArtifactFilter : public ImageArtifactFilter {
public:
    static GaussianArtifactFilter* New();
    vtkTypeMacro(GaussianArtifactFilter, ImageArtifactFilter);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkSetClampMacro(Mean, double, -1000.0, 3000.0);
    vtkGetMacro(Mean, double);

    vtkSetClampMacro(Sd, double, 0.0000001, 1000.0);
    vtkGetMacro(Sd, double);

    GaussianArtifactFilter(const GaussianArtifactFilter&) = delete;
    void operator=(const GaussianArtifactFilter&) = delete;

protected:
    GaussianArtifactFilter() = default;
    ~GaussianArtifactFilter() override = default;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto
    ExecuteDataWithImageInformation(vtkImageData* input,
                                    vtkImageData* output,
                                    vtkInformation* outInfo) -> void override;

    double Mean = 0.0;
    double Sd = 1.0;
};
