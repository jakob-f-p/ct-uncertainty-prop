#include "ImageArtifactConcatenation.h"

#include "CompositeArtifact.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(ImageArtifactConcatenation)

ImageArtifactConcatenation::ImageArtifactConcatenation() :
        Start() {
    Start->SetCompType(CompositeArtifact::CompositionType::SEQUENTIAL);
}

void ImageArtifactConcatenation::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

bool ImageArtifactConcatenation::ContainsImageArtifact(const ImageArtifact& imageArtifact) {
    return Start->ContainsImageArtifact(imageArtifact);
}

void ImageArtifactConcatenation::AddImageArtifact(ImageArtifact& imageArtifact, CompositeArtifact* parent) {
    if (ContainsImageArtifact(imageArtifact)) {
        qWarning("Cannot add given image artifact because it already exists within this image artifact concatenation");
        return;
    }

    if (!parent) {
        Start->AddImageArtifact(imageArtifact);
        return;
    }

    if (!ContainsImageArtifact(imageArtifact)) {
        qWarning("Cannot add given image artifact because given parent does not exist within this image artifact concatenation");
        return;
    }

    parent->AddImageArtifact(imageArtifact);
}

void ImageArtifactConcatenation::RemoveImageArtifact(ImageArtifact& imageArtifact) {
    if (!ContainsImageArtifact(imageArtifact)) {
        qWarning("Cannot remove given image artifact because it does not exist within this image artifact concatenation");
        return;
    }

    if (static_cast<ImageArtifact*>(Start) == &imageArtifact) {
        qWarning("Cannot remove root image artifact");
        return;
    }

    imageArtifact.GetParent()->RemoveImageArtifact(imageArtifact);
}

CompositeArtifact& ImageArtifactConcatenation::GetStart() {
    return *Start;
}
