#include "Artifact.h"

#include "GaussianArtifact.h"
#include "CompositeArtifact.h"
#include "MotionArtifact.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>

void Artifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Name: '" << Name << "'\n";
    os << indent << "ArtifactType: '" << TypeToString(GetArtifactType()) << "'\n";
    os << indent << "ArtifactSubType: '" << SubTypeToString(GetArtifactSubType()) << "'\n";
}

void Artifact::SetName(const std::string& name) {
    Name = name;
}

std::string Artifact::TypeToString(Artifact::Type type) {
    switch (type) {
        case Type::IMAGE_ARTIFACT:     return "Image Artifact";
        case Type::STRUCTURE_ARTIFACT: return "Structure Artifact";
        default: {
            qWarning("no matching artifact type");
            return "";
        }
    }
}

std::string Artifact::SubTypeToString(Artifact::SubType subType) {
    switch (subType) {
        case SubType::IMAGE_GAUSSIAN:      return "Gaussian";
        case SubType::IMAGE_SALT_PEPPER:   return "Salt and Pepper";
        case SubType::IMAGE_RING:          return "Ring";
        case SubType::IMAGE_CUPPING:       return "Cupping";
        case SubType::IMAGE_WIND_MILL:     return "Wind Mill";
        case SubType::IMAGE_STAIR_STEP:    return "Stair Step";
        case SubType::IMAGE_STREAKING:     return "Streaking";
        case SubType::IMAGE_COMPOSITION:   return "Composition";

        case SubType::STRUCTURE_STREAKING: return "Streaking";
        case SubType::STRUCTURE_METALLIC:  return "Metallic";
        case SubType::STRUCTURE_MOTION:    return "Motion";
        default: {
            qWarning("no matching sub artifact type");
            return "";
        }
    }
}

Artifact* Artifact::NewArtifact(Artifact::SubType subType) {
    switch (subType) {
        case SubType::IMAGE_GAUSSIAN:     return GaussianArtifact::New();
//        case SubType::IMAGE_SALT_PEPPER:
//        case SubType::IMAGE_RING:
//        case SubType::IMAGE_CUPPING:
//        case SubType::IMAGE_WIND_MILL:
//        case SubType::IMAGE_STAIR_STEP:
//        case SubType::IMAGE_STREAKING:
        case SubType::IMAGE_COMPOSITION:  return CompositeArtifact::New();
//        case SubType::STRUCTURE_STREAKING:
//        case SubType::STRUCTURE_METALLIC:
        case SubType::STRUCTURE_MOTION:   return MotionArtifact::New();
        default: {
            qWarning("TODO: implement");
            return nullptr;
        }
    }
}

Artifact::Type Artifact::GetType(Artifact::SubType subType) {
    return Artifact::Type::STRUCTURE_ARTIFACT;
}


template class ArtifactData<ImageArtifact, ImageArtifactData>;
template class ArtifactData<StructureArtifact, StructureArtifactData>;

template<typename TArtifact, typename TData>
std::unique_ptr<TData> ArtifactData<TArtifact, TData>::GetData(const TArtifact& artifact) {
    std::unique_ptr<TData> data = TData::Create(artifact);

    data->Name = QString::fromStdString(artifact.Name);
    data->Type = artifact.GetArtifactType();
    data->SubType = artifact.GetArtifactSubType();

    TData::AddDerivedData(artifact, *data);

    return data;
}

template<typename TArtifact, typename TData>
void ArtifactData<TArtifact, TData>::SetData(TArtifact& artifact, const TData& data) {
    artifact.Name = data.Name.toStdString();

    TData::SetDerivedData(artifact, data);
}

template<typename TArtifact, typename TData>
void ArtifactData<TArtifact, TData>::SetData(TArtifact& artifact, const QVariant& variant) {
    std::unique_ptr<TData> data = TData::FromQVariant(variant);

    if (data)
        SetData(artifact, *data);
}



template class ArtifactUi<ImageArtifactUi, ImageArtifactData>;
template class ArtifactUi<StructureArtifactUi, StructureArtifactData>;

template<typename Ui, typename Data>
QWidget* ArtifactUi<Ui, Data>::GetWidget(bool showSubTypeComboBox) {
    auto* widget = new QWidget();
    auto* fLayout = new QFormLayout(widget);
    fLayout->setHorizontalSpacing(15);

    auto* typeComboBox = new QComboBox();
    typeComboBox->setObjectName(TypeComboBoxObjectName);
    for (const auto &types : Artifact::GetTypeValues())
        typeComboBox->addItem(types.Name, QVariant::fromValue(types.EnumValue));
    fLayout->addRow(typeComboBox);
    typeComboBox->hide();

    auto* subTypeComboBox = new QComboBox();
    subTypeComboBox->setObjectName(SubTypeComboBoxObjectName);
    for (const auto &subTypes : Ui::GetSubTypeValues())
        subTypeComboBox->addItem(subTypes.Name, QVariant::fromValue(subTypes.EnumValue));
    subTypeComboBox->setCurrentIndex(0);
    fLayout->addRow("Type", subTypeComboBox);
    if (!showSubTypeComboBox)
        fLayout->setRowVisible(subTypeComboBox, false);

    QObject::connect(subTypeComboBox, &QComboBox::currentIndexChanged, [widget, fLayout, subTypeComboBox]() {
        auto* subTypeWidget = widget->findChild<QWidget*>(SubTypeWidgetName);
        fLayout->removeRow(subTypeWidget);

        auto* newSubTypeWidget = new QWidget();
        newSubTypeWidget->setObjectName(SubTypeWidgetName);
        auto* subTypeFLayout = new QFormLayout(newSubTypeWidget);
        subTypeFLayout->setContentsMargins(0, 0, 0, 0);
        fLayout->addRow(newSubTypeWidget);

        auto subType = subTypeComboBox->currentData().value<Artifact::SubType>();
        Ui::AddDerivedWidgets(subTypeFLayout, subType);
    });

    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName(NameEditObjectName);
    fLayout->addRow("Name", nameLineEdit);

    auto* subTypeWidget = new QWidget();
    subTypeWidget->setObjectName(SubTypeWidgetName);
    auto* subTypeFLayout = new QFormLayout(subTypeWidget);
    subTypeFLayout->setContentsMargins(0, 0, 0, 0);
    fLayout->addRow(subTypeWidget);

    auto subType = subTypeComboBox->currentData().value<Artifact::SubType>();
    Ui::AddDerivedWidgets(subTypeFLayout, subType);

    return widget;
}

template<typename Ui, typename Data>
std::unique_ptr<Data> ArtifactUi<Ui, Data>::GetWidgetData(QWidget* widget) {
    if (!widget) {
        qWarning("Given widget was nullptr");
        return {};
    }

    auto* subTypeComboBox = widget->findChild<QComboBox*>(SubTypeComboBoxObjectName);
    auto subType = subTypeComboBox->currentData().value<Artifact::SubType>();

    std::unique_ptr<Data> data = Data::Create(subType);

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    data->Name = nameLineEdit->text();

    auto* typeComboBox = widget->findChild<QComboBox*>(TypeComboBoxObjectName);
    data->Type = typeComboBox->currentData().value<Artifact::Type>();

    data->SubType = subType;

    Ui::AddDerivedWidgetsData(widget, *data);

    return data;
}

template<typename Ui, typename Data>
void ArtifactUi<Ui, Data>::SetWidgetData(QWidget* widget, const Data& data) {
    if (!widget) {
        qWarning("Given widget was nullptr");
        return;
    }

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    nameLineEdit->setText(data.Name);

    auto* typeComboBox = widget->findChild<QComboBox*>(TypeComboBoxObjectName);
    if (int idx = typeComboBox->findData(QVariant::fromValue(data.Type));
            idx != -1)
        typeComboBox->setCurrentIndex(idx);

    auto* subTypeComboBox = widget->findChild<QComboBox*>(SubTypeComboBoxObjectName);
    if (int idx = subTypeComboBox->findData(QVariant::fromValue(data.SubType));
            idx != -1)
        subTypeComboBox->setCurrentIndex(idx);

    Ui::SetDerivedWidgetsData(widget, data);
}

template<typename Ui, typename Data>
const QString ArtifactUi<Ui, Data>::NameEditObjectName = "NameLineEdit";

template<typename Ui, typename Data>
const QString ArtifactUi<Ui, Data>::TypeComboBoxObjectName = "TypeComboBox";

template<typename Ui, typename Data>
const QString ArtifactUi<Ui, Data>::SubTypeComboBoxObjectName = "SubTypeComboBox";

template<typename Ui, typename Data>
const QString ArtifactUi<Ui, Data>::SubTypeWidgetName = "SubTypeWidget";
