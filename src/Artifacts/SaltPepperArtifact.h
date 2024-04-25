#pragma once

#include <QWidget>

#include <vtkNew.h>

class SaltPepperArtifactFilter;
class SaltPepperArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class SaltPepperArtifact {
public:
    using Data = SaltPepperArtifactData;

    SaltPepperArtifact();
    SaltPepperArtifact(SaltPepperArtifact const&) = delete;
    auto operator= (SaltPepperArtifact const&) -> SaltPepperArtifact& = delete;
    SaltPepperArtifact(SaltPepperArtifact&&);
    auto operator= (SaltPepperArtifact&&) -> SaltPepperArtifact&;
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
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

private:
    friend class SaltPepperArtifactData;

    float SaltAmount;
    float PepperAmount;

    float SaltIntensityValue;
    float PepperIntensityValue;

    vtkNew<SaltPepperArtifactFilter> Filter;
};



struct SaltPepperArtifactData {
    using Artifact = SaltPepperArtifact;

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
    GetData() noexcept -> SaltPepperArtifactData;

    auto
    Populate(const SaltPepperArtifactData& data) noexcept -> void;

private:
    QDoubleSpinBox* SaltAmountSpinBox;
    QDoubleSpinBox* PepperAmountSpinBox;

    QDoubleSpinBox* SaltIntensityValueSpinBox;
    QDoubleSpinBox* PepperIntensityValueSpinBox;
};
