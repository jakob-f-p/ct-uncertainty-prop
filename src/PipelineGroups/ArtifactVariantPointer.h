#pragma once

#include "ObjectProperty.h"

#include <string>
#include <unordered_map>
#include <variant>

class ImageArtifact;
class StructureArtifact;

struct ArtifactVariantPointer {

    ArtifactVariantPointer() : ArtifactPointer(static_cast<ImageArtifact*>(nullptr)) {};

    explicit ArtifactVariantPointer(auto&& artifactPointer)
            : ArtifactPointer(std::forward<decltype(artifactPointer)>(artifactPointer)) {}

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetProperties() -> PipelineParameterProperties;

    [[nodiscard]] auto
    IsNullptr() const noexcept -> bool;

    [[nodiscard]] auto
    operator ==(ArtifactVariantPointer const& other) const noexcept -> bool = default;

    struct Hash {
        auto operator()(ArtifactVariantPointer const& o) const noexcept;
    };

private:
    std::variant<ImageArtifact*, StructureArtifact*> ArtifactPointer;
};
