#pragma once

#include "StructureArtifact.h"
#include "../../Utils/Types.h"

#include <vtkImageAlgorithm.h>

class CtStructureTree;
class TreeStructureArtifactListCollection;

class StructureArtifactsFilter : public vtkImageAlgorithm {
public:
    StructureArtifactsFilter(const StructureArtifactsFilter&) = delete;
    void operator=(const StructureArtifactsFilter&) = delete;

    static StructureArtifactsFilter* New();
    vtkTypeMacro(StructureArtifactsFilter, vtkImageAlgorithm)

    auto
    SetStructureArtifactCollection(TreeStructureArtifactListCollection* structureArtifactListCollection) -> void;

    [[nodiscard]] auto
    GetStructureArtifactCollection() const -> TreeStructureArtifactListCollection*;

    struct Algorithm {
        StructureArtifactsFilter* Self;
        vtkImageData* VolumeData;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
        TreeStructureArtifactListCollection* TreeArtifacts;
        float* Radiodensities;
        StructureId const* BasicStructureIds;
        std::array<float* const, 3> StructureArtifactValues;

        Algorithm(StructureArtifactsFilter* self,
                  vtkImageData* volumeData,
                  TreeStructureArtifactListCollection* treeArtifacts,
                  float* radiodensities,
                  StructureId const* basicStructureIds,
                  std::array<float* const, 3> structureArtifactValues);

        void operator()(vtkIdType pointId, vtkIdType endPointId) const;
    };

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

protected:
    StructureArtifactsFilter() = default;
    ~StructureArtifactsFilter() override = default;

    using SubType = StructureArtifact::SubType;

    vtkExecutive* CreateDefaultExecutive() override;

    int RequestInformation(vtkInformation* request,
                           vtkInformationVector** inputVector,
                           vtkInformationVector* outputVector) override;

    auto RequestData(vtkInformation* request,
                     vtkInformationVector** inputVector,
                     vtkInformationVector* outputVector) -> int override;

private:
    [[nodiscard]] auto static
    GetArrayName(SubType subType) noexcept -> std::string;

    TreeStructureArtifactListCollection* StructureArtifactCollection = nullptr;
    CtStructureTree* StructureTree = nullptr;
};
