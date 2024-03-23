#include "ImageArtifactConcatenation.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(ImageArtifactConcatenation)

ImageArtifactConcatenation::ImageArtifactConcatenation() :
        Start(*ImageArtifactComposition::New()) {
    Start.SetCompType(ImageArtifactComposition::CompositionType::SEQUENTIAL);
}

ImageArtifactConcatenation::~ImageArtifactConcatenation() {
    Start.Delete();
}

void ImageArtifactConcatenation::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

bool ImageArtifactConcatenation::ContainsImageArtifact(const ImageArtifact& imageArtifact) {
    return Start.ContainsImageArtifact(imageArtifact);
}

void ImageArtifactConcatenation::AddImageArtifact(ImageArtifact& imageArtifact, ImageArtifactComposition* parent) {
    if (ContainsImageArtifact(imageArtifact)) {
        qWarning("Cannot add given image artifact because it already exists within this image artifact concatenation");
        return;
    }

    if (!parent) {
        Start.AddImageArtifact(imageArtifact);
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

    if (static_cast<ImageArtifact*>(&Start) == &imageArtifact) {
        qWarning("Cannot remove root image artifact");
        return;
    }

    imageArtifact.GetParent()->RemoveImageArtifact(imageArtifact);
}

ImageArtifactComposition& ImageArtifactConcatenation::GetStart() {
    return Start;
}
