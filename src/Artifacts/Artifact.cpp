#include "Artifact.h"

#include "GaussianArtifact.h"
#include "ImageArtifactComposition.h"
#include "MotionArtifact.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <utility>

void Artifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Name: '" << Name << "'\n";
    os << indent << "ArtifactType: '" << TypeToString(GetArtifactType()) << "'\n";
    os << indent << "ArtifactSubType: '" << SubTypeToString(GetArtifactSubType()) << "'\n";
}

std::string Artifact::GetName() {
    return Name;
}

void Artifact::SetName(const std::string& name) {
    Name = name;
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

Artifact* Artifact::NewArtifact(Artifact::SubType subType) {
    switch (subType) {
        case IMAGE_GAUSSIAN:     return GaussianArtifact::New();
//        case IMAGE_SALT_PEPPER:
//        case IMAGE_RING:
//        case IMAGE_CUPPING:
//        case IMAGE_WIND_MILL:
//        case IMAGE_STAIR_STEP:
//        case IMAGE_STREAKING:
        case IMAGE_COMPOSITION:  return ImageArtifactComposition::New();
//        case STRUCTURE_STREAKING:
//        case STRUCTURE_METALLIC:
        case STRUCTURE_MOTION:   return MotionArtifact::New();
        default: {
            qWarning("TODO: implement");
            return nullptr;
        }
    }
}

ArtifactDetails Artifact::GetArtifactDetails() const {
    return { QString::fromStdString(Name),
             GetArtifactType(),
             GetArtifactSubType() };
}

QWidget* Artifact::GetEditWidget() const {
    auto* widget = new QWidget();
    auto* vLayout = new QVBoxLayout(widget);

    auto* nameEditBar = new QWidget();
    nameEditBar->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
    auto* nameEditLayout = new QHBoxLayout(nameEditBar);
    auto* nameLineEditLabel = new QLabel("Name");
    nameLineEditLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName(NameLineEditObjectName);
    nameEditLayout->addWidget(nameLineEditLabel);
    nameEditLayout->addSpacing(20);
    nameEditLayout->addWidget(nameLineEdit);
    vLayout->addWidget(nameEditBar);

    auto* childEditWidget = GetChildEditWidget();
    childEditWidget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
    childEditWidget->setObjectName(ChildEditWidgetObjectName);
    vLayout->addWidget(childEditWidget);

    return widget;
}

void Artifact::SetEditWidgetData(QWidget* widget, const ArtifactDetails& artifactDetails) {
    if (!widget) {
        qWarning("Widget must not be nullptr");
        return;
    }

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameLineEditObjectName);
    nameLineEdit->setText(artifactDetails.Name);

    auto* childEditWidget = widget->findChild<QWidget*>(ChildEditWidgetObjectName);
    SetChildEditWidgetData(childEditWidget, artifactDetails);
}

ArtifactDetails Artifact::GetArtifactEditWidgetData(QWidget* widget) const {
    if (!widget) {
        qWarning("Widget must not be nullptr");
        return {};
    }

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameLineEditObjectName);

    return {
        nameLineEdit->text(),
        GetArtifactType(),
        GetArtifactSubType()
    };
}

void Artifact::SetData(const ArtifactDetails& artifactDetails) {
    Name = artifactDetails.Name.toStdString();

    SetChildData(artifactDetails);
}

ArtifactDetails::ArtifactDetails(QString name, Artifact::Type type, Artifact::SubType subType) :
        Name(std::move(name)),
        Type(type),
        SubType(subType) {
}

ArtifactDetails::ArtifactDetails() :
        Type(Artifact::IMAGE_ARTIFACT),
        SubType(Artifact::IMAGE_GAUSSIAN) {
}
const QString Artifact::NameLineEditObjectName = "nameEdit";

const QString Artifact::ChildEditWidgetObjectName = "childEditWidget";
