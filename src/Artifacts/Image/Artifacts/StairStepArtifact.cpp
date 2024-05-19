#include "StairStepArtifact.h"

#include "../Filters/StairStepArtifactFilter.h"
#include "../../../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

StairStepArtifact::StairStepArtifact() = default;
StairStepArtifact::StairStepArtifact(StairStepArtifact&&) = default;
auto StairStepArtifact::operator= (StairStepArtifact&&) -> StairStepArtifact& = default;
StairStepArtifact::~StairStepArtifact() = default;

auto StairStepArtifact::UpdateFilterParameters() -> void {
    Filter->SetSamplingRate(SamplingRate);
}

auto StairStepArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto StairStepArtifactData::PopulateFromArtifact(const StairStepArtifact& artifact) noexcept -> void {
    SamplingRate = artifact.SamplingRate;
}

auto StairStepArtifactData::PopulateArtifact(StairStepArtifact& artifact) const noexcept -> void {
    artifact.SamplingRate = SamplingRate;

    artifact.UpdateFilterParameters();
}

StairStepArtifactWidget::StairStepArtifactWidget() :
        SamplingRateSpinBox(new QDoubleSpinBox()) {

    auto* fLayout = new QFormLayout(this);
    fLayout->setHorizontalSpacing(15);

    SamplingRateSpinBox->setRange(0.01, 1.00);
    SamplingRateSpinBox->setSingleStep(0.01);

    fLayout->addRow("Relative sampling rate", SamplingRateSpinBox);
}

auto StairStepArtifactWidget::GetData() noexcept -> StairStepArtifactData {
    return { static_cast<float>(SamplingRateSpinBox->value()) };
}

auto StairStepArtifactWidget::Populate(const StairStepArtifactData& data) noexcept -> void {
    SamplingRateSpinBox->setValue(data.SamplingRate);
}
