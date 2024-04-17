#pragma once

#include "ImageArtifactConcatenation.h"
#include "StructureWrapper.h"

#include <cstdint>
#include <memory>
#include <string>

class StructureArtifacts;
class ImageArtifactConcatenation;
class TreeStructureArtifactCollection;

struct CtStructureTreeEvent;

using uidx_t = uint16_t;

class Pipeline {
public:
    explicit Pipeline(uidx_t structureCount);

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetArtifactStructureWrapper(uidx_t structureIdx) const -> StructureArtifacts&;

    [[nodiscard]] auto
    GetImageArtifactConcatenation() const -> ImageArtifactConcatenation&;

    auto ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) -> void;

    auto operator==(const Pipeline& other) const noexcept -> bool;

    std::string Name;
    std::unique_ptr<TreeStructureArtifactCollection> TreeStructureArtifacts;
    std::unique_ptr<ImageArtifactConcatenation> ImageArtifactConcat;
};
