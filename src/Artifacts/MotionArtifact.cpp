#include "MotionArtifact.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(MotionArtifact)

Artifact::SubType MotionArtifact::GetArtifactSubType() const {
    return SubType::STRUCTURE_MOTION;
}

float MotionArtifact::EvaluateAtPosition(const double *x) {
    vtkErrorMacro("TODO: Implement");
    return 0;
}

bool MotionArtifact::IgnoreCompetingStructures() {
    vtkErrorMacro("TODO: Implement");
    return false;
}

void MotionArtifact::DeepCopy(StructureArtifact *source) {
    StructureArtifact::DeepCopy(source);
}

QWidget* MotionArtifact::GetChildEditWidget() const {
    qWarning("Todo");
    return nullptr;
}

void MotionArtifact::SetStructureArtifactChildEditWidgetData(QWidget* widget,
                                                             const StructureArtifactDetails& details) const {
    qWarning("Todo");
}

void MotionArtifact::SetStructureArtifactChildData(const StructureArtifactDetails& details) {
    qWarning("Todo");
}

StructureArtifactDetails MotionArtifact::GetStructureArtifactEditWidgetData(QWidget* widget) const {
    qWarning("Todo");
    return {};
}
