#include "Artifact.h"

void Artifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Name: '" << Name << "'\n";
    os << indent << "ArtifactType: '" << TypeToString(GetArtifactType()) << "'\n";
    os << indent << "ArtifactSubType: '" << SubTypeToString(GetArtifactSubType()) << "'\n";
}

std::string Artifact::GetName() {
    return Name;
}

std::string Artifact::TypeToString(Artifact::Type type) {
    switch (type) {
        case IMAGE_ARTIFACT:     return "Image Artifact";
        case STRUCTURE_ARTIFACT: return "Structure Artifact";
        default: {
            qWarning("no matching artifact type");
            return "";
        }
    }
}

std::string Artifact::SubTypeToString(Artifact::SubType subType) {
    switch (subType) {
        case IMAGE_GAUSSIAN:      return "Image Gaussian";
        case IMAGE_SALT_PEPPER:   return "Image Salt and Pepper";
        case IMAGE_RING:          return "Image Ring";
        case IMAGE_CUPPING:       return "Image Cupping";
        case IMAGE_WIND_MILL:     return "Image Wind Mill";
        case IMAGE_STAIR_STEP:    return "Image Stair Step";
        case IMAGE_STREAKING:     return "Image Streaking";
        case IMAGE_COMPOSITION:   return "Image Composition";

        case STRUCTURE_STREAKING: return "Structure Streaking";
        case STRUCTURE_METALLIC:  return "Structure Metallic";
        case STRUCTURE_MOTION:    return "Structure Motion";
        default: {
            qWarning("no matching sub artifact type");
            return "";
        }
    }
}

ArtifactDetails Artifact::GetArtifactDetails() {
    return { QString::fromStdString(Name),
             GetArtifactType(),
             GetArtifactSubType() };
}
