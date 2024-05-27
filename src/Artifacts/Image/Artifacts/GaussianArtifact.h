#pragma once

#include <QWidget>

#include <vtkNew.h>
#include "../../../PipelineGroups/ObjectProperty.h"

class GaussianArtifactFilter;
class GaussianArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class GaussianArtifact {
public:
    using Data = GaussianArtifactData;

    GaussianArtifact();
    GaussianArtifact(GaussianArtifact const& other);
    auto operator= (GaussianArtifact const&) -> GaussianArtifact& = delete;
    GaussianArtifact(GaussianArtifact&&) noexcept ;
    auto operator= (GaussianArtifact&&) -> GaussianArtifact&;
    ~GaussianArtifact();

    [[nodiscard]] auto
    GetMean() const noexcept -> float { return Mean; }

    auto
    SetMean(float mean) -> void { Mean = mean; }

    [[nodiscard]] auto
    GetStandardDeviation() const noexcept -> float { return Sd; }

    auto
    SetStandardDeviation(float sd) -> void { Sd = sd; }

    auto
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        PipelineParameterProperties properties;
        properties.Add(FloatObjectProperty("Mean",
                                           [this] { return GetMean(); },
                                           [this](float mean) { this->SetMean(mean); },
                                           { -2000.0, 2000.0 }));
        properties.Add(FloatObjectProperty("Standard Deviation",
                                           [this] { return GetStandardDeviation(); },
                                           [this](float sd) { this->SetStandardDeviation(sd); },
                                           { 0.0, 2000.0 }));
        return properties;
    };

private:
    friend class GaussianArtifactData;

    float Mean = 0.0F;
    float Sd = 0.0F;

    vtkNew<GaussianArtifactFilter> Filter;
};


class GaussianArtifactWidget;

struct GaussianArtifactData {
    using Artifact = GaussianArtifact;
    using Widget = GaussianArtifactWidget;

    float Mean = 0.0F;
    float Sd = 0.0F;

    auto
    PopulateFromArtifact(const GaussianArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(GaussianArtifact& artifact) const noexcept -> void;
};



class GaussianArtifactWidget : public QWidget {
public:
    using Data = GaussianArtifactData;

    GaussianArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> GaussianArtifactData;

    auto
    Populate(const GaussianArtifactData& data) noexcept -> void;

private:
    QDoubleSpinBox* MeanSpinBox;
    QDoubleSpinBox* SdSpinBox;
};
