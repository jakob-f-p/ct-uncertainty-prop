#include "CtStructure.h"

#include "CombinedStructure.h"
#include "SimpleTransform.h"
#include "../Artifacts/StructureArtifactList.h"

#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include <vtkNew.h>
#include <QGroupBox>
#include <QDoubleSpinBox>

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

bool CtStructure::IsBasicStructure() const {
    return GetSubType() == BASIC;
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

CtStructureDetails CtStructure::GetCtStructureDetails() const {
    return {
        QString::fromStdString(Name),
        QString::fromStdString(GetViewName()),
        Transform->GetTranslationRotationScaling()
    };
}

void CtStructure::SetCtStructureDetails(const CtStructureDetails &ctStructureDetails) {
    SetName(ctStructureDetails.Name.toStdString());
    SetTransform(ctStructureDetails.Transform);
}

void CtStructure::AddNameEditWidget(QLayout* layout) {
    auto* nameEditBar = new QWidget();
    auto* nameEditLayout = new QHBoxLayout(nameEditBar);
    auto* nameLineEditLabel = new QLabel("Name");
    nameLineEditLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName(NameEditObjectName);
    nameEditLayout->addWidget(nameLineEditLabel);
    nameEditLayout->addSpacing(20);
    nameEditLayout->addWidget(nameLineEdit);
    layout->addWidget(nameEditBar);
}

void CtStructure::AddTransformEditWidget(QLayout* layout) {
    auto* transformEditGroup = new QGroupBox("Transform");
    auto* transformVerticalLayout = new QVBoxLayout(transformEditGroup);

    std::array<double, 3> transformStepSizes { 2.0, 1.0, 0.1 };
    for (int i = 0; i < TransformNames.size(); i++)
        CreateTransformationEditGroup(TransformNames[i], transformStepSizes[i], transformVerticalLayout);

    layout->addWidget(transformEditGroup);
}

void CtStructure::SetEditWidgetData(QWidget* widget, const CtStructureDetails& ctStructureDetails) {
    if (!widget) {
        qWarning("Widget must not be nullptr");
        return;
    }

    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    nameLineEdit->setText(ctStructureDetails.Name);

    for (int i = 0; i < ctStructureDetails.Transform.size(); ++i) {
        for (int j = 0; j < ctStructureDetails.Transform[i].size(); ++j) {
            auto* spinBox = widget->findChild<QDoubleSpinBox*>(GetSpinBoxName(TransformNames[i], AxisNames[j]));
            spinBox->setValue(ctStructureDetails.Transform[i][j]);
        }
    }
}

CtStructureDetails CtStructure::GetEditWidgetData(QWidget* widget) {
    auto* nameLineEdit = widget->findChild<QLineEdit*>(NameEditObjectName);
    QString name = nameLineEdit->text();

    std::array<std::array<float, 3>, 3> transform {};
    for (int i = 0; i < transform.size(); ++i) {
        for (int j = 0; j < transform[i].size(); ++j) {
            auto* spinBox = widget->findChild<QDoubleSpinBox*>(GetSpinBoxName(TransformNames[i], AxisNames[j]));
            transform[i][j] = static_cast<float>(spinBox->value());
        }
    }

    return { name, "", transform };
}

void CtStructure::CreateTransformationEditGroup(const QString& transformName,
                                                double stepSize,
                                                QVBoxLayout* parentLayout) {
    auto* bar = new QWidget();
    bar->setObjectName(transformName);

    auto* hLayout = new QHBoxLayout(bar);

    auto* titleLabel = new QLabel(transformName);
    hLayout->addWidget(titleLabel);
    hLayout->addStretch();

    for (const auto& axisName : AxisNames) {
        hLayout->addSpacing(10);
        auto* label = new QLabel(axisName);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hLayout->addWidget(label);
        auto* transformSpinBox = new QDoubleSpinBox();
        transformSpinBox->setObjectName(GetSpinBoxName(transformName, axisName));
        transformSpinBox->setRange(-100.0, 100.0);
        transformSpinBox->setSingleStep(stepSize);
        hLayout->addWidget(transformSpinBox);
    }

    parentLayout->addWidget(bar);
}

QString CtStructure::GetSpinBoxName(const QString& transformName, const QString& axisName) {
    return transformName + axisName;
}

QString CtStructure::NameEditObjectName = "NameEdit";

QStringList CtStructure::TransformNames { "Translate", "Rotate", "Scale" };

QStringList CtStructure::AxisNames { "x", "y", "z" };
