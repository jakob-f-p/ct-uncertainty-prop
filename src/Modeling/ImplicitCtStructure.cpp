#include "ImplicitCtStructure.h"
#include "CtStructure.h"
#include "tracy/Tracy.hpp"

#include <vtkBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSphere.h>

#include <utility>

vtkStandardNewMacro(ImplicitCtStructure)

void ImplicitCtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Implicit Function Type:" << ImplicitFunctionTypeToString(ImplicitFType) << "\n";
    os << indent << "Implicit Function: (" << ImplicitFunction << ")\n";
    os << indent << "Tissue Type: " << Tissue << ")\n";
    os << indent << "Structure Artifact List: " << StructureArtifacts << ")\n";
}

vtkMTimeType ImplicitCtStructure::GetMTime() {
    vtkMTimeType thisMTime = Superclass::GetMTime();
    vtkMTimeType implicitFunctionMTime = ImplicitFunction->GetMTime();

    return std::max(thisMTime, implicitFunctionMTime);
}

std::string
ImplicitCtStructure::ImplicitFunctionTypeToString(ImplicitCtStructure::ImplicitFunctionType implicitFunctionType) {
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

void ImplicitCtStructure::SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType implicitFunctionType) {
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

std::ostream& operator<<(ostream& stream, const ImplicitCtStructure::TissueOrMaterialType& type) {
    return stream << type.Name << ": ('" << type.CtNumber << "')";
}

ImplicitCtStructure::TissueOrMaterialType
ImplicitCtStructure::GetTissueOrMaterialTypeByName(const std::string& tissueName) {
    if (auto search = TissueTypeMap.find(tissueName); search != TissueTypeMap.end()) {
        return search->second;
    }

    qWarning("No tissue type with requested name present. Returning 'Air'");

    return TissueTypeMap.at("Air");
}

QStringList ImplicitCtStructure::GetTissueAndMaterialTypeNames() {
    QStringList names;
    for (const auto &typeEntry: TissueTypeMap) {
        names.push_back(typeEntry.first.c_str());
    }
    return names;
}

void ImplicitCtStructure::SetTransform(const std::array<std::array<float, 3>, 3>& trs) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot set transform.");
        return;
    }

    this->Transform->SetTranslationRotationScaling(trs);

    this->ImplicitFunction->SetTransform(Transform);
}

void ImplicitCtStructure::SetTissueType(ImplicitCtStructure::TissueOrMaterialType tissueType) {
    this->Tissue = std::move(tissueType);

    this->Modified();
}

void ImplicitCtStructure::EvaluateAtPosition(const double x[3], CtStructure::Result& result) {
    ZoneScopedN("EvaluateStructure");
    result.FunctionValue = this->FunctionValue(x);
    result.IntensityValue = this->Tissue.CtNumber;

    StructureArtifacts->AddArtifactValuesAtPositionToMap(x, result.ArtifactValueMap);
}

const CtStructure::ModelingResult
ImplicitCtStructure::EvaluateImplicitModel(const double x[3]) const {
    return { FunctionValue(x), Tissue.CtNumber, Id };
}

float ImplicitCtStructure::FunctionValue(const double x[3]) const {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot calculate function value.");
        return 0.0;
    }

    return static_cast<float>(this->ImplicitFunction->FunctionValue(x));
}

bool ImplicitCtStructure::CtStructureExists(const CtStructure* structure) {
    return this == structure;
}

int ImplicitCtStructure::ChildCount() const {
    return 0;
}

const std::vector<CtStructure*>* ImplicitCtStructure::GetChildren() const {
    return nullptr;
}

const CtStructure* ImplicitCtStructure::ChildAt(int idx) const {
    return nullptr;
}

QVariant ImplicitCtStructure::Data() const {
    ImplicitCtStructureDetails implicitCtStructureDetails {
        GetCtStructureDetails(),
        ImplicitFType,
        Tissue.Name.c_str(),
    };
    return QVariant::fromValue(implicitCtStructureDetails);
}

ImplicitCtStructure::ImplicitCtStructure() :
        Id(++GlobalIdImplicitCtStructureId),
        ImplicitFType(SPHERE),
        ImplicitFunction(vtkSphere::New()),
        Tissue(GetTissueOrMaterialTypeByName("Air")),
        StructureArtifacts(StructureArtifactList::New()) {
}

ImplicitCtStructure::~ImplicitCtStructure() {
    this->ImplicitFunction->Delete();
    this->StructureArtifacts->Delete();

    this->Modified();
}

std::string ImplicitCtStructure::GetViewName() const {
    return ImplicitFunctionTypeToString(ImplicitFType) + (Name.empty() ? "" : " (" + Name + ")");
}

std::map<std::string, ImplicitCtStructure::TissueOrMaterialType> ImplicitCtStructure::TissueTypeMap = {
        { "Air",             { "Air",            -1000.0f } },
        { "Fat",             { "Fat",             -100.0f } },
        { "Water",           { "Water",              0.0f } },
        { "Soft Tissue",     { "Soft Tissue",      200.0f } },
        { "Cancellous Bone", { "Cancellous Bone",  350.0f } },
        { "Cortical Bone",   { "Cortical Bone",    800.0f } },
        { "Metal",           { "Metal",          15000.0f } }
};

std::atomic<uint16_t> ImplicitCtStructure::GlobalIdImplicitCtStructureId(0);

void ImplicitCtStructure::SetData(const QVariant& variant) {
    auto implicitCtStructureDetails = variant.value<ImplicitCtStructureDetails>();
    SetData(implicitCtStructureDetails);
}

void ImplicitCtStructure::SetData(const ImplicitCtStructureDetails &implicitCtStructureDetails) {
    SetCtStructureDetails(implicitCtStructureDetails);

    SetImplicitFunction(implicitCtStructureDetails.ImplicitFunctionType);
    SetTissueType(GetTissueOrMaterialTypeByName(implicitCtStructureDetails.TissueName.toStdString()));
}

bool ImplicitCtStructure::IsImplicitCtStructure() const {
    return true;
}

void ImplicitCtStructure::DeepCopy(CtStructure* source, CtStructure* parent) {
    Superclass::DeepCopy(source, parent);

    auto* implicitCtStructureSource = dynamic_cast<ImplicitCtStructure*>(source);
    Id = implicitCtStructureSource->Id;
    ImplicitFType = implicitCtStructureSource->ImplicitFType;
    ImplicitFunction = implicitCtStructureSource->ImplicitFunction;
    ImplicitFunction->Register(this);
    Tissue = implicitCtStructureSource->Tissue;
    StructureArtifacts = StructureArtifactList::New();
    StructureArtifacts->DeepCopy(implicitCtStructureSource->StructureArtifacts);
}
