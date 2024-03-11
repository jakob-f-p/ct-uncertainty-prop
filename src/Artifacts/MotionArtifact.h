#pragma once

#include "StructureArtifact.h"

class MotionArtifact : public StructureArtifact {
public:
    static MotionArtifact* New();

    SubType GetArtifactSubType() const override;

    float EvaluateAtPosition(const double* x) override;

    bool IgnoreCompetingStructures() override;

    MotionArtifact(const MotionArtifact&) = delete;
    void operator=(const MotionArtifact&) = delete;

protected:
    MotionArtifact() = default;
    ~MotionArtifact() override = default;
};