#pragma once

#include "CompositeArtifact.h"

#include <vtkNew.h>

class ImageArtifact;
class CompositeArtifact;

class ImageArtifactConcatenation {
public:
    ImageArtifactConcatenation() noexcept;

    [[nodiscard]] auto
    ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool;

    auto
    AddImageArtifact(ImageArtifact& imageArtifact, CompositeArtifact* parent = nullptr) -> void;

    auto
    RemoveImageArtifact(ImageArtifact& imageArtifact) -> void;

    [[nodiscard]] auto
    GetStart() noexcept -> CompositeArtifact&;

private:
    vtkNew<CompositeArtifact> Start;
};
