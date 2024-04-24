#pragma once

#include "../Types.h"

#include <memory>
#include <vector>

class CtStructureTree;
class StructureArtifact;

using uidx_t = uint16_t;

class StructureArtifactList {
public:
    StructureArtifactList() = default;
    StructureArtifactList(StructureArtifactList const&) = default;
    StructureArtifactList(StructureArtifactList&&) = default;
    auto operator= (StructureArtifactList const&) -> StructureArtifactList& = default;
    auto operator= (StructureArtifactList&&) -> StructureArtifactList& = default;
    ~StructureArtifactList();

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
    MoveStructureArtifact(StructureArtifact& artifact, int newIdx) -> void;

//    using TypeToStructureArtifactsMap = std::map<SubType, std::vector<StructureArtifact*>>;
//    TypeToStructureArtifactsMap GetStructureArtifactMap();
//
//    auto
//    AddArtifactValuesAtPositionToMap(const double x[3], std::map<SubType, float>& artifactValueMap) -> void;

private:
    friend class TreeStructureArtifactListCollection;

    using StructureArtifacts = std::vector<std::unique_ptr<StructureArtifact>>;

    StructureArtifacts Artifacts;
};


class TreeStructureArtifactListCollection {
public:
    [[nodiscard]] auto
    GetMTime() -> vtkMTimeType;

    [[nodiscard]] auto
    GetForCtStructureIdx(uidx_t structureIdx) -> StructureArtifactList&;

    auto
    AddStructureArtifactList(uidx_t insertionIdx) -> void;

    auto
    RemoveStructureArtifactList(uidx_t removeIdx) -> void;

private:
    std::vector<StructureArtifactList> ArtifactLists;
};