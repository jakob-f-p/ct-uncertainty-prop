#include "ImageArtifactConcatenation.h"

#include "CompositeArtifact.h"

ImageArtifactConcatenation::ImageArtifactConcatenation() noexcept :
        Start() {
    Start->SetCompType(CompositeArtifact::CompositionType::SEQUENTIAL);
}

bool ImageArtifactConcatenation::ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept {
    return Start->ContainsImageArtifact(imageArtifact);
}

void ImageArtifactConcatenation::AddImageArtifact(ImageArtifact& imageArtifact, CompositeArtifact* parent) {
    if (ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot add given image artifact because it already exists within this image artifact concatenation");

    if (!parent) {
        Start->AddImageArtifact(imageArtifact);
        return;
    }

    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot add given image artifact because given parent does not exist within this image artifact concatenation");

    parent->AddImageArtifact(imageArtifact);
}

void ImageArtifactConcatenation::RemoveImageArtifact(ImageArtifact& imageArtifact) {
    if (!ContainsImageArtifact(imageArtifact))
        throw std::runtime_error("Cannot remove given image artifact because it does not exist within this image artifact concatenation");

    if (static_cast<ImageArtifact*>(Start) == &imageArtifact)
        throw std::runtime_error("Cannot remove root image artifact");

    imageArtifact.GetParent()->RemoveImageArtifact(imageArtifact);
}

CompositeArtifact& ImageArtifactConcatenation::GetStart() noexcept {
    return *Start;
}
