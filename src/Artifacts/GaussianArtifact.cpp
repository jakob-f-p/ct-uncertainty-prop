#include <vtkObjectFactory.h>

#include "GaussianArtifact.h"

vtkStandardNewMacro(GaussianArtifact)

Artifact::SubType GaussianArtifact::GetArtifactSubType() const {
    return IMAGE_GAUSSIAN;
}
