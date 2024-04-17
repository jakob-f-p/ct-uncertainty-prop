#pragma once

#include "Artifact.h"

#include <vtkSmartPointer.h>

class CtStructureTree;
class StructureArtifact;

using uidx_t = uint16_t;

class StructureArtifacts {
public:
    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

    [[nodiscard]] auto
    Get(int idx) const -> StructureArtifact&;

    [[nodiscard]] auto
    GetNumberOfArtifacts() const noexcept -> uidx_t;

    auto
    AddStructureArtifact(StructureArtifact& structureArtifact, int insertionIdx = -1) -> void;

    auto
    RemoveStructureArtifact(StructureArtifact& structureArtifact) -> void;

    auto
    MoveStructureArtifact(StructureArtifact* artifact, int newIdx) -> void;

    using TypeToStructureArtifactsMap = std::map<Artifact::SubType, std::vector<StructureArtifact*>>;
    TypeToStructureArtifactsMap GetStructureArtifactMap();

    auto
    AddArtifactValuesAtPositionToMap(const double x[3], std::map<Artifact::SubType, float>& artifactValueMap) -> void;

private:
    friend class TreeStructureArtifactCollection;

    using StructureArtifactList = std::vector<vtkSmartPointer<StructureArtifact>>;

    StructureArtifactList StructureArtifacts;
};


class TreeStructureArtifactCollection {
public:
    [[nodiscard]] auto
    GetMTime() -> vtkMTimeType;

    [[nodiscard]] auto
    GetForCtStructureIdx(uidx_t structureIdx) -> StructureArtifacts&;

    auto
    AddStructureArtifactList(uidx_t insertionIdx) -> void;

    auto
    RemoveStructureArtifactList(uidx_t removeIdx) -> void;

private:
    std::vector<StructureArtifacts> ArtifactLists;
};