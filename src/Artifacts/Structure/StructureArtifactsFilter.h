#pragma once

#include "StructureArtifact.h"
#include "../../Utils/LinearAlgebraTypes.h"

#include <vtkImageAlgorithm.h>

class CtStructureTree;
class CtStructureVariant;
class TreeStructureArtifactListCollection;


class StructureArtifactsFilter : public vtkImageAlgorithm {
public:
    StructureArtifactsFilter(const StructureArtifactsFilter&) = delete;
    void operator=(const StructureArtifactsFilter&) = delete;

    static StructureArtifactsFilter* New();
    vtkTypeMacro(StructureArtifactsFilter, vtkImageAlgorithm)

    auto
    SetCtStructureTree(CtStructureTree const& ctStructureTree) -> void;

    auto
    SetStructureArtifactCollection(TreeStructureArtifactListCollection* structureArtifactListCollection) -> void;

    [[nodiscard]] auto
    GetStructureArtifactCollection() const -> TreeStructureArtifactListCollection*;

    struct Algorithm {
        StructureArtifactsFilter* Self;
        CtStructureTree const* StructureTree;
        vtkImageData* VolumeData;
        CtStructureVariant const* StructureVariant;
        float MaxStructureRadiodensity;
        StructureArtifact const* StructureArtifact_;
        std::array<double, 3> Spacing;
        std::array<int, 3> UpdateDims;
        DoublePoint StartPoint;
        float* Radiodensities;
        StructureId const* BasicStructureIds;
        std::vector<StructureId> StructureArtifactIds;
        float* const ArtifactValues;

        Algorithm(StructureArtifactsFilter* self,
                  CtStructureTree const* structureTree,
                  vtkImageData* volumeData,
                  CtStructureVariant const* structureVariant,
                  StructureArtifact const* structureArtifact,
                  float* radiodensities,
                  StructureId const* basicStructureIds,
                  float* const artifactValues);

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
    CtStructureTree const* StructureTree = nullptr;
};
