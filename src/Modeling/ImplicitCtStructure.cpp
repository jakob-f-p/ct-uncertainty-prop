#include "ImplicitCtStructure.h"

#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include <utility>

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

void ImplicitCtStructure::SetTransform(const QVariant& trs) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot set transform.");
        return;
    }

    this->Transform->SetTranslationRotationScaling(trs);

    this->ImplicitFunction->SetTransform(Transform);
}

ImplicitCtStructure::ImplicitCtStructure() {
    this->ImplicitFunction = nullptr;
    this->Tissue = CT::GetTissueOrMaterialTypeByName("Air");
    this->StructureArtifacts = StructureArtifactList::New();
}

ImplicitCtStructure::~ImplicitCtStructure() {
    this->ImplicitFunction->Delete();
    this->StructureArtifacts->Delete();

    this->Modified();
}

void ImplicitCtStructure::SetTissueType(CT::TissueOrMaterialType tissueType) {
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

const std::vector<CtStructure*>& ImplicitCtStructure::GetChildren() const {
    return {};
}

const CtStructure* ImplicitCtStructure::ChildAt(int idx) const {
    return nullptr;
}

QVariant ImplicitCtStructure::Data(int idx) const {
    switch (idx) {
        case SUBTYPE: return ImplicitFunction->GetClassName();
        case NAME: return Name.c_str();
        case DETAILS: return Tissue.Name.c_str();
        case LONG_NAME: return ("Basic Structure: " + std::string(ImplicitFunction->GetClassName()) + (Name.empty() ? "" : " (" + Name + ")")).c_str();
        case TRANSFORM: return GetTransformQVariant();
        case TISSUE_TYPE: return QMap<QString, QVariant>{ {Tissue.Name.c_str(), Tissue.CtNumber} };
//        case STRUCTURE_ARTIFACTS: return {}; // TODO
        default: return {};
    }
}
