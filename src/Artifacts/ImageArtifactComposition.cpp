#include "ImageArtifactComposition.h"

#include "ImageArtifactDetails.h"

#include <QHBoxLayout>
#include <QWidget>

#include <vtkObjectFactory.h>
#include <QComboBox>
#include <QLabel>

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
                return a->IsComposition()
                        ? dynamic_cast<ImageArtifactComposition*>(a)->ContainsImageArtifact(artifact)
                        : a == &artifact;
            });
}

void ImageArtifactComposition::AddImageArtifact(ImageArtifact& artifact, int idx) {
    if (idx == -1) {
        ImageArtifacts.push_back(&artifact);
    } else {
        ImageArtifacts.insert(std::next(ImageArtifacts.begin(), idx), &artifact);
    }
    artifact.SetParent(this);
    artifact.Register(this);
}

void ImageArtifactComposition::RemoveImageArtifact(ImageArtifact& artifact) {
    auto artifactIt = std::find(ImageArtifacts.begin(), ImageArtifacts.end(), &artifact);

    if (artifactIt == ImageArtifacts.end()) {
        qWarning("Cannot remove image artifact from this composition since given image artifact is not contained in child artifacts");
        return;
    }

    ImageArtifacts.erase(artifactIt);
    artifact.Delete();
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
    details.Composition.CompositionType = CompType;
    return QVariant::fromValue(details);
}

ImageArtifactDetails ImageArtifactComposition::GetImageArtifactEditWidgetData(QWidget* widget) const {
    auto* compTypeComboBox = widget->findChild<QComboBox*>(CompTypeComboBoxObjectName);
    return {
            GetArtifactEditWidgetData(widget),
            { compTypeComboBox->currentData().value<CompositionType>() },
            {}
    };
}

void ImageArtifactComposition::MoveChildImageArtifact(ImageArtifact* imageArtifact, int newIdx) {
    if (!imageArtifact || newIdx < 0 || newIdx >= ImageArtifacts.size()) {
        qWarning("Cannot move given image artifact to index");
        return;
    }

    auto currentIt = std::find(ImageArtifacts.begin(), ImageArtifacts.end(), imageArtifact);
    if (currentIt == ImageArtifacts.end()) {
        qWarning("Cannot move given image artifact to index");
        return;
    }
    auto currentIdx = std::distance(ImageArtifacts.begin(), currentIt);

    if (currentIdx == newIdx)
        return;

    auto newIt = std::next(ImageArtifacts.begin(), newIdx);
    if (currentIdx < newIdx) {
        std::rotate(currentIt, std::next(currentIt), std::next(newIt));
    } else {
        std::rotate(newIt, std::next(newIt), std::next(currentIt));
    }
}

ImageArtifactComposition::ImageArtifactComposition() :
        CompType(INVALID),
        CompTypeComboBoxObjectName("compTypeComboBox") {
}

ImageArtifactComposition::~ImageArtifactComposition() {
    for (const auto& imageArtifact: ImageArtifacts) {
        imageArtifact->Delete();
    }
}

QWidget* ImageArtifactComposition::GetChildEditWidget() const {
    auto* widget = new QWidget();
    auto* hLayout = new QHBoxLayout(widget);

    auto* compTypeLabel = new QLabel("Composition Type");
    hLayout->addWidget(compTypeLabel);
    auto* compTypeComboBox = new QComboBox();
    compTypeComboBox->setObjectName(CompTypeComboBoxObjectName);
    for (const auto &compTypeAndName : GetCompositionTypeValues()) {
        compTypeComboBox->addItem(compTypeAndName.Name, compTypeAndName.EnumValue);
    }
    hLayout->addWidget(compTypeComboBox);

    return widget;
}

void ImageArtifactComposition::SetImageArtifactChildEditWidgetData(QWidget* widget,
                                                                   const ImageArtifactDetails& details) const {
    auto* compTypeComboBox = widget->findChild<QComboBox*>(CompTypeComboBoxObjectName);
    if (int idx = compTypeComboBox->findData(details.Composition.CompositionType);
            idx != -1) {
        compTypeComboBox->setCurrentIndex(idx);
    }
}

void ImageArtifactComposition::SetImageArtifactChildData(const ImageArtifactDetails& details) {
    CompType = details.Composition.CompositionType;
}
