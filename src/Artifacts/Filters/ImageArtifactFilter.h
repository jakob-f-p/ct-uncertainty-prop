#pragma once

#include "../Image/BasicImageArtifact.h"

#include <vtkImageAlgorithm.h>

class vtkFloatArray;

class ImageArtifactFilter : public vtkImageAlgorithm {
protected:
    using SubType = BasicImageArtifact::SubType;

public:
    auto static
    AddArrayInformationToPointDataVector(SubType subType,
                                         vtkInformationVector* outputVector) -> void;

protected:
    auto RequestInformation(vtkInformation* request,
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector) -> int override;

    auto RequestData(vtkInformation* request,
                     vtkInformationVector** inputVector,
                     vtkInformationVector* outputVector) -> int override;

    virtual auto
    ExecuteDataWithImageInformation(vtkImageData* input, vtkImageData* output, vtkInformation* outInfo) -> void;

    [[nodiscard]] auto static
    GetRadiodensitiesArray(vtkImageData* imageData) noexcept -> vtkFloatArray*;

    [[nodiscard]] auto static
    GetDeepCopiedRadiodensitiesArray(vtkImageData* input, vtkImageData* output) noexcept -> vtkFloatArray*;

    [[nodiscard]] auto static
    GetArtifactArray(vtkImageData* imageData, SubType subType) noexcept -> vtkFloatArray*;

    [[nodiscard]] auto static
    GetDeepCopiedArtifactArray(vtkImageData* input, vtkImageData* output, SubType subType) noexcept -> vtkFloatArray*;

    [[nodiscard]] auto static
    PointDataInformationVectorHasArray(vtkInformation* info, SubType subType) -> bool;

    auto
    ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) -> void override;

    auto CreateDefaultExecutive() -> vtkExecutive* override;

    ImageArtifactFilter() = default;
    ~ImageArtifactFilter() override = default;

private:
    [[nodiscard]] auto static
    GetArrayName(SubType subType) noexcept -> std::string;

    [[nodiscard]] auto static
    AddArtifactArray(vtkImageData* imageData, SubType subType) noexcept -> vtkFloatArray*;
};
