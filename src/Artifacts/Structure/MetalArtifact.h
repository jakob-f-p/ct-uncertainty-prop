#pragma once

#include "../../Utils/LinearAlgebraTypes.h"
#include "../../Utils/SimpleTransform.h"
#include "../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkVector.h>

#include <array>

class CtStructureTree;
class CtStructureVariant;

struct MetalArtifactData;

class QFormLayout;


class MetalArtifact {
public:
    using Data = MetalArtifactData;

    MetalArtifact() = default;
    MetalArtifact(MetalArtifact const& other) = default;
    auto operator= (MetalArtifact const&) -> MetalArtifact& = delete;
    MetalArtifact(MetalArtifact&&) = default;
    auto operator= (MetalArtifact&&) -> MetalArtifact& = default;

    auto
    SetDirectionOfHighestAttenuation(vtkVector2<float> direction) noexcept -> void;

    [[nodiscard]] auto
    GetMaxAttenuationFactor() const noexcept -> float { return MaxAttenuationFactor; }

    auto
    SetMaxAttenuationFactor(float factor) noexcept -> void;

    [[nodiscard]] auto
    EvaluateAtPosition(DoublePoint const& point,
                       float maxRadiodensity,
                       bool pointOccupiedByStructure,
                       CtStructureTree const& structureTree,
                       CtStructureVariant const& structure,
                       std::array<double, 3> spacing) const noexcept -> float;

    [[nodiscard]] auto
    GetLength() const noexcept -> float { return Length; }

    auto
    SetLength(float length) noexcept -> void;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return MTime; }

    auto
    Modified() noexcept -> void { MTime.Modified(); }

private:
    friend struct MetalArtifactData;

    vtkVector2<float> DirectionHighestAttenuation;
    vtkVector2<float> DirectionHighestAttenuationNormed;
    float MaxAttenuationFactor = 0.0F;
    float MaxAttenuationChangeFactor = 0.0F;
    float FactorRange = 0.0F;
    float Length = 0.0F;
    vtkTimeStamp MTime;
};


struct MetalArtifactData {
    using Artifact = MetalArtifact;

    vtkVector2<float> DirectionHighestAttenuation;
    float MaxAttenuationFactor = 0.0F;
    float Length = 0.0F;

    auto
    PopulateFromArtifact(MetalArtifact const& artifact) noexcept -> void;

    auto
    PopulateArtifact(MetalArtifact& artifact) const noexcept -> void;
};


class MetalArtifactWidget : public QWidget {
public:
    using Data = MetalArtifactData;

    MetalArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> Data;

    auto
    Populate(Data const& data) const noexcept -> void;

private:
    QFormLayout* Layout;
    QDoubleSpinBox* DirectionXSpinBox;
    QDoubleSpinBox* DirectionYSpinBox;
    QDoubleSpinBox* MaxAttenuationFactorSpinBox;
    QDoubleSpinBox* LengthSpinBox;
};
