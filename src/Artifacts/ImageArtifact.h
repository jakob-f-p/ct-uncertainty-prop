#pragma once

#include "Artifact.h"

class ImageArtifact : public Artifact {
public:
    Type getArtifactType() override {
        return Type::IMAGE_ARTIFACT;
    };
};