#include "StructureArtifact.h"
#include "MotionArtifact.h"

void StructureArtifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

Artifact::Type StructureArtifact::GetArtifactType() const {
    return Type::STRUCTURE_ARTIFACT;
}

StructureArtifact *StructureArtifact::NewStructureArtifact(Artifact::SubType subType) {
    switch (subType) {
        case STRUCTURE_STREAKING: qWarning("implement");
        case STRUCTURE_METALLIC: qWarning("implement");
        case STRUCTURE_MOTION: return MotionArtifact::New();
        default: {
            qWarning("Not a supported structure artifact subtype");
            return {};
        }
    }
}

void StructureArtifact::DeepCopy(StructureArtifact* source) {
    Name = source->Name;
}
