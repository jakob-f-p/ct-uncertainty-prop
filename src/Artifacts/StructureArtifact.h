#pragma once

#include "Artifact.h"

class StructureArtifact : public Artifact {
public:
    Type getArtifactType() override {
        return Type::STRUCTURE_ARTIFACT;
    };
};