#pragma once

#include "Artifact.h"

class StructureArtifact : public Artifact {
public:
    vtkTypeMacro(StructureArtifact, Artifact);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type getArtifactType() const override;

    virtual bool IgnoreCompetingStructures() = 0;

    StructureArtifact(const StructureArtifact&) = delete;
    void operator=(const StructureArtifact&) = delete;

protected:
    StructureArtifact() = default;
    ~StructureArtifact() override = default;
};