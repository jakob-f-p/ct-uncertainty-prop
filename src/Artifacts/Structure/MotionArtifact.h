#pragma once

#include "../../Utils/LinearAlgebraTypes.h"
#include "../../Utils/SimpleTransform.h"
#include "../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkTransform.h>

#include <array>

class CtStructureTree;
class CtStructureVariant;

struct MotionArtifactData;

class QFormLayout;
class QSpinBox;


class MotionArtifact {
public:
    using Data = MotionArtifactData;

    MotionArtifact() = default;
    MotionArtifact(MotionArtifact const& other) = default;
    auto operator= (MotionArtifact const&) -> MotionArtifact& = delete;
    MotionArtifact(MotionArtifact&&) = default;
    auto operator= (MotionArtifact&&) -> MotionArtifact& = default;

    [[nodiscard]] auto
    GetRadiodensityFactor() const noexcept -> float { return RadiodensityFactor; }

    auto
    SetRadiodensityFactor(float factor) noexcept -> void;

    auto
    SetBlurKernelRadius(uint16_t kernelRadius) noexcept -> void;

    [[nodiscard]] auto
    GetBlurKernelStandardDeviation() const noexcept -> float { return KernelSd; }

    auto
    SetBlurKernelStandardDeviation(float sd) noexcept -> void;

    auto
    SetTransform(SimpleTransformData const& data) noexcept -> void;

    [[nodiscard]] auto
    EvaluateAtPosition(DoublePoint const& point,
                       float maxRadiodensity,
                       bool pointOccupiedByStructure,
                       CtStructureTree const& structureTree,
                       CtStructureVariant const& structure,
                       std::array<double, 3> const& spacing) const noexcept -> float;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return MTime; }

    auto
    Modified() noexcept -> void { MTime.Modified(); }

private:
    friend struct MotionArtifactData;

    auto
    UpdateKernel() noexcept -> void;

    float RadiodensityFactor = 0.0F;
    uint16_t KernelRadius = 0;
    float KernelSd = 1.0F;
    SimpleTransform Transform;
    std::vector<float> Kernel2D;

    vtkTimeStamp MTime;
};


struct MotionArtifactData {
    using Artifact = MotionArtifact;

    float RadiodensityFactor = 0.0F;
    uint16_t KernelRadius = 0;
    float KernelSd = 1.0F;
    SimpleTransformData Transform {};

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
    GetData() const noexcept -> Data;

    auto
    Populate(Data const& data) const noexcept -> void;

private:
    QFormLayout* Layout;
    QDoubleSpinBox* RadiodensitySpinBox;
    QSpinBox* KernelRadiusSpinBox;
    QDoubleSpinBox* KernelSdSpinBox;
    SimpleTransformWidget* TransformWidget;
};
