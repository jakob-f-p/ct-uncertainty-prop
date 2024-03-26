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

void ImageArtifact::SetParent(ImageArtifactComposition* parent) {
    Parent = parent;
}

bool ImageArtifact::IsComposition() const {
    return GetArtifactSubType() == IMAGE_COMPOSITION;
}

ImageArtifactDetails ImageArtifact::GetImageArtifactDetails() {
    return {
        GetArtifactDetails(),
        QString::fromStdString(GetViewName()),
        {},
        {}
    };
}

std::string ImageArtifact::GetViewName() {
    std::string subTypeFullName = SubTypeToString(GetArtifactSubType());
    std::string subTypeViewName = subTypeFullName.erase(0, subTypeFullName.find(' ') + 1);
    std::string viewName = subTypeViewName + (Name.empty() ? "" : (" (" + Name + ")"));
    return viewName;
}

void ImageArtifact::SetChildEditWidgetData(QWidget* widget, const ArtifactDetails& artifactDetails) const {
    const auto& imageArtifactDetails = dynamic_cast<const ImageArtifactDetails&>(artifactDetails);
    SetImageArtifactChildEditWidgetData(widget, imageArtifactDetails);
}

void ImageArtifact::SetChildData(const ArtifactDetails& artifactDetails) {
    const auto& imageArtifactDetails = dynamic_cast<const ImageArtifactDetails&>(artifactDetails);
    SetImageArtifactChildData(imageArtifactDetails);
}
