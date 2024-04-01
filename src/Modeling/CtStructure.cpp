#include "CtStructure.h"

#include "BasicStructure.h"
#include "CombinedStructure.h"
#include "SimpleTransform.h"
#include "../Artifacts/StructureArtifactList.h"

#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include <vtkNew.h>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QFormLayout>

void CtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Name: " << Name << "\n";
    os << indent << "View Name: " << GetViewName() << "\n";
    os << indent << "Transform: " << "\n";
    Transform->PrintSelf(os, indent.GetNextIndent());
    os << indent << "Parent: " << Parent << std::endl;
}

vtkMTimeType CtStructure::GetMTime() {
    return std::max({ Superclass::GetMTime(), Transform->GetMTime() });
}

void CtStructure::SetName(std::string name) {
    Name = std::move(name);

    this->Modified();
}

CombinedStructure* CtStructure::GetParent() const {
    return Parent;
}

void CtStructure::SetParent(CombinedStructure* parent) {
    Parent = parent;
}

bool CtStructure::IsBasic() const {
    return GetSubType() == BASIC;
}

bool CtStructure::IsBasic(void* ctStructure) {
    return FromVoid(ctStructure)->IsBasic();
}

BasicStructure* CtStructure::ToBasic(void* basicStructure) {
    return dynamic_cast<BasicStructure*>(FromVoid(basicStructure));
}

BasicStructure* CtStructure::ToBasic(CtStructure* basicStructure) {
    return dynamic_cast<BasicStructure*>(basicStructure);
}

CombinedStructure* CtStructure::ToCombined(void* combinedStructure) {
    return dynamic_cast<CombinedStructure*>(FromVoid(combinedStructure));
}

CombinedStructure* CtStructure::ToCombined(CtStructure* combinedStructure) {
    return dynamic_cast<CombinedStructure*>(combinedStructure);
}

CtStructure* CtStructure::FromVoid(void* ctStructure) {
    return static_cast<CtStructure*>(ctStructure);
}

void CtStructure::DeepCopy(CtStructure* source, CombinedStructure* parent) {
    Name = source->Name;
    Transform->DeepCopy(source->Transform);
    Parent = parent;
    StructureArtifacts = StructureArtifactList::New();
    StructureArtifacts->DeepCopy(source->StructureArtifacts);
}

CtStructure::CtStructure() :
        Transform(SimpleTransform::New()),
        StructureArtifacts(StructureArtifactList::New()),
        Parent(nullptr) {
}

CtStructure::~CtStructure() {
    Transform->Delete();
    StructureArtifacts->Delete();
}


template struct CtStructureData<BasicStructure, BasicStructureData>;
template struct CtStructureData<CombinedStructure, CombinedStructureData>;

template<typename Structure, typename Data>
void CtStructureData<Structure, Data>::AddBaseData(const Structure& structure, Data& data) {
    data.Name = QString::fromStdString(structure.Name);
    data.ViewName = QString::fromStdString(static_cast<const CtStructure&>(structure).GetViewName());
    data.Transform = structure.Transform->GetTranslationRotationScaling();

    Data::AddDerivedData(structure, data);
}

template<typename Structure, typename Data>
void CtStructureData<Structure, Data>::SetBaseData(Structure& structure, const Data& data) {
    structure.SetName(data.Name.toStdString());

    Data::SetDerivedData(structure, data);

    structure.SetTransform(data.Transform);
}



template class CtStructureUi<BasicStructureUi, BasicStructureData>;
template class CtStructureUi<CombinedStructureUi, CombinedStructureData>;

template<typename Ui, typename Data>
void CtStructureUi<Ui, Data>::AddBaseWidgets(QWidget* widget) {
    auto* fLayout = new QFormLayout(widget);
    fLayout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    fLayout->setHorizontalSpacing(15);

    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName(NameEditObjectName);
    fLayout->addRow("Name", nameLineEdit);

    Ui::AddDerivedWidgets(fLayout);

    auto* transformGroup = new QGroupBox("Transform");
    auto* transformGLayout = new QGridLayout(transformGroup);
    transformGLayout->setColumnStretch(0, 1);
    std::array<std::array<double, 2>, 3> transformRanges { -100.0, 100.0, 0.0, 360.0, -10.0, 10.0 };
    std::array<double, 3> transformStepSizes { 2.0, 1.0, 0.1 };
    for (int i = 0; i < TransformNames.size(); i++) {
        AddCoordinatesRow(TransformNames[i], TransformNames[i],
                          transformRanges[i][0], transformRanges[i][1], transformStepSizes[i],
                          transformGLayout, i, i == 2 ? 1.0 : 0.0);
    }
    fLayout->addRow(transformGroup);
}

template<typename Ui, typename Data>
void CtStructureUi<Ui, Data>::AddBaseWidgetsData(QWidget* widget, Data& data) {
    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    data.Name = nameLineEdit->text();

    for (int i = 0; i < data.Transform.size(); ++i) {
        for (int j = 0; j < data.Transform[0].size(); ++j) {
            auto* spinBox = widget->findChild<QDoubleSpinBox*>(
                    GetAxisSpinBoxName(TransformNames[i], Base::AxisNames[j]));
            data.Transform[i][j] = static_cast<float>(spinBox->value());
        }
    }

    Ui::AddDerivedWidgetsData(widget, data);
}

template<typename Ui, typename Data>
void CtStructureUi<Ui, Data>::SetBaseWidgetsData(QWidget* widget, const Data& data) {
    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    nameLineEdit->setText(data.Name);

    for (int i = 0; i < data.Transform.size(); ++i) {
        for (int j = 0; j < data.Transform[0].size(); ++j) {
            auto* spinBox = widget->findChild<QDoubleSpinBox*>(
                    GetAxisSpinBoxName(TransformNames[i], Base::AxisNames[j]));
            spinBox->setValue(data.Transform[i][j]);
        }
    }

    Ui::SetDerivedWidgetsData(widget, data);
}

template<typename Ui, typename Data>
void CtStructureUi<Ui, Data>::AddCoordinatesRow(const QString& baseName, const QString& labelText,
                                                double minValue, double maxValue, double stepSize,
                                                QGridLayout* gridLayout, int gridLayoutRow,
                                                double defaultValue) {
    auto* titleLabel = new QLabel(labelText);
    gridLayout->addWidget(titleLabel, gridLayoutRow, 0);

    for (int i = 0; i < Base::AxisNames.size(); i++) {
        auto* coordinateLabel = new QLabel(Base::AxisNames[i]);
        coordinateLabel->setMinimumWidth(15);
        coordinateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gridLayout->addWidget(coordinateLabel, gridLayoutRow, 1 + 2*i);
        auto* coordinateSpinBox = new QDoubleSpinBox();
        coordinateSpinBox->setObjectName(GetAxisSpinBoxName(baseName, Base::AxisNames[i]));
        coordinateSpinBox->setRange(minValue, maxValue);
        coordinateSpinBox->setSingleStep(stepSize);
        coordinateSpinBox->setValue(defaultValue);
        gridLayout->addWidget(coordinateSpinBox, gridLayoutRow, 2 + 2*i);
    }
}

template<typename Ui, typename Data>
QWidget* CtStructureUi<Ui, Data>::GetCoordinatesRow(const QString& baseName,
                                                    double minValue, double maxValue, double stepSize) {
    auto* widget = new QWidget();
    auto* hLayout = new QHBoxLayout(widget);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch();

    for (int i = 0; i < Base::AxisNames.size(); i++) {
        auto* coordinateLabel = new QLabel(Base::AxisNames[i]);
        coordinateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (i > 0)
            coordinateLabel->setMinimumWidth(15);
        hLayout->addWidget(coordinateLabel);

        auto* coordinateSpinBox = new QDoubleSpinBox();
        coordinateSpinBox->setObjectName(GetAxisSpinBoxName(baseName, Base::AxisNames[i]));
        coordinateSpinBox->setRange(minValue, maxValue);
        coordinateSpinBox->setSingleStep(stepSize);
        hLayout->addWidget(coordinateSpinBox);
    }

    return widget;
}

template<typename Ui, typename Data>
QString CtStructureUi<Ui, Data>::GetAxisSpinBoxName(const QString& transformName, const QString& axisName) {
    return transformName + axisName;
}

template<typename Ui, typename Data>
const QString CtStructureUi<Ui, Data>::NameEditObjectName = "NameEdit";

template<typename Ui, typename Data>
const QStringList CtStructureUi<Ui, Data>::TransformNames { "Translate", "Rotate", "Scale" };
