#include "CompositeArtifact.h"

#include "Filters/MergeParallelImageArtifactFilters.h"

#include <QComboBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include <vtkImageAlgorithm.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(CompositeArtifact)

void CompositeArtifact::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

Artifact::SubType CompositeArtifact::GetArtifactSubType() const {
    return SubType::IMAGE_COMPOSITION;
}

std::string
CompositeArtifact::CompositionTypeToString(CompositeArtifact::CompositionType compositionType) {
    switch (compositionType) {
        case SEQUENTIAL: return "Sequential";
        case PARALLEL:   return "Parallel";
        default: {
            qWarning("no matching composition type");
            return "";
        }
    }
}

bool CompositeArtifact::ContainsImageArtifact(const ImageArtifact& artifact) {
    return this == &artifact
            || std::any_of(ImageArtifacts.begin(), ImageArtifacts.end(), [&](ImageArtifact* a) {
                return a->IsComposition()
                        ? dynamic_cast<CompositeArtifact*>(a)->ContainsImageArtifact(artifact)
                        : a == &artifact;
            });
}

void CompositeArtifact::AddImageArtifact(ImageArtifact& artifact, int idx) {
    if (idx == -1)
        ImageArtifacts.push_back(&artifact);
    else
        ImageArtifacts.insert(std::next(ImageArtifacts.begin(), idx), &artifact);

    artifact.SetParent(this);
}

void CompositeArtifact::RemoveImageArtifact(ImageArtifact& artifact) {
    auto artifactIt = std::find(ImageArtifacts.begin(), ImageArtifacts.end(), &artifact);

    if (artifactIt == ImageArtifacts.end()) {
        qWarning("Cannot remove image artifact from this composition since given image artifact is not contained in child artifacts");
        return;
    }

    ImageArtifacts.erase(artifactIt);
}

ImageArtifact* CompositeArtifact::ChildArtifact(int idx) {
    return ImageArtifacts.at(idx);
}

int CompositeArtifact::GetChildIdx(ImageArtifact& artifact) {
    uint64_t idx = std::distance(ImageArtifacts.begin(),
                                 std::find(ImageArtifacts.begin(), ImageArtifacts.end(), &artifact));
    return static_cast<int>(idx);
}

int CompositeArtifact::NumberOfChildren() {
    return static_cast<int>(ImageArtifacts.size());
}

void CompositeArtifact::MoveChildImageArtifact(ImageArtifact* imageArtifact, int newIdx) {
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

vtkImageAlgorithm& CompositeArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) {
    switch (CompType) {

        case SEQUENTIAL: {
            vtkImageAlgorithm* currentImageAlgorithm = &inputAlgorithm;

            for (auto& imageArtifact : ImageArtifacts)
                currentImageAlgorithm = &imageArtifact->AppendImageFilters(*currentImageAlgorithm);

            return *currentImageAlgorithm;
        }

        case PARALLEL: {
            if (!Filter)
                Filter = MergeParallelImageArtifactFilters::New();
            else
                Filter->RemoveAllInputs();

            Filter->SetBaseFilterConnection(inputAlgorithm.GetOutputPort());

            for (auto& imageArtifact : ImageArtifacts) {
                auto& appendedAlgorithm = imageArtifact->AppendImageFilters(inputAlgorithm);

                Filter->AddParallelFilterConnection(appendedAlgorithm.GetOutputPort());
            }

            return *Filter;
        }

        default: throw std::runtime_error("invalid composition type");
    }
}

void CompositeArtifactData::AddSubTypeData(const ImageArtifact& imageArtifact) {
    auto& artifact = dynamic_cast<const CompositeArtifact&>(imageArtifact);
    CompositionType = artifact.CompType;
}

void CompositeArtifactData::SetSubTypeData(ImageArtifact& imageArtifact) const {
    auto& artifact = dynamic_cast<CompositeArtifact&>(imageArtifact);
    artifact.CompType = CompositionType;
}


void CompositeArtifactUi::AddSubTypeWidgets(QFormLayout* fLayout) {
    auto* group = new QGroupBox("Composite");
    auto* hLayout = new QHBoxLayout(group);

    auto* compTypeLabel = new QLabel("Composition Type");
    hLayout->addWidget(compTypeLabel);
    auto* compTypeComboBox = new QComboBox();
    compTypeComboBox->setObjectName(CompTypeComboBoxObjectName);
    for (const auto &compTypeAndName : CompositeArtifact::GetCompositionTypeValues()) {
        compTypeComboBox->addItem(compTypeAndName.Name, compTypeAndName.EnumValue);
    }
    hLayout->addWidget(compTypeComboBox);

    fLayout->addRow(group);
}

void CompositeArtifactUi::AddSubTypeWidgetsData(QWidget* widget, CompositeArtifactData& data) {
    auto* compTypeComboBox = widget->findChild<QComboBox*>(CompTypeComboBoxObjectName);
    data.CompositionType = compTypeComboBox->currentData().value<CompositeArtifact::CompositionType>();
}

void CompositeArtifactUi::SetSubTypeWidgetsData(QWidget* widget, const CompositeArtifactData& data) {
    auto* compTypeComboBox = widget->findChild<QComboBox*>(CompTypeComboBoxObjectName);
    if (int idx = compTypeComboBox->findData(data.CompositionType);
            idx != -1)
        compTypeComboBox->setCurrentIndex(idx);
}

const QString CompositeArtifactUi::CompTypeComboBoxObjectName = "CompositionTypeComboBox";