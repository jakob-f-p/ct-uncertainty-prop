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

void ImplicitCtStructure::SetTransform(vtkAbstractTransform* transform) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified. Cannot set transform.");
        return;
    }

    this->ImplicitFunction->SetTransform(transform);
}

vtkAbstractTransform* ImplicitCtStructure::GetTransform() {
    return this->ImplicitFunction->GetTransform();
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
