#pragma once

#include "StructureArtifact.h"

class MotionArtifact : public StructureArtifact {
public:
    static MotionArtifact* New();

    SubType GetArtifactSubType() const override;

    float EvaluateAtPosition(const double* x) override;

    bool IgnoreCompetingStructures() override;

    void DeepCopy(StructureArtifact *source) override;

    MotionArtifact(const MotionArtifact&) = delete;
    void operator=(const MotionArtifact&) = delete;

protected:
    MotionArtifact() = default;
    ~MotionArtifact() override = default;
};



struct MotionArtifactData : StructureArtifactData {
    struct MotionData {
    };
    MotionData Motion;

    ~MotionArtifactData() override = default;

protected:
    friend struct StructureArtifactData;

    static void AddSubTypeData(const MotionArtifact& artifact, MotionArtifactData& data);

    static void SetSubTypeData(MotionArtifact& artifact, const MotionArtifactData& data);
};



class MotionArtifactUi : public StructureArtifactUi {
protected:
    friend struct StructureArtifactUi;

    static void AddSubTypeWidgets(QFormLayout* fLayout);

    static void AddSubTypeWidgetsData(QWidget* widget, MotionArtifactData& data);

    static void SetSubTypeWidgetsData(QWidget* widget, const MotionArtifactData& data);
};
