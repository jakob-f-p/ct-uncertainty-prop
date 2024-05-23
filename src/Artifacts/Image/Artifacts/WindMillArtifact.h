#pragma once

#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class CoordinateRowWidget;
class WindMillArtifactFilter;
class WindMillArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class WindMillArtifact {
public:
    using Data = WindMillArtifactData;

    WindMillArtifact();
    WindMillArtifact(WindMillArtifact const&) = delete;
    auto operator= (WindMillArtifact const&) -> WindMillArtifact& = delete;
    WindMillArtifact(WindMillArtifact&&);
    auto operator= (WindMillArtifact&&) -> WindMillArtifact&;
    ~WindMillArtifact();

    auto
    SetBrightAngularWidth(float brightAngularWidthWidth) -> void { BrightAngularWidth = brightAngularWidthWidth; }

    auto
    SetDarkAngularWidth(float darkAngularWidthWidth) -> void { DarkAngularWidth = darkAngularWidthWidth; }

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
    GetData() noexcept -> WindMillArtifactData;

    auto
    Populate(const WindMillArtifactData& data) noexcept -> void;

private:
    QDoubleSpinBox* BrightAngularWidthSpinBox;
    QDoubleSpinBox* DarkAngularWidthSpinBox;

    QDoubleSpinBox* BrightIntensityValueSpinBox;
    QDoubleSpinBox* DarkIntensityValueSpinBox;

    CoordinateRowWidget* CenterPointWidget;
};
