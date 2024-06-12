#pragma once

#include "../../Utils/Types.h"
#include "../../Utils/SimpleTransform.h"
#include "../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkTransform.h>

#include <array>

struct MotionArtifactData;

class QFormLayout;


class MotionArtifact {
public:
    using Data = MotionArtifactData;

    MotionArtifact() = default;
    MotionArtifact(MotionArtifact const& other) = default;
    auto operator= (MotionArtifact const&) -> MotionArtifact& = delete;
    MotionArtifact(MotionArtifact&&) = default;
    auto operator= (MotionArtifact&&) -> MotionArtifact& = default;

    [[nodiscard]] auto
    GetCtNumberFactor() const noexcept -> float { return CtNumberFactor; }

    auto
    SetCtNumberFactor(float factor) noexcept -> void { CtNumberFactor = factor; Modified(); }

    auto
    SetTransform(SimpleTransformData const& data) noexcept -> void { Transform.SetData(data); Modified(); }

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

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        PipelineParameterProperties properties;
        properties.Add(
                FloatObjectProperty("Ct Number Factor",
                                    [this] { return GetCtNumberFactor(); },
                                    [this](float ctNumberFactor) { this->SetCtNumberFactor(ctNumberFactor); },
                                    { 0.01, 10.00, 0.01, 2 }));
        return properties;
    }

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return MTime; }

    auto
    Modified() noexcept -> void { MTime.Modified(); }

private:
    friend struct MotionArtifactData;

    SimpleTransform Transform;
    float CtNumberFactor = 0.0F;
    vtkTimeStamp MTime;
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
