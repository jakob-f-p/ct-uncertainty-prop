#include "ImageArtifactConcatenation.h"

#include "ImageArtifact.h"

ImageArtifactConcatenation::ImageArtifactConcatenation() noexcept :
        Start(new CompositeImageArtifact(CompositeImageArtifact::CompositionType::SEQUENTIAL)) {}

ImageArtifactConcatenation::~ImageArtifactConcatenation() = default;

auto ImageArtifactConcatenation::ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool {
    return Start->ContainsImageArtifact(imageArtifact);
}

auto ImageArtifactConcatenation::AddImageArtifact(ImageArtifact&& imageArtifact,
                                                  ImageArtifact* parent,
                                                  int insertionIdx) -> ImageArtifact& {
    if (ContainsImageArtifact(imageArtifact) || (parent && !parent->IsComposite()))
        throw std::runtime_error("Cannot add given image artifact");

    if (!parent)
        return Start->AddImageArtifact(std::move(imageArtifact), insertionIdx);

    imageArtifact.SetParent(parent);

    return parent->ToComposite().AddImageArtifact(std::move(imageArtifact), insertionIdx);
}

void ImageArtifactConcatenation::RemoveImageArtifact(ImageArtifact& imageArtifact) {
    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot remove given image artifact because it does not exist within this image artifact concatenation");

    if (imageArtifact.IsComposite() && Start.get() == &imageArtifact.ToComposite())
        throw std::runtime_error("Cannot remove root image artifact");

    imageArtifact.GetParent()->ToComposite().RemoveImageArtifact(imageArtifact);
}

auto ImageArtifactConcatenation::GetStart() noexcept -> CompositeImageArtifact& {
    return *Start;
}

auto ImageArtifactConcatenation::Get(uint16_t idx) -> ImageArtifact& {
    uint16_t currentIdx = 1;
    ImageArtifact* imageArtifact = Start->Get(idx, currentIdx);
    if (!imageArtifact)
        throw std::runtime_error("No image artifact with the given index exists in this concatenation");

    return *imageArtifact;
}

auto ImageArtifactConcatenation::IndexOf(const ImageArtifact& imageArtifact) const -> uint16_t {
    uint16_t currentIdx = 1;
    uint16_t const idx = Start->IndexOf(imageArtifact, currentIdx);
    if (idx == 0)
        throw std::runtime_error("Given image artifact does not exist in this concatenation");

    return idx;
}
