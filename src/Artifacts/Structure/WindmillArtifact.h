#pragma once

#include "../../Utils/LinearAlgebraTypes.h"
#include "../../Utils/SimpleTransform.h"
#include "../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkTransform.h>

#include <array>

class CtStructureTree;
class CtStructureVariant;

struct WindmillArtifactData;

class QFormLayout;
class QSpinBox;


class WindmillArtifact {
public:
    using Data = WindmillArtifactData;

    WindmillArtifact() = default;
    WindmillArtifact(WindmillArtifact const& other) = default;
    auto operator= (WindmillArtifact const&) -> WindmillArtifact& = delete;
    WindmillArtifact(WindmillArtifact&&) = default;
    auto operator= (WindmillArtifact&&) -> WindmillArtifact& = default;

    [[nodiscard]] auto
    GetRadiodensityFactor() const noexcept -> float { return RadiodensityFactor; }

    auto
    SetRadiodensityFactor(float factor) noexcept -> void;

    [[nodiscard]] auto
    GetAngularWidth() const noexcept -> float { return AngularWidth; }

    auto
    SetAngularWidth(float angularWidth) noexcept -> void;

    [[nodiscard]] auto
    GetRotationPerSlice() const noexcept -> float { return RotationPerSlice; }

    auto
    SetRotationPerSlice(float rotationPerSlice) noexcept -> void;

    [[nodiscard]] auto
    GetLength() const noexcept -> float { return Length; }

    auto
    SetLength(float length) noexcept -> void;

    [[nodiscard]] auto
    EvaluateAtPosition(DoublePoint const& point,
                       float maxRadiodensity,
                       bool pointOccupiedByStructure,
                       CtStructureTree const& structureTree,
                       CtStructureVariant const& structure,
                       std::array<double, 3> spacing) const noexcept -> float;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType { return MTime; }

    auto
    Modified() noexcept -> void { MTime.Modified(); }

private:
    friend struct WindmillArtifactData;

    float RadiodensityFactor = 1.0F;
    float AngularWidth = 0.0F;
    float RoundedAngularWidth = 0.0F;
    float RotationPerSlice = 0.0F;
    float Length = 0.0F;

    vtkTimeStamp MTime;
};


struct WindmillArtifactData {
    using Artifact = WindmillArtifact;

    float RadiodensityFactor = 1.0F;
    float AngularWidth = 0.0F;
    float RotationPerSlice = 0.0F;
    float Length = 0.0F;

    auto
    PopulateFromArtifact(WindmillArtifact const& artifact) noexcept -> void;

    auto
    PopulateArtifact(WindmillArtifact& artifact) const noexcept -> void;
};


class WindmillArtifactWidget : public QWidget {
public:
    using Data = WindmillArtifactData;

    WindmillArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> Data;

    auto
    Populate(Data const& data) noexcept -> void;

private:
    QFormLayout* Layout;
    QDoubleSpinBox* RadiodensityFactorSpinBox;
    QDoubleSpinBox* AngularWidthSpinBox;
    QDoubleSpinBox* RotationPerSliceSpinBox;
    QDoubleSpinBox* LengthSpinBox;
};
