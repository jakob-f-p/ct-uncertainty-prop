#include "GaussianArtifact.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(GaussianArtifact)

Artifact::SubType GaussianArtifact::GetArtifactSubType() const {
    return IMAGE_GAUSSIAN;
}

QVariant GaussianArtifact::Data() {
    return QVariant();
}
