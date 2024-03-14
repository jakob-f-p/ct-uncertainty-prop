#include "ImplicitCtStructure.h"
#include "CtStructure.h"

#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <utility>
#include <vtkSphere.h>

vtkStandardNewMacro(ImplicitCtStructure)

void ImplicitCtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "Implicit Function: (" << ImplicitFunction << ")\n";
    os << indent << "Tissue Type: " << Tissue << ")\n";
}

vtkMTimeType ImplicitCtStructure::GetMTime() {
    vtkMTimeType thisMTime = this->Superclass::GetMTime();
    vtkMTimeType implicitFunctionMTime = this->ImplicitFunction->GetMTime();

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

ImplicitCtStructure::ImplicitFunctionType ImplicitCtStructure::StringToImplicitFunctionType(const std::string& string){
    ImplicitFunctionType type;
    for (int i = 0; i < NUMBER_OF_IMPLICIT_FUNCTION_TYPES; ++i) {
        type = static_cast<ImplicitFunctionType>(i);
        if (ImplicitFunctionTypeToString(type) == string) {
            return type;
        }
    }

    qWarning("No matching implicit function type found");
    return NUMBER_OF_IMPLICIT_FUNCTION_TYPES;
}


void ImplicitCtStructure::SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType implicitFunctionType) {
    if (ImplicitFunction) {
        ImplicitFunction->Delete();
    }

    ImplicitFType = implicitFunctionType;
    switch (implicitFunctionType) {
        case SPHERE: {
            ImplicitFunction = vtkSphere::New();
            break;
        }
        case BOX: {
            ImplicitFunction = vtkSphere::New();
            break;
        }
        case CONE: {
            qWarning("Todo"); // TODO
        }

        default: qWarning("No matching implicit function type");
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

std::vector<std::string> ImplicitCtStructure::GetTissueAndMaterialTypeNames() {
    std::vector<std::string> names;
    for (const auto &typeEntry: TissueTypeMap) {
        names.push_back(typeEntry.first);
    }
    return names;
}

QStringList ImplicitCtStructure::GetTissueAndMaterialTypeNamesQ() {
    QStringList names;
    for (const auto &typeEntry: TissueTypeMap) {
        names.push_back(typeEntry.first.c_str());
    }
    return names;
}

void ImplicitCtStructure::SetTransform(const QVariant& trs) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot set transform.");
        return;
    }

    this->Transform->SetTranslationRotationScaling(trs);

    this->ImplicitFunction->SetTransform(Transform);
}

ImplicitCtStructure::ImplicitCtStructure() {
    this->ImplicitFType = NUMBER_OF_IMPLICIT_FUNCTION_TYPES;
    this->ImplicitFunction = nullptr;
    this->Tissue = GetTissueOrMaterialTypeByName("Air");
    this->StructureArtifacts = StructureArtifactList::New();
}

ImplicitCtStructure::~ImplicitCtStructure() {
    this->ImplicitFunction->Delete();
    this->StructureArtifacts->Delete();

    this->Modified();
}

void ImplicitCtStructure::SetTissueType(ImplicitCtStructure::TissueOrMaterialType tissueType) {
    this->Tissue = std::move(tissueType);

    this->Modified();
}

void ImplicitCtStructure::EvaluateAtPosition(const double x[3], CtStructure::Result& result) {
    result.FunctionValue = this->FunctionValue(x);
    result.IntensityValue = this->Tissue.CtNumber;

    StructureArtifacts->AddArtifactValuesAtPositionToMap(x, result.ArtifactValueMap);
}

float ImplicitCtStructure::FunctionValue(const double x[3]) {
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

QVariant ImplicitCtStructure::PackageData(CtStructure::DataKey dataKey) const {
    switch (dataKey) {
        case NAME: return Name.c_str();
        case EDIT_DIALOG_NAME: return ImplicitFunctionTypeToString(ImplicitFType).c_str();
        case TREE_VIEW_NAME: return (ImplicitFunctionTypeToString(ImplicitFType) + (Name.empty() ? "" : " (" + Name + ")")).c_str();
        case TRANSFORM: return GetTransformQVariant();
        case IMPLICIT_FUNCTION_TYPE: return static_cast<int>(ImplicitFType);
        case TISSUE_TYPE: return Tissue.Name.c_str();
        case STRUCTURE_ARTIFACTS: return QVariant(); // TODO
        default: return {};
    }
}

std::map<std::string, ImplicitCtStructure::TissueOrMaterialType> ImplicitCtStructure::TissueTypeMap = {
        { "Air",            { "Air",           -1000.0f } },
        { "Fat",            { "Fat",            -100.0f } },
        { "Water",          { "Water",             0.0f } },
        { "SoftTissue",     { "SoftTissue",      200.0f } },
        { "CancellousBone", { "CancellousBone",  350.0f } },
        { "CorticalBone",   { "CorticalBone",    800.0f } },
        { "Metal",          { "Metal",         15000.0f } }
};
