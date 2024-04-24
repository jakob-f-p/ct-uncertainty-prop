#pragma once

#include <memory>

class CompositeImageArtifact;
class ImageArtifact;

class ImageArtifactConcatenation {
public:
    ImageArtifactConcatenation() noexcept;
    ~ImageArtifactConcatenation();

    [[nodiscard]] auto
    ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool;

    auto
    AddImageArtifact(ImageArtifact&& imageArtifact,
                     ImageArtifact* parent = nullptr,
                     int insertionIdx = -1) -> ImageArtifact&;

    auto
    RemoveImageArtifact(ImageArtifact& imageArtifact) -> void;

    [[nodiscard]] auto
    GetStart() noexcept -> CompositeImageArtifact&;

    [[nodiscard]] auto
    Get(uint16_t idx) -> ImageArtifact&;

    [[nodiscard]] auto
    IndexOf(const ImageArtifact& imageArtifact) const -> uint16_t;

private:
    std::unique_ptr<CompositeImageArtifact> Start;
};
