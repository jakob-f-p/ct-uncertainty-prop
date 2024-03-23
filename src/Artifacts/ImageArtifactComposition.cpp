#include "ImageArtifactComposition.h"
#include "ImageArtifactDetails.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(ImageArtifactComposition)

void ImageArtifactComposition::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

Artifact::SubType ImageArtifactComposition::GetArtifactSubType() const {
    return IMAGE_COMPOSITION;
}

std::string
ImageArtifactComposition::CompositionTypeToString(ImageArtifactComposition::CompositionType compositionType) {
    switch (compositionType) {
        case SEQUENTIAL: return "Sequential";
        case PARALLEL:   return "Parallel";
        default: {
            qWarning("no matching composition type");
            return "";
        }
    }
}

bool ImageArtifactComposition::ContainsImageArtifact(const ImageArtifact& artifact) {
    return this == &artifact
            || std::any_of(ImageArtifacts.begin(), ImageArtifacts.end(), [&](ImageArtifact* a) {
                return a->GetArtifactSubType() == IMAGE_COMPOSITION
                        ? dynamic_cast<ImageArtifactComposition*>(a)->ContainsImageArtifact(artifact)
                        : a == &artifact;
            });
}

void ImageArtifactComposition::AddImageArtifact(ImageArtifact& artifact) {
    ImageArtifacts.push_back(&artifact);
    artifact.Register(this);
}

void ImageArtifactComposition::RemoveImageArtifact(ImageArtifact& artifact) {
    auto artifactIt = std::find(ImageArtifacts.begin(), ImageArtifacts.end(), &artifact);

    if (artifactIt == ImageArtifacts.end()) {
        qWarning("Cannot remove image artifact from this composition since given image artifact is not contained in child artifacts");
        return;
    }

    ImageArtifacts.erase(artifactIt);
}

ImageArtifact* ImageArtifactComposition::ChildArtifact(int idx) {
    return ImageArtifacts.at(idx);
}

int ImageArtifactComposition::GetChildIdx(ImageArtifact& artifact) {
    uint64_t idx = std::distance(ImageArtifacts.begin(),
                                 std::find(ImageArtifacts.begin(), ImageArtifacts.end(), &artifact));
    return static_cast<int>(idx);
}

int ImageArtifactComposition::NumberOfChildren() {
    return static_cast<int>(ImageArtifacts.size());
}

QVariant ImageArtifactComposition::Data() {
    ImageArtifactDetails details = GetImageArtifactDetails();
//    details.CompositionDetails.CompositionType = CompType;
    return QVariant::fromValue(details);
}

ImageArtifactComposition::ImageArtifactComposition() :
        CompType(INVALID) {
}

ImageArtifactComposition::~ImageArtifactComposition() {
    for (const auto& imageArtifact: ImageArtifacts) {
        imageArtifact->Delete();
    }
}
