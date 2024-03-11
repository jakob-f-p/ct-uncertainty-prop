#include "Artifact.h"

void Artifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "Name: '" << name << "'\n";
}

std::string Artifact::getName() {
    return name;
}

std::vector<Artifact::SubType> Artifact::getImageArtifactTypes() {
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

std::vector<Artifact::SubType> Artifact::getStructureArtifactTypes() {
    return {
        STRUCTURE_STREAKING,
        STRUCTURE_METALLIC,
        STRUCTURE_MOTION
    };
}
