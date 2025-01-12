#pragma once

#include <QWidget>

#include <vtkNew.h>
#include "../../../PipelineGroups/ObjectProperty.h"

class GaussianArtifactFilter;
struct GaussianArtifactData;

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
    auto operator= (GaussianArtifact&&) noexcept -> GaussianArtifact&;
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
    UpdateFilterParameters() const -> void;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        PipelineParameterProperties properties;
        properties.Add(FloatObjectProperty("Mean",
                                           [this] { return GetMean(); },
                                           [this](float mean) { this->SetMean(mean);
                                                                        this->UpdateFilterParameters();  },
                                           { -1000.0, 3000.0 }));
        properties.Add(FloatObjectProperty("Standard Deviation",
                                           [this] { return GetStandardDeviation(); },
                                           [this](float sd) { this->SetStandardDeviation(sd);
                                                              this->UpdateFilterParameters(); },
                                           { 0.0000001, 1000.0 }));
        return properties;
    }

private:
    friend struct GaussianArtifactData;

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
    GetData() const noexcept -> GaussianArtifactData;

    auto
    Populate(const GaussianArtifactData& data) const noexcept -> void;

private:
    QDoubleSpinBox* MeanSpinBox;
    QDoubleSpinBox* SdSpinBox;
};
