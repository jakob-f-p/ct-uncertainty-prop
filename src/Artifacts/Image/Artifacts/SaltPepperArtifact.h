#pragma once

#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class SaltPepperArtifactFilter;
struct SaltPepperArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class SaltPepperArtifact {
public:
    using Data = SaltPepperArtifactData;

    SaltPepperArtifact();
    SaltPepperArtifact(SaltPepperArtifact const& other);
    auto operator= (SaltPepperArtifact const&) -> SaltPepperArtifact& = delete;
    SaltPepperArtifact(SaltPepperArtifact&&) noexcept;
    auto operator= (SaltPepperArtifact&&) noexcept -> SaltPepperArtifact&;
    ~SaltPepperArtifact();

    auto
    SetSaltAmount(float saltAmount) -> void { SaltAmount = saltAmount; }

    auto
    SetPepperAmount(float pepperAmount) -> void { PepperAmount = pepperAmount; }

    auto
    SetSaltIntensity(float saltIntensityValue) -> void { SaltIntensityValue = saltIntensityValue; }

    auto
    SetPepperIntensity(float pepperIntensityValue) -> void { PepperIntensityValue = pepperIntensityValue; }

    auto
    UpdateFilterParameters() const -> void;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties {
        PipelineParameterProperties properties;
        properties.Add(FloatObjectProperty("Salt Amount",
                                           [this] { return SaltAmount; },
                                           [this](float amount) { this->SetSaltAmount(amount);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 1.0, 0.001, 4 }));
        properties.Add(FloatObjectProperty("Pepper Amount",
                                           [this] { return PepperAmount; },
                                           [this](float amount) { this->SetPepperAmount(amount);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 1.0, 0.001, 4 }));
        properties.Add(FloatObjectProperty("Salt Intensity",
                                           [this] { return SaltIntensityValue; },
                                           [this](float intensity) { this->SetSaltIntensity(intensity);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ 0.0, 3000.0 }));
        properties.Add(FloatObjectProperty("Pepper Intensity",
                                           [this] { return PepperIntensityValue; },
                                           [this](float intensity) { this->SetPepperIntensity(intensity);
                                               this->UpdateFilterParameters(); },
                                           FloatObjectProperty::PropertyRange{ -1000.0, 0.0 }));
        return properties;
    }

private:
    friend class SaltPepperArtifactData;

    float SaltAmount;
    float PepperAmount;

    float SaltIntensityValue;
    float PepperIntensityValue;

    vtkNew<SaltPepperArtifactFilter> Filter;
};


class SaltPepperArtifactWidget;

struct SaltPepperArtifactData {
    using Artifact = SaltPepperArtifact;
    using Widget = SaltPepperArtifactWidget;

    float SaltAmount;
    float PepperAmount;

    float SaltIntensityValue;
    float PepperIntensityValue;

    auto
    PopulateFromArtifact(const SaltPepperArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(SaltPepperArtifact& artifact) const noexcept -> void;
};



class SaltPepperArtifactWidget : public QWidget {
public:
    using Data = SaltPepperArtifactData;

    SaltPepperArtifactWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> SaltPepperArtifactData;

    auto
    Populate(const SaltPepperArtifactData& data) const noexcept -> void;

private:
    QDoubleSpinBox* SaltAmountSpinBox;
    QDoubleSpinBox* PepperAmountSpinBox;

    QDoubleSpinBox* SaltIntensityValueSpinBox;
    QDoubleSpinBox* PepperIntensityValueSpinBox;
};
