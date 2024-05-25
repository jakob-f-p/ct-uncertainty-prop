#pragma once

#include "ObjectProperty.h"

#include <string>

class ImageArtifact;
class StructureArtifact;

struct ArtifactVariantPointer {

    ArtifactVariantPointer() : ArtifactPointer(static_cast<ImageArtifact*>(nullptr)) {};

    template<typename PointerType>
    requires std::is_pointer_v<PointerType>
    explicit ArtifactVariantPointer(PointerType artifactPointer)
            : ArtifactPointer(artifactPointer) {}

    [[nodiscard]] auto
    GetVariant() const noexcept -> std::variant<ImageArtifact*, StructureArtifact*> const&;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetProperties() -> PipelineParameterProperties;

    [[nodiscard]] auto
    IsNullptr() const noexcept -> bool;

    [[nodiscard]] auto
    operator ==(ArtifactVariantPointer const& other) const noexcept -> bool = default;

private:
    std::variant<ImageArtifact*, StructureArtifact*> ArtifactPointer;
};
