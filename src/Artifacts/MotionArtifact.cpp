#include "MotionArtifact.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(MotionArtifact)

Artifact::SubType MotionArtifact::GetArtifactSubType() const {
    return SubType::STRUCTURE_MOTION;
}

float MotionArtifact::EvaluateAtPosition(const double *x) {
    vtkErrorMacro("TODO: Implement");
    return 0;
}

bool MotionArtifact::IgnoreCompetingStructures() {
    vtkErrorMacro("TODO: Implement");
    return false;
}

void MotionArtifact::DeepCopy(StructureArtifact *source) {
    StructureArtifact::DeepCopy(source);
}
