#pragma once

#include "Artifact.h"

struct StructureArtifactDetails;

class StructureArtifact : public Artifact {
public:
    vtkTypeMacro(StructureArtifact, Artifact)
    static StructureArtifact* NewStructureArtifact(SubType subType);

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    Type GetArtifactType() const override;

    virtual float EvaluateAtPosition(const double x[3]) = 0;

    virtual bool IgnoreCompetingStructures() = 0;

    virtual void DeepCopy(StructureArtifact* source);

    virtual StructureArtifactDetails GetStructureArtifactEditWidgetData(QWidget* widget) const = 0;

    StructureArtifact(const StructureArtifact&) = delete;
    void operator=(const StructureArtifact&) = delete;

protected:
    StructureArtifact() = default;
    ~StructureArtifact() override = default;

    void SetChildEditWidgetData(QWidget* widget, const ArtifactDetails& artifactDetails) const override;
    void SetChildData(const ArtifactDetails& artifactDetails) override;

    virtual void SetStructureArtifactChildEditWidgetData(QWidget* widget, const StructureArtifactDetails& details) const = 0;
    virtual void SetStructureArtifactChildData(const StructureArtifactDetails& details) = 0;
};

struct StructureArtifactDetails : ArtifactDetails {

};