#include "ArtifactVariantPointer.h"

#include "../Artifacts/Image/ImageArtifact.h"
#include "../Artifacts/Structure/StructureArtifact.h"

auto ArtifactVariantPointer::GetName() const noexcept -> std::string {
    return std::visit([](auto* artifact) -> std::string { return artifact->GetViewName(); },
                      ArtifactPointer);
}

auto ArtifactVariantPointer::GetProperties() -> PipelineParameterProperties {
    if (IsNullptr())
        throw std::runtime_error("Artifact pointer must not be nullptr");

    return std::visit([](auto* artifact) -> PipelineParameterProperties { return artifact->GetProperties(); },
                      ArtifactPointer);
}

auto ArtifactVariantPointer::IsNullptr() const noexcept -> bool {
    return std::visit([](auto artifactP) { return artifactP == nullptr; },
                      ArtifactPointer);
}

auto ArtifactVariantPointer::Hash::operator()(const ArtifactVariantPointer& o) const noexcept {
    return std::visit([](auto artifactP) { return reinterpret_cast<uintptr_t>(artifactP); },
                      o.ArtifactPointer);
}
