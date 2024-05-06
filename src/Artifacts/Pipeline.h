#pragma once

#include "../Types.h"

#include <cstdint>
#include <memory>
#include <string>

class CtStructureTree;
class ImageArtifactConcatenation;
class StructureArtifactList;
class TreeStructureArtifactListCollection;

struct CtStructureTreeEvent;

class Pipeline {
public:
    explicit Pipeline(CtStructureTree const& structureTree);
    ~Pipeline();

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetStructureArtifactListCollectionForIdx(uint16_t structureIdx) const -> StructureArtifactList&;

    [[nodiscard]] auto
    GetTreeArtifacts() const -> TreeStructureArtifactListCollection&;

    [[nodiscard]] auto
    GetImageArtifactConcatenation() const -> ImageArtifactConcatenation&;

    auto ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) const -> void;

    auto operator==(const Pipeline& other) const noexcept -> bool;

    std::string Name;
    std::unique_ptr<TreeStructureArtifactListCollection> TreeStructureArtifacts;
    std::unique_ptr<ImageArtifactConcatenation> ImageArtifactConcat;
};
