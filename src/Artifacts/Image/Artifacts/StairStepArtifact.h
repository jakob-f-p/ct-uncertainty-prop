#pragma once

#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class StairStepArtifactFilter;
struct StairStepArtifactData;

class QDoubleSpinBox;

class vtkImageAlgorithm;


class StairStepArtifact {
public:
    using Data = StairStepArtifactData;

    StairStepArtifact();
    StairStepArtifact(StairStepArtifact const& other);
    auto operator= (StairStepArtifact const&) -> StairStepArtifact& = delete;
    StairStepArtifact(StairStepArtifact&&) noexcept;
    auto operator= (StairStepArtifact&&) noexcept -> StairStepArtifact&;
    ~StairStepArtifact();

    auto
    SetRelativeZAxisSamplingRate(float samplingRate) -> void { SamplingRate = samplingRate; }

    auto
    UpdateFilterParameters() const -> void;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        PipelineParameterProperties properties;
        properties.Add(FloatObjectProperty("z-Axis Sampling Rate",
                                           [this] { return SamplingRate; },
                                           [this](float samplingRate) { this->SetRelativeZAxisSamplingRate(samplingRate);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.01, 1.0 }));
        return properties;
    }

private:
    friend class StairStepArtifactData;

    float SamplingRate = 0.0F;

    vtkNew<StairStepArtifactFilter> Filter;
};


class StairStepArtifactWidget;

struct StairStepArtifactData {
    using Artifact = StairStepArtifact;
    using Widget = StairStepArtifactWidget;

    float SamplingRate = 0.0F;

    auto
    PopulateFromArtifact(const StairStepArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(StairStepArtifact& artifact) const noexcept -> void;
};



class StairStepArtifactWidget : public QWidget {
public:
    using Data = StairStepArtifactData;

    StairStepArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> StairStepArtifactData;

    auto
    Populate(const StairStepArtifactData& data) const noexcept -> void;

private:
    QDoubleSpinBox* SamplingRateSpinBox;
};
