#include "Artifact.h"

void Artifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "Name: '" << Name << "'\n";
}

std::string Artifact::GetName() {
    return Name;
}

std::vector<Artifact::SubType> Artifact::GetImageArtifactTypes() {
    return {
        IMAGE_GAUSSIAN,
        IMAGE_SALT_PEPPER,
        IMAGE_RING,
        IMAGE_CUPPING,
        IMAGE_WIND_MILL,
        IMAGE_STAIR_STEP,
        IMAGE_STREAKING
    };
}

std::vector<Artifact::SubType> Artifact::GetStructureArtifactTypes() {
    return {
        STRUCTURE_STREAKING,
        STRUCTURE_METALLIC,
        STRUCTURE_MOTION
    };
}
