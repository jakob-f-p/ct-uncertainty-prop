#pragma once

#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class DoubleCoordinateRowWidget;
class RingArtifactFilter;
class RingArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class RingArtifact {
public:
    using Data = RingArtifactData;

    RingArtifact();
    RingArtifact(RingArtifact const& other);
    auto operator= (RingArtifact const&) -> RingArtifact& = delete;
    RingArtifact(RingArtifact&&) noexcept ;
    auto operator= (RingArtifact&&) noexcept -> RingArtifact&;
    ~RingArtifact();

    auto
    SetInnerRadius(float innerRadius) -> void { InnerRadius = innerRadius; }

    auto
    SetRingWidth(float ringWidth) -> void { RingWidth = ringWidth; }

    auto
    SetRadiodensityFactor(float radiodensityFactor) -> void { RadiodensityFactor = radiodensityFactor; }

    auto
    SetCenter(std::array<float, 3> center) -> void { Center = center; }

    auto
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

private:
    friend class RingArtifactData;

    float InnerRadius = 0.0F;
    float RingWidth = 0.0F;
    float RadiodensityFactor = 1.0F;

    std::array<float, 3> Center { 0.0F, 0.0F, 0.0F };

    vtkNew<RingArtifactFilter> Filter;
};


class RingArtifactWidget;

struct RingArtifactData {
    using Artifact = RingArtifact;
    using Widget = RingArtifactWidget;

    float InnerRadius = 0.0F;
    float RingWidth = 0.0F;
    float RadiodensityFactor = 0.0F;
    std::array<float, 3> Center = { 0.0F, 0.0F, 0.0F };

    auto
    PopulateFromArtifact(RingArtifact const& artifact) noexcept -> void;

    auto
    PopulateArtifact(RingArtifact& artifact) const noexcept -> void;
};



class RingArtifactWidget : public QWidget {
public:
    using Data = RingArtifactData;

    RingArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> RingArtifactData;

    auto
    Populate(const RingArtifactData& data) noexcept -> void;

private:
    QDoubleSpinBox* InnerRadiusSpinBox;
    QDoubleSpinBox* RingWidthSpinBox;
    QDoubleSpinBox* RadiodensityFactorSpinBox;

    DoubleCoordinateRowWidget* CenterPointWidget;
};
