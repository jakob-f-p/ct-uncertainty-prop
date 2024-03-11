#include <vtkObjectFactory.h>

#include <utility>
#include <vtkNew.h>

#include "ImplicitCtStructure.h"
#include "../Artifacts/MotionArtifact.h"
#include "../Artifacts/GaussianArtifact.h"

vtkStandardNewMacro(ImplicitCtStructure)

void ImplicitCtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    vtkObject::PrintSelf(os, indent);

    os << indent << "Implicit Function: (" << ImplicitFunction << ")\n";
    os << indent << "Tissue Type: " << Tissue << ")\n";
}

ImplicitCtStructure::ImplicitCtStructure() {
    this->ImplicitFunction = nullptr;
    this->Tissue = CT::GetTissueOrMaterialTypeByName("Air");
    this->StructureArtifacts = StructureArtifactList::New();
    vtkNew<MotionArtifact> motionArtifact;
    this->StructureArtifacts->AddStructureArtifact(motionArtifact);
}

ImplicitCtStructure::~ImplicitCtStructure() {
    this->ImplicitFunction->Delete();
    this->StructureArtifacts->Delete();

    this->Modified();
}

double ImplicitCtStructure::FunctionValue(const double *x) {
    if (!this->ImplicitFunction) {
        vtkErrorMacro("No implicit function specified.");
        return 0.0;
    }

    return this->ImplicitFunction->FunctionValue(x);
}

void ImplicitCtStructure::SetTissueType(CT::TissueOrMaterialType tissueType) {
    this->Tissue = std::move(tissueType);
}
