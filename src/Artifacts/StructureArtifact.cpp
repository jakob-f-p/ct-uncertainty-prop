#include "StructureArtifact.h"

void StructureArtifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "ArtifactType: Structure Artifact\n";
}

Artifact::Type StructureArtifact::getArtifactType() const {
    return Type::STRUCTURE_ARTIFACT;
}
