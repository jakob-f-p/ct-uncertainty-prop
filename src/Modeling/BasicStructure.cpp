#include "BasicStructure.h"

#include "CtStructure.h"
#include "SimpleTransform.h"
#include "../Artifacts/StructureArtifactList.h"

#include "tracy/Tracy.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QWidget>

#include <vtkBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSphere.h>

#include <utility>

vtkStandardNewMacro(BasicStructure)

void BasicStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Implicit Function Type:" << ImplicitFunctionTypeToString(FunctionType) << "\n";
    os << indent << "Implicit Function: (" << ImplicitFunction << ")\n";
    os << indent << "Tissue Type: " << Tissue << ")\n";
    os << indent << "Structure Artifact List: " << StructureArtifacts << ")\n";
}

vtkMTimeType BasicStructure::GetMTime() {
    vtkMTimeType thisMTime = Superclass::GetMTime();
    vtkMTimeType implicitFunctionMTime = ImplicitFunction->GetMTime();

    return std::max(thisMTime, implicitFunctionMTime);
}

void BasicStructure::SetTransform(const std::array<std::array<float, 3>, 3>& trs) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot set transform.");
        return;
    }

    this->Transform->SetTranslationRotationScaling(trs);

    this->ImplicitFunction->SetTransform(Transform);
}

std::string
BasicStructure::ImplicitFunctionTypeToString(BasicStructure::ImplicitFunctionType implicitFunctionType) {
    switch (implicitFunctionType) {
        case ImplicitFunctionType::SPHERE: return "Sphere";
        case ImplicitFunctionType::BOX:    return "Box";
        case ImplicitFunctionType::CONE:   return "Cone";
        default: qWarning("No matching implicit function type found");
    }
    return "";
}

BasicStructure::ImplicitFunctionType BasicStructure::GetFunctionType() const {
    return FunctionType;
}

void BasicStructure::SetImplicitFunction(BasicStructure::ImplicitFunctionType implicitFunctionType) {
    if (ImplicitFunction) {
        ImplicitFunction->Delete();
    }

    FunctionType = implicitFunctionType;
    switch (implicitFunctionType) {
        case ImplicitFunctionType::SPHERE: {
            auto* sphere = vtkSphere::New();
            sphere->SetRadius(25.0f);
            ImplicitFunction = sphere;
            break;
        }
        case ImplicitFunctionType::BOX: {
            auto* box = vtkBox::New();
            box->SetBounds(-40.0, 40.0, -10.0, 10.0, -10.0, 10.0);
            ImplicitFunction = box;
            break;
        }
        case ImplicitFunctionType::CONE: {
            qWarning("Todo"); // TODO
        }

        default: qWarning("No matching implicit function type");
    }

    if (Transform) {
        ImplicitFunction->SetTransform(Transform);
    }
}

std::ostream& operator<<(ostream& stream, const BasicStructure::TissueOrMaterialType& type) {
    return stream << type.Name << ": ('" << type.CtNumber << "')";
}

BasicStructure::TissueOrMaterialType
BasicStructure::GetTissueOrMaterialTypeByName(const std::string& tissueName) {
    if (auto search = TissueTypeMap.find(tissueName); search != TissueTypeMap.end()) {
        return search->second;
    }

    qWarning("No tissue type with requested name present. Returning 'Air'");

    return TissueTypeMap.at("Air");
}

QStringList BasicStructure::GetTissueAndMaterialTypeNames() {
    QStringList names;
    std::transform(TissueTypeMap.cbegin(), TissueTypeMap.cend(), std::back_inserter(names),
                   [](const auto& type) { return QString::fromStdString(type.first); });
    return names;
}

void BasicStructure::SetTissueType(BasicStructure::TissueOrMaterialType tissueType) {
    this->Tissue = std::move(tissueType);

    this->Modified();
}

void BasicStructure::EvaluateAtPosition(const double x[3], CtStructure::Result& result) {
    ZoneScopedN("EvaluateStructure");
    result.FunctionValue = this->FunctionValue(x);
    result.IntensityValue = this->Tissue.CtNumber;

    StructureArtifacts->AddArtifactValuesAtPositionToMap(x, result.ArtifactValueMap);
}

const CtStructure::ModelingResult
BasicStructure::EvaluateImplicitModel(const double x[3]) const {
    return { FunctionValue(x), Tissue.CtNumber, Id };
}

float BasicStructure::FunctionValue(const double x[3]) const {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot calculate function value.");
        return 0.0;
    }

    return static_cast<float>(this->ImplicitFunction->FunctionValue(x));
}

bool BasicStructure::CtStructureExists(const CtStructure* structure) {
    return this == structure;
}

CtStructure::SubType BasicStructure::GetSubType() const {
    return BASIC;
}

void BasicStructure::DeepCopy(CtStructure* source, CombinedStructure* parent) {
    Superclass::DeepCopy(source, parent);

    auto* basicStructureSource = dynamic_cast<BasicStructure*>(source);
    Id = basicStructureSource->Id;
    FunctionType = basicStructureSource->FunctionType;
    ImplicitFunction = basicStructureSource->ImplicitFunction;
    ImplicitFunction->Register(this);
    Tissue = basicStructureSource->Tissue;
}

BasicStructure::BasicStructure() :
        Id(++GlobalBasicStructureId),
        FunctionType(ImplicitFunctionType::INVALID),
        ImplicitFunction(nullptr),
        Tissue(GetTissueOrMaterialTypeByName("Air")) {
}

BasicStructure::~BasicStructure() {
    this->ImplicitFunction->Delete();

    this->Modified();
}

std::string BasicStructure::GetViewName() const {
    return ImplicitFunctionTypeToString(FunctionType) + (Name.empty() ? "" : " (" + Name + ")");
}

std::map<std::string, BasicStructure::TissueOrMaterialType> BasicStructure::TissueTypeMap = {
        { "Air",             { "Air",            -1000.0f } },
        { "Fat",             { "Fat",             -100.0f } },
        { "Water",           { "Water",              0.0f } },
        { "Soft Tissue",     { "Soft Tissue",      200.0f } },
        { "Cancellous Bone", { "Cancellous Bone",  350.0f } },
        { "Cortical Bone",   { "Cortical Bone",    800.0f } },
        { "Metal",           { "Metal",          15000.0f } }
};

std::atomic<uint16_t> BasicStructure::GlobalBasicStructureId(0);

void BasicStructureData::AddDerivedData(const BasicStructure& structure, BasicStructureData& data) {
    data.FunctionType = structure.FunctionType;
    data.TissueName = QString::fromStdString(structure.Tissue.Name);

    switch (data.FunctionType) {
        case BasicStructure::ImplicitFunctionType::SPHERE: {
            auto* sphere = dynamic_cast<vtkSphere*>(structure.ImplicitFunction);
            data.Sphere.Radius = sphere->GetRadius();
            sphere->GetCenter(data.Sphere.Center.data());
            break;
        }

        case BasicStructure::ImplicitFunctionType::BOX: {
            auto* box = dynamic_cast<vtkBox*>(structure.ImplicitFunction);
            box->GetXMin(data.Box.MinPoint.data());
            box->GetXMax(data.Box.MaxPoint.data());
            break;
        }

        case BasicStructure::ImplicitFunctionType::CONE: {
            qWarning("Todo");
        }

        default: qWarning("No matching implicit function type");
    }
}

void BasicStructureData::SetDerivedData(BasicStructure& structure, const BasicStructureData& data) {
    structure.SetImplicitFunction(data.FunctionType);
    structure.SetTissueType(
            BasicStructure::GetTissueOrMaterialTypeByName(data.TissueName.toStdString()));

    switch (data.FunctionType) {
        case BasicStructure::ImplicitFunctionType::SPHERE: {
            auto* sphere = dynamic_cast<vtkSphere*>(structure.ImplicitFunction);
            sphere->SetRadius(data.Sphere.Radius);
            sphere->SetCenter(data.Sphere.Center.data());
            break;
        }

        case BasicStructure::ImplicitFunctionType::BOX: {
            auto* box = dynamic_cast<vtkBox*>(structure.ImplicitFunction);
            std::array<double, 3> minPoint {}, maxPoint {};
            std::copy(data.Box.MinPoint.begin(), data.Box.MinPoint.end(), minPoint.begin());
            std::copy(data.Box.MaxPoint.begin(), data.Box.MaxPoint.end(), maxPoint.begin());
            box->SetXMin(minPoint.data());
            box->SetXMax(maxPoint.data());
            break;
        }

        case BasicStructure::ImplicitFunctionType::CONE: {
            qWarning("Todo");
        }

        default: qWarning("No matching implicit function type");
    }
}

void BasicStructureUi::SetFunctionType(QWidget* widget, BasicStructure::ImplicitFunctionType functionType) {
    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    if (int idx = functionTypeComboBox->findData(QVariant::fromValue(functionType));
            idx != -1)
        functionTypeComboBox->setCurrentIndex(idx);
}

void BasicStructureUi::AddDerivedWidgets(QFormLayout* fLayout) {
    auto* functionTypeComboBox = new QComboBox();
    functionTypeComboBox->setObjectName(FunctionTypeComboBoxName);
    for (const auto &implicitFunctionAndName : BasicStructure::GetImplicitFunctionTypeValues()) {
        functionTypeComboBox->addItem(implicitFunctionAndName.Name,
                                      QVariant::fromValue(implicitFunctionAndName.EnumValue));
    }
    functionTypeComboBox->setCurrentIndex(0);

    auto* tissueTypeComboBox = new QComboBox();
    tissueTypeComboBox->setObjectName(TissueTypeComboBoxName);
    tissueTypeComboBox->addItems(BasicStructure::GetTissueAndMaterialTypeNames());

    fLayout->addRow("Tissue Type", tissueTypeComboBox);
    fLayout->addRow("Structure Type", functionTypeComboBox);

    auto* functionGroup = new QGroupBox();
    functionGroup->setObjectName(FunctionParametersGroupName);
    fLayout->addRow(functionGroup);
    UpdateFunctionParametersGroup(fLayout);

    QObject::connect(functionTypeComboBox, &QComboBox::currentIndexChanged,
                     [&, fLayout]() { UpdateFunctionParametersGroup(fLayout); });
}

void BasicStructureUi::AddDerivedWidgetsData(QWidget* widget, BasicStructureData& data) {
    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto* tissueTypeComboBox = widget->findChild<QComboBox*>(TissueTypeComboBoxName);

    data.FunctionType = functionTypeComboBox->currentData().value<BasicStructure::ImplicitFunctionType>();
    data.TissueName = tissueTypeComboBox->currentText();

    switch (data.FunctionType) {
        case BasicStructure::ImplicitFunctionType::SPHERE: {
            auto* radiusSpinBox = widget->findChild<QDoubleSpinBox*>(SphereRadiusSpinBoxName);
            data.Sphere.Radius = radiusSpinBox->value();
            for (int i = 0; i < data.Sphere.Center.size(); i++) {
                auto* centerSpinBox = widget->findChild<QDoubleSpinBox*>(
                        CtStructureUi::GetAxisSpinBoxName(SphereCenterName, AxisNames[i]));
                data.Sphere.Center[i] = centerSpinBox->value();
            }
            break;
        }

        case BasicStructure::ImplicitFunctionType::BOX: {
            for (int i = 0; i < data.Box.MinPoint.size(); i++) {
                auto* minPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                        CtStructureUi::GetAxisSpinBoxName(BoxMinPointName, AxisNames[i]));
                data.Box.MinPoint[i] = minPointSpinBox->value();

                auto* maxPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                        CtStructureUi::GetAxisSpinBoxName(BoxMaxPointName, AxisNames[i]));
                data.Box.MaxPoint[i] = maxPointSpinBox->value();
            }
            break;
        }

        case BasicStructure::ImplicitFunctionType::CONE: {
            qWarning("TODO: implement");
            break;
        }

        default: qWarning("No matching implicit function type");
    }
}

void BasicStructureUi::SetDerivedWidgetsData(QWidget* widget, const BasicStructureData& data) {
    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto* tissueTypeComboBox = widget->findChild<QComboBox*>(TissueTypeComboBoxName);

    if (int idx = functionTypeComboBox->findData(QVariant::fromValue(data.FunctionType));
            idx != -1)
        functionTypeComboBox->setCurrentIndex(idx);

    if (int idx = tissueTypeComboBox->findText(data.TissueName);
            idx != -1)
        tissueTypeComboBox->setCurrentIndex(idx);

    UpdateFunctionParametersGroup(widget->findChild<QFormLayout*>());

    switch (data.FunctionType) {
        case BasicStructure::ImplicitFunctionType::SPHERE: {
            auto* radiusSpinBox = widget->findChild<QDoubleSpinBox*>(SphereRadiusSpinBoxName);
            radiusSpinBox->setValue(data.Sphere.Radius);
            for (int i = 0; i < data.Sphere.Center.size(); i++) {
                auto* centerSpinBox = widget->findChild<QDoubleSpinBox*>(
                        CtStructureUi::GetAxisSpinBoxName(SphereCenterName, AxisNames[i]));
                centerSpinBox->setValue(data.Sphere.Center[i]);
            }
            break;
        }

        case BasicStructure::ImplicitFunctionType::BOX: {
            for (int i = 0; i < data.Box.MinPoint.size(); i++) {
                auto* minPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                        CtStructureUi::GetAxisSpinBoxName(BoxMinPointName, AxisNames[i]));
                minPointSpinBox->setValue(data.Box.MinPoint[i]);

                auto* maxPointSpinBox = widget->findChild<QDoubleSpinBox*>(
                        CtStructureUi::GetAxisSpinBoxName(BoxMaxPointName, AxisNames[i]));
                maxPointSpinBox->setValue(data.Box.MaxPoint[i]);
            }
            break;
        }

        case BasicStructure::ImplicitFunctionType::CONE: {
            qWarning("TODO: implement");
            break;
        }

        default: qWarning("No matching implicit function type");
    }
}

QGroupBox* BasicStructureUi::GetFunctionParametersGroup(BasicStructure::ImplicitFunctionType functionType) {
    auto* group = new QGroupBox();
    group->setObjectName(FunctionParametersGroupName);
    group->setTitle(QString::fromStdString(BasicStructure::ImplicitFunctionTypeToString(functionType)));

    auto* fLayout = new QFormLayout(group);
    fLayout->setHorizontalSpacing(15);

    switch (functionType) {
        case BasicStructure::ImplicitFunctionType::SPHERE: {
            auto* radiusSpinBox = new QDoubleSpinBox();
            radiusSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
            radiusSpinBox->setObjectName(SphereRadiusSpinBoxName);
            radiusSpinBox->setRange(0.0, 100.0);
            radiusSpinBox->setSingleStep(1.0);
            fLayout->addRow("Radius", radiusSpinBox);

            auto* centerCoordinates = GetCoordinatesRow(SphereCenterName, -100.0, 100.0, 1.0);
            fLayout->addRow("Center", centerCoordinates);
            break;
        }

        case BasicStructure::ImplicitFunctionType::BOX: {
            QWidget* minPointCoordinates = GetCoordinatesRow(BoxMinPointName, -100.0, 100.0, 1.0);
            QWidget* maxPointCoordinates = GetCoordinatesRow(BoxMaxPointName, -100.0, 100.0, 1.0);

            fLayout->addRow("Min. Point", minPointCoordinates);
            fLayout->addRow("Max. Point", maxPointCoordinates);
            break;
        }

        case BasicStructure::ImplicitFunctionType::CONE: {
            qWarning("TODO: implement");
            break;
        }

        default: qWarning("No matching Function Type");
    }

    return group;
}

void BasicStructureUi::UpdateFunctionParametersGroup(QFormLayout* fLayout) {
    auto* widget = fLayout->parentWidget();
    auto* oldFunctionParametersGroup = widget->findChild<QGroupBox*>(FunctionParametersGroupName);
    if (!oldFunctionParametersGroup) {
        qWarning("No function parameters group exists");
        return;
    }

    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto functionType = functionTypeComboBox->currentData().value<BasicStructure::ImplicitFunctionType>();
    auto* newFunctionParametersGroup = GetFunctionParametersGroup(functionType);

    fLayout->replaceWidget(oldFunctionParametersGroup, newFunctionParametersGroup);
    delete oldFunctionParametersGroup;
}

const QString BasicStructureUi::FunctionTypeComboBoxName = "functionType";

const QString BasicStructureUi::TissueTypeComboBoxName = "tissueType";

const QString BasicStructureUi::FunctionParametersGroupName = "functionTypeParametersGroup";

const QString BasicStructureUi::SphereRadiusSpinBoxName = "sphereRadius";

const QString BasicStructureUi::SphereCenterName = "sphereCenter";

const QString BasicStructureUi::BoxMinPointName = "boxMinPoint";

const QString BasicStructureUi::BoxMaxPointName = "boxMaxPoint";
