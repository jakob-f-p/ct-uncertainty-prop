#pragma once

#include<string>

class Artifact {
public:
    virtual std::string getName() = 0;

    enum Type {
        IMAGE_ARTIFACT,
        STRUCTURE_ARTIFACT
    };

    virtual Type getArtifactType() = 0;
};