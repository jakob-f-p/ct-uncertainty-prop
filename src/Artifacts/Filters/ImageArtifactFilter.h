#pragma once

#include "../Artifact.h"

#include <vtkImageAlgorithm.h>

class vtkFloatArray;

class ImageArtifactFilter : public vtkImageAlgorithm {
public:
    auto static
    AddArrayInformationToPointDataVector(Artifact::SubType subType,
                                         vtkInformationVector* outputVector) -> void;

protected:
    auto RequestInformation(vtkInformation* request,
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector) -> int override;

    auto RequestData(vtkInformation* request,
                     vtkInformationVector** inputVector,
                     vtkInformationVector* outputVector) -> int override;

    virtual auto
    ExecuteDataWithImageInformation(vtkImageData* output, vtkInformation* outInfo) -> void;

    [[nodiscard]] auto static
    GetRadiodensitiesArray(vtkImageData* output) noexcept -> vtkFloatArray*;

    [[nodiscard]] auto static
    GetArtifactArray(vtkImageData* output, Artifact::SubType subType) noexcept -> vtkFloatArray*;

    [[nodiscard]] auto static
    PointDataInformationVectorHasArray(vtkInformation* info, Artifact::SubType subType) -> bool;

    auto
    ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) -> void override;

    auto CreateDefaultExecutive() -> vtkExecutive* override;

private:
    [[nodiscard]] auto static
    GetArrayName(Artifact::SubType subType) noexcept -> std::string;
};
