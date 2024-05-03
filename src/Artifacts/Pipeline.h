#pragma once

#include "../Types.h"

#include <cstdint>
#include <memory>
#include <string>

class StructureArtifactList;
class ImageArtifactConcatenation;
class TreeStructureArtifactListCollection;

struct CtStructureTreeEvent;

class Pipeline {
public:
    explicit Pipeline(uidx_t structureCount);
    ~Pipeline();

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetStructureArtifactListCollectionForIdx(uint16_t structureIdx) const -> StructureArtifactList&;

    [[nodiscard]] auto
    GetImageArtifactConcatenation() const -> ImageArtifactConcatenation&;

    auto ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) const -> void;

    auto operator==(const Pipeline& other) const noexcept -> bool;

    std::string Name;
    std::unique_ptr<TreeStructureArtifactListCollection> TreeStructureArtifacts;
    std::unique_ptr<ImageArtifactConcatenation> ImageArtifactConcat;
};
