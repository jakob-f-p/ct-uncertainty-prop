#pragma once

#include "../../Types.h"
#include "../../Modeling/SimpleTransform.h"

#include <QWidget>

#include <vtkTransform.h>

#include <array>

struct MotionArtifactData;

class QFormLayout;


class MotionArtifact {
public:
    using Data = MotionArtifactData;

    auto
    SetCtNumberFactor(float factor) noexcept -> void { CtNumberFactor = factor; }

    auto
    SetTransform(SimpleTransformData const& data) noexcept -> void { Transform.SetData(data); }

    [[nodiscard]] auto
    EvaluateAtPosition(DoublePoint const& point,
                       bool pointOccupiedByStructure,
                       float tissueValue,
                       std::function<float(DoublePoint)> const& functionValueEvaluator) const noexcept -> float {
        if (pointOccupiedByStructure)
            return 0.0F;

        DoublePoint const transformedPoint = Transform.TransformPoint(point);

        float const functionValue = functionValueEvaluator(transformedPoint);

        return functionValue < 0.0F
                ? tissueValue * CtNumberFactor + 1000.0F
                : 0.0F;
    }

private:
    friend struct MotionArtifactData;

    SimpleTransform Transform;
    float CtNumberFactor = 0.0F;
};


struct MotionArtifactData {
    using Artifact = MotionArtifact;

    SimpleTransformData Transform {};
    float CtNumberFactor = 0.0F;

    auto
    PopulateFromArtifact(MotionArtifact const& artifact) noexcept -> void;

    auto
    PopulateArtifact(MotionArtifact& artifact) const noexcept -> void;
};


class MotionArtifactWidget : public QWidget {
public:
    using Data = MotionArtifactData;

    MotionArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> Data;

    auto
    Populate(Data const& data) noexcept -> void;

private:
    QFormLayout* Layout;
    QDoubleSpinBox* CtNumberFactorSpinBox;
    SimpleTransformWidget* TransformWidget;
};
