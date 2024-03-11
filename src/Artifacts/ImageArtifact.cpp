#include "ImageArtifact.h"

void ImageArtifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "ArtifactType: Image Artifact\n";
}

Artifact::Type ImageArtifact::GetArtifactType() const {
    return Type::IMAGE_ARTIFACT;
}
