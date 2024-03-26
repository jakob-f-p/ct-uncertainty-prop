#pragma once

#include "StructureArtifact.h"

class MotionArtifact : public StructureArtifact {
public:
    static MotionArtifact* New();

    SubType GetArtifactSubType() const override;

    float EvaluateAtPosition(const double* x) override;

    bool IgnoreCompetingStructures() override;

    void DeepCopy(StructureArtifact *source) override;

    StructureArtifactDetails GetStructureArtifactEditWidgetData(QWidget* widget) const override;

    MotionArtifact(const MotionArtifact&) = delete;
    void operator=(const MotionArtifact&) = delete;

protected:
    MotionArtifact() = default;
    ~MotionArtifact() override = default;

    QWidget* GetChildEditWidget() const override;

    void
    SetStructureArtifactChildEditWidgetData(QWidget* widget, const StructureArtifactDetails& details) const override;

    void SetStructureArtifactChildData(const StructureArtifactDetails& details) override;
};