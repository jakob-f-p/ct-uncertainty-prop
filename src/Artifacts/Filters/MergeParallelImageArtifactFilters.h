#pragma once

#include "ImageArtifactFilter.h"

class MergeParallelImageArtifactFilters : public ImageArtifactFilter {
public:
    MergeParallelImageArtifactFilters(const MergeParallelImageArtifactFilters&) = delete;
    void operator=(const MergeParallelImageArtifactFilters&) = delete;

    static MergeParallelImageArtifactFilters* New();
    vtkTypeMacro(MergeParallelImageArtifactFilters, ImageArtifactFilter);

    auto
    SetBaseFilterConnection(vtkAlgorithmOutput* input) noexcept -> void;

    auto
    AddParallelFilterConnection(vtkAlgorithmOutput* input) noexcept -> void;

protected:
    MergeParallelImageArtifactFilters();
    ~MergeParallelImageArtifactFilters() override = default;

    int FillInputPortInformation(int port, vtkInformation* info) override;

    int RequestInformation(vtkInformation *request,
                           vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;

    auto RequestData(vtkInformation* request,
                     vtkInformationVector** inputVector,
                     vtkInformationVector* outputVector) -> int override;
private:
    [[nodiscard]] auto static
    InfoPointDataInformationVectorHasArray(vtkInformationVector* infos, Artifact::SubType subType) -> bool;

    [[nodiscard]] auto static
    GetContainedSubTypes(vtkInformation* baseInInfo, vtkInformationVector* parallelInInfos) noexcept
    -> std::vector<Artifact::SubType>;
};
