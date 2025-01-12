#pragma once

#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class DoubleCoordinateRowWidget;
class WindMillArtifactFilter;
struct WindMillArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class WindMillArtifact {
public:
    using Data = WindMillArtifactData;

    WindMillArtifact();
    WindMillArtifact(WindMillArtifact const& other);
    auto operator= (WindMillArtifact const&) -> WindMillArtifact& = delete;
    WindMillArtifact(WindMillArtifact&&) noexcept;
    auto operator= (WindMillArtifact&&) noexcept -> WindMillArtifact&;
    ~WindMillArtifact();

    auto
    SetBrightAngularWidth(float brightAngularWidth) -> void { BrightAngularWidth = brightAngularWidth; }

    auto
    SetDarkAngularWidth(float darkAngularWidth) -> void { DarkAngularWidth = darkAngularWidth; }

    auto
    SetBrightIntensity(float brightIntensityValue) -> void { BrightIntensityValue = brightIntensityValue; }

    auto
    SetDarkIntensity(float darkIntensityValue) -> void { DarkIntensityValue = darkIntensityValue; }

    auto
    SetCenter(std::array<float, 3> center) -> void { Center = center; }

    auto
    UpdateFilterParameters() const -> void;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        PipelineParameterProperties properties;
        properties.Add(FloatObjectProperty("Bright Angular Width",
                                           [this] { return BrightAngularWidth; },
                                           [this](float width) { this->SetBrightAngularWidth(width);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 360.0 }));
        properties.Add(FloatObjectProperty("Dark Angular Width",
                                           [this] { return DarkAngularWidth; },
                                           [this](float width) { this->SetDarkAngularWidth(width);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 360.0 }));
        properties.Add(FloatObjectProperty("Bright Intensity",
                                           [this] { return BrightIntensityValue; },
                                           [this](float intensity) { this->SetBrightIntensity(intensity);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 1000.0 }));
        properties.Add(FloatObjectProperty("Dark Intensity",
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
    friend class WindMillArtifactData;

    float BrightAngularWidth = 0.0F;
    float DarkAngularWidth = 0.0F;

    float BrightIntensityValue = 0.0F;
    float DarkIntensityValue = 0.0F;

    std::array<float, 3> Center { 0.0F, 0.0F, 0.0F };

    vtkNew<WindMillArtifactFilter> Filter;
};


class WindMillArtifactWidget;

struct WindMillArtifactData {
    using Artifact = WindMillArtifact;
    using Widget = WindMillArtifactWidget;

    float BrightAngularWidth = 0.0F;
    float DarkAngularWidth = 0.0F;

    float BrightIntensityValue = 0.0F;
    float DarkIntensityValue = 0.0F;

    std::array<float, 3> Center = { 0.0F, 0.0F, 0.0F };

    auto
    PopulateFromArtifact(const WindMillArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(WindMillArtifact& artifact) const noexcept -> void;
};



class WindMillArtifactWidget : public QWidget {
public:
    using Data = WindMillArtifactData;

    WindMillArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> WindMillArtifactData;

    auto
    Populate(const WindMillArtifactData& data) const noexcept -> void;

private:
    QDoubleSpinBox* BrightAngularWidthSpinBox;
    QDoubleSpinBox* DarkAngularWidthSpinBox;

    QDoubleSpinBox* BrightIntensityValueSpinBox;
    QDoubleSpinBox* DarkIntensityValueSpinBox;

    DoubleCoordinateRowWidget* CenterPointWidget;
};
