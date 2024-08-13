#include "ImageArtifactConcatenation.h"

#include <utility>

#include "ImageArtifact.h"
#include "../Pipeline.h"
#include "Filters/PassThroughImageArtifactFilter.h"

ImageArtifactConcatenation::ImageArtifactConcatenation(BeforeRemoveArtifactCallback callback) noexcept :
        BeforeRemoveCallback(std::move(callback)),
        Start(new ImageArtifact { std::move(CompositeImageArtifact(
                CompositeImageArtifact::CompositionType::SEQUENTIAL)) }) {}

ImageArtifactConcatenation::~ImageArtifactConcatenation() = default;

auto ImageArtifactConcatenation::ContainsImageArtifact(ImageArtifact const& imageArtifact) const noexcept -> bool {
    return Start->ContainsImageArtifact(imageArtifact);
}

auto ImageArtifactConcatenation::AddImageArtifact(ImageArtifact&& imageArtifact,
                                                  ImageArtifact* parent,
                                                  int insertionIdx) -> ImageArtifact& {
    if (ContainsImageArtifact(imageArtifact) || (parent && !parent->IsComposite()))
        throw std::runtime_error("Cannot add given image artifact");

    ImageArtifact& realParent = parent ? *parent : *Start;

    imageArtifact.SetParent(&realParent);

    auto& addedArtifact = realParent.ToComposite().AddImageArtifact(std::move(imageArtifact), insertionIdx);

    EmitEvent();

    return addedArtifact;
}

void ImageArtifactConcatenation::RemoveImageArtifact(ImageArtifact& imageArtifact) {
    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot remove given image artifact because it does not exist within this"
                                 "image artifact concatenation");

    if (imageArtifact.IsComposite() && Start.get() == &imageArtifact)
        throw std::runtime_error("Cannot remove root image artifact");

    BeforeRemoveCallback(imageArtifact);

    imageArtifact.GetParent()->ToComposite().RemoveImageArtifact(imageArtifact);

    EmitEvent();
}

void ImageArtifactConcatenation::MoveChildImageArtifact(ImageArtifact const& imageArtifact, int newIdx) {
    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot move given image artifact because it does not exist within this"
                                 "image artifact concatenation");

    if (imageArtifact.IsComposite() && Start.get() == &imageArtifact)
        throw std::runtime_error("Cannot move root image artifact");

    imageArtifact.GetParent()->ToComposite().MoveChildImageArtifact(imageArtifact, newIdx);

    EmitEvent();
}

auto ImageArtifactConcatenation::GetStartFilter() const -> vtkImageAlgorithm& {
    return *StartFilter;
}

auto ImageArtifactConcatenation::GetEndFilter() const -> vtkImageAlgorithm& {
    return *EndFilter;
}

auto ImageArtifactConcatenation::UpdateArtifactFilter() -> void {
    auto& secondToLastFilter = Start->AppendImageFilters(*StartFilter);
    EndFilter->SetInputConnection(secondToLastFilter.GetOutputPort());
}

auto ImageArtifactConcatenation::GetFilterMTime() const noexcept -> vtkMTimeType {
    return EndFilter->GetMTime();
}

auto ImageArtifactConcatenation::GetStart() noexcept -> ImageArtifact& {
    return *Start;
}

auto ImageArtifactConcatenation::Get(uint16_t idx) -> ImageArtifact& {
    uint16_t currentIdx = 0;
    ImageArtifact* imageArtifact = Start->Get(idx, currentIdx);
    if (!imageArtifact)
        throw std::runtime_error("No image artifact with the given index exists in this concatenation");

    return *imageArtifact;
}

auto ImageArtifactConcatenation::IndexOf(ImageArtifact const& imageArtifact) const -> uint16_t {
    uint16_t currentIdx = 0;
    uint32_t const idx = Start->IndexOf(imageArtifact, currentIdx);
    if (idx == -1)
        throw std::runtime_error("Given image artifact does not exist in this concatenation");

    return static_cast<uint16_t>(idx);
}

auto ImageArtifactConcatenation::EmitEvent() -> void {
    UpdateArtifactFilter();
}
