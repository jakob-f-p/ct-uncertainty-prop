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
        PipelineParameterProperties properties;
        properties.Add(FloatObjectProperty("Bright Ring Width",
                                           [this] { return BrightRingWidth; },
                                           [this](float width) { this->SetBrightRingWidth(width);
                                                                        this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 100.0 }));
        properties.Add(FloatObjectProperty("Dark Ring Width",
                                           [this] { return DarkRingWidth; },
                                           [this](float width) { this->SetDarkRingWidth(width);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 100.0 }));
        properties.Add(FloatObjectProperty("Bright Ring Intensity",
                                           [this] { return BrightIntensityValue; },
                                           [this](float intensity) { this->SetBrightIntensity(intensity);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 1000.0 }));
        properties.Add(FloatObjectProperty("Dark Ring Intensity",
                                           [this] { return DarkIntensityValue; },
                                           [this](float intensity) { this->SetDarkIntensity(intensity);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ -1000.0, 0.0 }));
        properties.Add(FloatPointObjectProperty("Center",
                                                [this] { return Center; },
                                                [this](FloatPoint center) { this->SetCenter(center);
                                                    this->UpdateFilterParameters(); },
                                                {}));
        return properties;
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

    DoubleCoordinateRowWidget* CenterPointWidget;
};
