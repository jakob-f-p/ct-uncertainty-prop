#include "GaussianArtifact.h"

#include "../Filters/GaussianArtifactFilter.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

GaussianArtifact::GaussianArtifact() = default;
GaussianArtifact::GaussianArtifact(GaussianArtifact&&) = default;
auto GaussianArtifact::operator= (GaussianArtifact&&) -> GaussianArtifact& = default;
GaussianArtifact::~GaussianArtifact() = default;

auto GaussianArtifact::UpdateFilterParameters() -> void {
    Filter->SetMean(Mean);
    Filter->SetSd(Sd);
}

auto GaussianArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto GaussianArtifactData::PopulateFromArtifact(const GaussianArtifact& artifact) noexcept -> void {
    Mean = artifact.Mean;
    Sd = artifact.Sd;
}

auto GaussianArtifactData::PopulateArtifact(GaussianArtifact& artifact) const noexcept -> void {
    artifact.Mean = Mean;
    artifact.Sd = Sd;

    artifact.UpdateFilterParameters();
}

GaussianArtifactWidget::GaussianArtifactWidget() :
        MeanSpinBox(new QDoubleSpinBox()),
        SdSpinBox  (new QDoubleSpinBox()) {

    auto* fLayout = new QFormLayout(this);

    MeanSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    MeanSpinBox->setRange(-1000.0, 1000.0);
    MeanSpinBox->setSingleStep(1.0);
    fLayout->addRow("Mean", MeanSpinBox);

    SdSpinBox->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);
    SdSpinBox->setRange(0.0, 1000.0);
    SdSpinBox->setSingleStep(1.0);
    fLayout->addRow("SD", SdSpinBox);
}

auto GaussianArtifactWidget::GetData() noexcept -> GaussianArtifactData {
    return { static_cast<float>(MeanSpinBox->value()),
             static_cast<float>(SdSpinBox->value()) };
}

auto GaussianArtifactWidget::Populate(const GaussianArtifactData& data) noexcept -> void {
    MeanSpinBox->setValue(data.Mean);
    SdSpinBox->setValue(data.Sd);
}
