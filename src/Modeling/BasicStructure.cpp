#include "BasicStructure.h"

#include "CtStructure.h"
#include "SimpleTransform.h"
#include "../Artifacts/StructureArtifactList.h"

#include "tracy/Tracy.hpp"

#include <QWidget>

#include <vtkBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSphere.h>

#include <utility>
#include <QLabel>
#include <QComboBox>

vtkStandardNewMacro(BasicStructure)

void BasicStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Implicit Function Type:" << ImplicitFunctionTypeToString(ImplicitFType) << "\n";
    os << indent << "Implicit Function: (" << ImplicitFunction << ")\n";
    os << indent << "Tissue Type: " << Tissue << ")\n";
    os << indent << "Structure Artifact List: " << StructureArtifacts << ")\n";
}

vtkMTimeType BasicStructure::GetMTime() {
    vtkMTimeType thisMTime = Superclass::GetMTime();
    vtkMTimeType implicitFunctionMTime = ImplicitFunction->GetMTime();

    return std::max(thisMTime, implicitFunctionMTime);
}

std::string
BasicStructure::ImplicitFunctionTypeToString(BasicStructure::ImplicitFunctionType implicitFunctionType) {
    switch (implicitFunctionType) {
        case SPHERE: return "Sphere";
        case BOX:    return "Box";
        case CONE:   return "Cone";
        default: {
            qWarning("No matching implicit function type found");
            return "";
        }
    }
}

void BasicStructure::SetImplicitFunction(BasicStructure::ImplicitFunctionType implicitFunctionType) {
    if (ImplicitFunction) {
        ImplicitFunction->Delete();
    }

    ImplicitFType = implicitFunctionType;
    switch (implicitFunctionType) {
        case SPHERE: {
            auto* sphere = vtkSphere::New();
            sphere->SetRadius(25.0f);
            ImplicitFunction = sphere;
            break;
        }
        case BOX: {
            auto* box = vtkBox::New();
            box->SetBounds(-40.0, 40.0, -10.0, 10.0, -10.0, 10.0);
            ImplicitFunction = box;
            break;
        }
        case CONE: {
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

void BasicStructure::SetTransform(const std::array<std::array<float, 3>, 3>& trs) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot set transform.");
        return;
    }

    this->Transform->SetTranslationRotationScaling(trs);

    this->ImplicitFunction->SetTransform(Transform);
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

QVariant BasicStructure::Data() const {
    BasicStructureDetails basicStructureDetails {
        GetCtStructureDetails(),
        ImplicitFType,
        QString::fromStdString(Tissue.Name),
    };
    return QVariant::fromValue(basicStructureDetails);
}

BasicStructure::BasicStructure() :
        Id(++GlobalBasicStructureId),
        ImplicitFType(INVALID),
        ImplicitFunction(nullptr),
        Tissue(GetTissueOrMaterialTypeByName("Air")) {
}

BasicStructure::~BasicStructure() {
    this->ImplicitFunction->Delete();

    this->Modified();
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

void BasicStructure::SetData(const QVariant& variant) {
    auto basicStructureDetails = variant.value<BasicStructureDetails>();
    SetData(basicStructureDetails);
}

void BasicStructure::SetData(const BasicStructureDetails &basicStructureDetails) {
    SetImplicitFunction(basicStructureDetails.ImplicitFunctionType);
    SetTissueType(GetTissueOrMaterialTypeByName(basicStructureDetails.TissueName.toStdString()));

    SetCtStructureDetails(basicStructureDetails);
}

CtStructure::SubType BasicStructure::GetSubType() const {
    return BASIC;
}

QWidget* BasicStructure::GetEditWidget(ImplicitFunctionType functionType) {
    auto* widget = new QWidget();
    auto* vLayout = new QVBoxLayout(widget);

    CtStructure::AddNameEditWidget(vLayout);

    auto* basicStructureWidget = new QWidget();
    auto* basicStructureHLayout = new QHBoxLayout(basicStructureWidget);
    auto* functionTypeLabel = new QLabel("Structure Type");
    basicStructureHLayout->addWidget(functionTypeLabel);
    auto* functionTypeComboBox = new QComboBox();
    functionTypeComboBox->setObjectName(FunctionTypeComboBoxName);
    basicStructureHLayout->addWidget(functionTypeComboBox);
    for (const auto &implicitFunctionAndName : BasicStructure::GetImplicitFunctionTypeValues()) {
        functionTypeComboBox->addItem(implicitFunctionAndName.Name, implicitFunctionAndName.EnumValue);
    }

    basicStructureHLayout->addSpacing(20);

    auto* tissueTypeLabel = new QLabel("Tissue Type");
    basicStructureHLayout->addWidget(tissueTypeLabel);
    auto* tissueTypeComboBox = new QComboBox();
    tissueTypeComboBox->setObjectName(TissueTypeComboBoxName);
    tissueTypeComboBox->addItems(BasicStructure::GetTissueAndMaterialTypeNames());
    basicStructureHLayout->addWidget(tissueTypeComboBox);
    vLayout->addWidget(basicStructureWidget);

    CtStructure::AddTransformEditWidget(vLayout);

    return widget;
}

void BasicStructure::SetEditWidgetData(QWidget* widget, const BasicStructureDetails& basicStructureDetails) {
    CtStructure::SetEditWidgetData(widget, basicStructureDetails);

    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto* tissueTypeComboBox = widget->findChild<QComboBox*>(TissueTypeComboBoxName);

    if (int idx = functionTypeComboBox->findData(basicStructureDetails.ImplicitFunctionType);
            idx != -1)
        functionTypeComboBox->setCurrentIndex(idx);

    if (int idx = tissueTypeComboBox->findText(basicStructureDetails.TissueName);
            idx != -1)
        tissueTypeComboBox->setCurrentIndex(idx);
}

BasicStructureDetails BasicStructure::GetEditWidgetData(QWidget* widget) {
    CtStructureDetails ctStructureDetails = CtStructure::GetEditWidgetData(widget);

    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto* tissueTypeComboBox = widget->findChild<QComboBox*>(TissueTypeComboBoxName);

    return { ctStructureDetails,
             functionTypeComboBox->currentData().value<ImplicitFunctionType>(),
             tissueTypeComboBox->currentText() };
}

void BasicStructure::DeepCopy(CtStructure* source, CombinedStructure* parent) {
    Superclass::DeepCopy(source, parent);

    auto* basicStructureSource = dynamic_cast<BasicStructure*>(source);
    Id = basicStructureSource->Id;
    ImplicitFType = basicStructureSource->ImplicitFType;
    ImplicitFunction = basicStructureSource->ImplicitFunction;
    ImplicitFunction->Register(this);
    Tissue = basicStructureSource->Tissue;
}
std::string BasicStructure::GetViewName() const {
    return ImplicitFunctionTypeToString(ImplicitFType) + (Name.empty() ? "" : " (" + Name + ")");
}

QString BasicStructure::FunctionTypeComboBoxName = "functionType";

QString BasicStructure::TissueTypeComboBoxName = "tissueType";
