#include "SaltPepperArtifact.h"

#include "../Filters/SaltPepperArtifactFilter.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

SaltPepperArtifact::SaltPepperArtifact() = default;
SaltPepperArtifact::SaltPepperArtifact(SaltPepperArtifact&&) = default;
auto SaltPepperArtifact::operator= (SaltPepperArtifact&&) -> SaltPepperArtifact& = default;
SaltPepperArtifact::~SaltPepperArtifact() = default;

auto SaltPepperArtifact::UpdateFilterParameters() -> void {
    Filter->SetSaltAmount(SaltAmount);
    Filter->SetPepperAmount(PepperAmount);

    Filter->SetSaltIntensityValue(SaltIntensityValue);
    Filter->SetPepperIntensityValue(PepperIntensityValue);
}

auto SaltPepperArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto SaltPepperArtifactData::PopulateFromArtifact(const SaltPepperArtifact& artifact) noexcept -> void {
    SaltAmount = artifact.SaltAmount;
    PepperAmount = artifact.PepperAmount;

    SaltIntensityValue = artifact.SaltIntensityValue;
    PepperIntensityValue = artifact.PepperIntensityValue;
}

auto SaltPepperArtifactData::PopulateArtifact(SaltPepperArtifact& artifact) const noexcept -> void {
    artifact.SaltAmount = SaltAmount;
    artifact.PepperAmount = PepperAmount;

    artifact.SaltIntensityValue = SaltIntensityValue;
    artifact.PepperIntensityValue = PepperIntensityValue;

    artifact.UpdateFilterParameters();
}

SaltPepperArtifactWidget::SaltPepperArtifactWidget() :
        SaltAmountSpinBox  (new QDoubleSpinBox()),
        PepperAmountSpinBox(new QDoubleSpinBox()),
        SaltIntensityValueSpinBox  (new QDoubleSpinBox()),
        PepperIntensityValueSpinBox(new QDoubleSpinBox()) {

    auto* gLayout = new QGridLayout(this);
    gLayout->setHorizontalSpacing(15);

    std::vector<QString> labels { "Salt", "Pepper" };
    std::vector<QDoubleSpinBox*> amountSpinBoxes { SaltAmountSpinBox, PepperAmountSpinBox };
    std::vector<QDoubleSpinBox*> intensitySpinBoxes { SaltIntensityValueSpinBox, PepperIntensityValueSpinBox };
    std::vector<double> ranges { 0.0, 2000.0, -2000.0, 0.0 };

    for (int i = 0; i < 2; i++) {
        auto* label = new QLabel(labels.at(i));
        gLayout->addWidget(label, i, 0);

        auto* amountLabel = new QLabel("Amount");
        amountLabel->setMinimumWidth(15);
        amountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gLayout->addWidget(amountLabel, i, 1);

        amountSpinBoxes[i]->setRange(0.0, 1.0);
        amountSpinBoxes[i]->setSingleStep(0.01);
        gLayout->addWidget(amountSpinBoxes[i], i, 2);

        auto* intensityLabel = new QLabel("Intensity");
        intensityLabel->setMinimumWidth(15);
        intensityLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gLayout->addWidget(intensityLabel, i, 3);

        intensitySpinBoxes[i]->setRange(ranges[2 * i], ranges[2 * i + 1]);
        intensitySpinBoxes[i]->setSingleStep(100.0);
        gLayout->addWidget(intensitySpinBoxes[i], i, 4);
    }
}

auto SaltPepperArtifactWidget::GetData() noexcept -> SaltPepperArtifactData {
    return { static_cast<float>(SaltAmountSpinBox->value()),
             static_cast<float>(PepperAmountSpinBox->value()),
             static_cast<float>(SaltIntensityValueSpinBox->value()),
             static_cast<float>(PepperIntensityValueSpinBox->value()), };
}

auto SaltPepperArtifactWidget::Populate(const SaltPepperArtifactData& data) noexcept -> void {
    SaltAmountSpinBox->setValue(data.SaltAmount);
    PepperAmountSpinBox->setValue(data.PepperAmount);

    SaltIntensityValueSpinBox->setValue(data.SaltIntensityValue);
    PepperIntensityValueSpinBox->setValue(data.PepperIntensityValue);
}
