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



void MotionArtifactData::AddSubTypeData(const StructureArtifact& structureArtifact) {
    auto& artifact = dynamic_cast<const MotionArtifact&>(structureArtifact);
    qWarning("Todo");
}

void MotionArtifactData::SetSubTypeData(StructureArtifact& structureArtifact) const {
    auto& artifact = dynamic_cast<MotionArtifact&>(structureArtifact);
    qWarning("Todo");
}



void MotionArtifactUi::AddSubTypeWidgets(QFormLayout* fLayout) {
    qWarning("Todo");
}

void MotionArtifactUi::AddSubTypeWidgetsData(QWidget* widget, MotionArtifactData& data) {
    qWarning("Todo");
}

void MotionArtifactUi::SetSubTypeWidgetsData(QWidget* widget, const MotionArtifactData& data) {
    qWarning("Todo");
}
