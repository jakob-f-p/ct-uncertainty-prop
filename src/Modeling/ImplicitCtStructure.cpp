#include "ImplicitCtStructure.h"
#include "CtStructure.h"

#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <utility>
#include <vtkSphere.h>

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

QVariant ImplicitCtStructure::Data() const {
    ImplicitCtStructureDetails implicitCtStructureDetails {
        GetCtStructureDetails(),
        ImplicitFType,
        Tissue.Name.c_str(),
        {}
    };
    return QVariant::fromValue(implicitCtStructureDetails);
}

ImplicitCtStructure::ImplicitCtStructure() {
    this->ImplicitFType = SPHERE;
    this->ImplicitFunction = vtkSphere::New();
    this->Tissue = GetTissueOrMaterialTypeByName("Air");
    this->StructureArtifacts = StructureArtifactList::New();
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
