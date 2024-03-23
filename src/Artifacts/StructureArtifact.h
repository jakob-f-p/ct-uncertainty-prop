#pragma once

#include "Artifact.h"

class StructureArtifact : public Artifact {
public:
    vtkTypeMacro(StructureArtifact, Artifact)
    static StructureArtifact* NewStructureArtifact(SubType subType);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type GetArtifactType() const override;

    virtual float EvaluateAtPosition(const double x[3]) = 0;

    virtual bool IgnoreCompetingStructures() = 0;

    virtual void DeepCopy(StructureArtifact* source);

    StructureArtifact(const StructureArtifact&) = delete;
    void operator=(const StructureArtifact&) = delete;

protected:
    StructureArtifact() = default;
    ~StructureArtifact() override = default;
};
