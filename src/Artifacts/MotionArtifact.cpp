#include <vtkObjectFactory.h>

#include "MotionArtifact.h"

vtkStandardNewMacro(MotionArtifact)

Artifact::SubType MotionArtifact::GetArtifactSubType() const {
    return SubType::STRUCTURE_MOTION;
}

bool MotionArtifact::IgnoreCompetingStructures() {
    return false;
}
