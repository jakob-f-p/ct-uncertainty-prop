#pragma once

#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class CoordinateRowWidget;
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
    RingArtifact(RingArtifact&&);
    auto operator= (RingArtifact&&) -> RingArtifact&;
    ~RingArtifact();

    auto
    SetBrightRingWidth(float brightRingWidth) -> void { BrightRingWidth = brightRingWidth; }

    auto
    SetDarkRingWidth(float darkRingWidth) -> void { DarkRingWidth = darkRingWidth; }

    auto
    SetBrightIntensity(float brightIntensityValue) -> void { BrightIntensityValue = brightIntensityValue; }

    auto
    SetDarkIntensity(float darkIntensityValue) -> void { DarkIntensityValue = darkIntensityValue; }

    auto
    SetCenter(std::array<float, 3> center) -> void { Center = center; }

    auto
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        // TODO
        return {};
    }

private:
    friend class RingArtifactData;

    float BrightRingWidth = 0.0F;
    float DarkRingWidth = 0.0F;

    float BrightIntensityValue = 0.0F;
    float DarkIntensityValue = 0.0F;

    std::array<float, 3> Center { 0.0F, 0.0F, 0.0F };

    vtkNew<RingArtifactFilter> Filter;
};


class RingArtifactWidget;

struct RingArtifactData {
    using Artifact = RingArtifact;
    using Widget = RingArtifactWidget;

    float BrightRingWidth = 0.0F;
    float DarkRingWidth = 0.0F;

    float BrightIntensityValue = 0.0F;
    float DarkIntensityValue = 0.0F;

    std::array<float, 3> Center = { 0.0F, 0.0F, 0.0F };

    auto
    PopulateFromArtifact(const RingArtifact& artifact) noexcept -> void;

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
    QDoubleSpinBox* BrightRingWidthSpinBox;
    QDoubleSpinBox* DarkRingWidthSpinBox;

    QDoubleSpinBox* BrightIntensityValueSpinBox;
    QDoubleSpinBox* DarkIntensityValueSpinBox;

    CoordinateRowWidget* CenterPointWidget;
};
