#include "ImageArtifact.h"
#include "ImageArtifactDetails.h"

void ImageArtifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

Artifact::Type ImageArtifact::GetArtifactType() const {
    return Type::IMAGE_ARTIFACT;
}

ImageArtifact::ImageArtifact() :
        Parent(nullptr) {
}

ImageArtifactComposition* ImageArtifact::GetParent() {
    return Parent;
}

ImageArtifactDetails ImageArtifact::GetImageArtifactDetails() {
    std::string subTypeFullName = SubTypeToString(GetArtifactSubType());
    std::string subTypeViewName = subTypeFullName.erase(0, subTypeFullName.find(' ') + 1);
    return {
        GetArtifactDetails(),
        QString::fromStdString(subTypeViewName),
//        {}
    };
}
