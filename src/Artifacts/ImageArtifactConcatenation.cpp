#include "ImageArtifactConcatenation.h"

#include "ImageArtifact.h"
#include "Filters/PassThroughImageArtifactFilter.h"
#include "../Modeling/CtDataSource.h"
#include "../App.h"

#include <vtkImageAlgorithm.h>

ImageArtifactConcatenation::ImageArtifactConcatenation() noexcept :
        Start(new ImageArtifact { CompositeImageArtifact(CompositeImageArtifact::CompositionType::SEQUENTIAL) }) {
    StartFilter->SetDataTree(&App::GetInstance()->GetCtDataTree());
}

ImageArtifactConcatenation::~ImageArtifactConcatenation() = default;

auto ImageArtifactConcatenation::ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool {
    return Start->ContainsImageArtifact(imageArtifact);
}

auto ImageArtifactConcatenation::AddImageArtifact(ImageArtifact&& imageArtifact,
                                                  ImageArtifact* parent,
                                                  int insertionIdx) -> ImageArtifact& {
    if (ContainsImageArtifact(imageArtifact) || (parent && !parent->IsComposite()))
        throw std::runtime_error("Cannot add given image artifact");

    if (!parent) {
        imageArtifact.SetParent(Start.get());
        return Start->ToComposite().AddImageArtifact(std::move(imageArtifact), insertionIdx);
    }

    imageArtifact.SetParent(parent);

    auto& addedArtifact = parent->ToComposite().AddImageArtifact(std::move(imageArtifact), insertionIdx);

    EmitEvent();

    return addedArtifact;
}

void ImageArtifactConcatenation::RemoveImageArtifact(ImageArtifact& imageArtifact) {
    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot remove given image artifact because it does not exist within this image artifact concatenation");

    if (imageArtifact.IsComposite() && Start.get() == &imageArtifact)
        throw std::runtime_error("Cannot remove root image artifact");

    imageArtifact.GetParent()->ToComposite().RemoveImageArtifact(imageArtifact);

    EmitEvent();
}

void ImageArtifactConcatenation::MoveChildImageArtifact(const ImageArtifact& imageArtifact, int newIdx) {
    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot move given image artifact because it does not exist within this image artifact concatenation");

    if (imageArtifact.IsComposite() && Start.get() == &imageArtifact)
        throw std::runtime_error("Cannot move root image artifact");

    imageArtifact.GetParent()->ToComposite().MoveChildImageArtifact(imageArtifact, newIdx);

    EmitEvent();
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

auto ImageArtifactConcatenation::IndexOf(const ImageArtifact& imageArtifact) const -> uint16_t {
    uint16_t currentIdx = 0;
    uint32_t const idx = Start->IndexOf(imageArtifact, currentIdx);
    if (idx == -1)
        throw std::runtime_error("Given image artifact does not exist in this concatenation");

    return static_cast<uint16_t>(idx);
}

auto ImageArtifactConcatenation::GetArtifactFilter() const -> vtkImageAlgorithm& {
    return *EndFilter;
}

auto ImageArtifactConcatenation::UpdateArtifactFilter() -> void {
    auto& secondToLastFilter = Start->AppendImageFilters(*StartFilter);
    EndFilter->SetInputConnection(secondToLastFilter.GetOutputPort());
}

void
ImageArtifactConcatenation::AddEventCallback(void* receiver, ImageArtifactConcatenation::EventCallback&& callback) {
    if (receiver == nullptr)
        throw std::runtime_error("receiver may not be null");

    CallbackMap.emplace(receiver, std::move(callback));
}

auto ImageArtifactConcatenation::EmitEvent() -> void {
    UpdateArtifactFilter();

    for (auto const& [receiver, callback] : CallbackMap)
        callback();
}
