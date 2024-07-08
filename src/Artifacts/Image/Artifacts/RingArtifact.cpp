#include "RingArtifact.h"

#include "../Filters/RingArtifactFilter.h"
#include "../../../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

RingArtifact::RingArtifact() = default;
RingArtifact::RingArtifact(RingArtifact const& other) :
        BrightIntensityValue(other.BrightIntensityValue),
        DarkIntensityValue(other.DarkIntensityValue),
        BrightRingWidth(other.BrightRingWidth),
        DarkRingWidth(other.DarkRingWidth),
        Center(other.Center) {}
RingArtifact::RingArtifact(RingArtifact&&) = default;
auto RingArtifact::operator= (RingArtifact&&) -> RingArtifact& = default;

RingArtifact::~RingArtifact() = default;

auto RingArtifact::UpdateFilterParameters() -> void {
    Filter->SetBrightRingWidth(BrightRingWidth);
    Filter->SetDarkRingWidth(DarkRingWidth);

    Filter->SetBrightIntensityValue(BrightIntensityValue);
    Filter->SetDarkIntensityValue(DarkIntensityValue);

    Filter->SetCenterPoint(Center);
}

auto RingArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto RingArtifactData::PopulateFromArtifact(const RingArtifact& artifact) noexcept -> void {
    BrightRingWidth = artifact.BrightRingWidth;
    DarkRingWidth = artifact.DarkRingWidth;

    BrightIntensityValue = artifact.BrightIntensityValue;
    DarkIntensityValue = artifact.DarkIntensityValue;

    Center = artifact.Center;
}

auto RingArtifactData::PopulateArtifact(RingArtifact& artifact) const noexcept -> void {
    artifact.BrightRingWidth = BrightRingWidth;
    artifact.DarkRingWidth = DarkRingWidth;

    artifact.BrightIntensityValue = BrightIntensityValue;
    artifact.DarkIntensityValue = DarkIntensityValue;

    artifact.Center = Center;

    artifact.UpdateFilterParameters();
}

RingArtifactWidget::RingArtifactWidget() :
        BrightRingWidthSpinBox(new QDoubleSpinBox()),
        DarkRingWidthSpinBox  (new QDoubleSpinBox()),
        BrightIntensityValueSpinBox(new QDoubleSpinBox()),
        DarkIntensityValueSpinBox (new QDoubleSpinBox()),
        CenterPointWidget(new CoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 }, "Center")) {

    auto* gLayout = new QGridLayout(this);
    gLayout->setHorizontalSpacing(15);

    std::vector<QString> labels { "Bright", "Dark" };
    std::vector<QDoubleSpinBox*> widthSpinBoxes { BrightRingWidthSpinBox, DarkRingWidthSpinBox };
    std::vector<QDoubleSpinBox*> intensitySpinBoxes { BrightIntensityValueSpinBox, DarkIntensityValueSpinBox };
    std::vector<double> ranges { 0.0, 1000.0, -1000.0, 0.0 };

    for (int i = 0; i < 2; i++) {
        auto* label = new QLabel(labels.at(i));
        gLayout->addWidget(label, i, 0);

        auto* amountLabel = new QLabel("Width");
        amountLabel->setMinimumWidth(15);
        amountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gLayout->addWidget(amountLabel, i, 1);

        widthSpinBoxes[i]->setRange(0.0, 100.0);
        widthSpinBoxes[i]->setSingleStep(1.0);
        gLayout->addWidget(widthSpinBoxes[i], i, 2);

        auto* intensityLabel = new QLabel("Intensity");
        intensityLabel->setMinimumWidth(15);
        intensityLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gLayout->addWidget(intensityLabel, i, 3);

        intensitySpinBoxes[i]->setRange(ranges[2 * i], ranges[2 * i + 1]);
        intensitySpinBoxes[i]->setSingleStep(10.0);
        gLayout->addWidget(intensitySpinBoxes[i], i, 4);
    }

    gLayout->addWidget(CenterPointWidget, 2, 0, 1, 5);
}

auto RingArtifactWidget::GetData() noexcept -> RingArtifactData {
    return { static_cast<float>(BrightRingWidthSpinBox->value()),
             static_cast<float>(DarkRingWidthSpinBox->value()),
             static_cast<float>(BrightIntensityValueSpinBox->value()),
             static_cast<float>(DarkIntensityValueSpinBox->value()),
             CenterPointWidget->GetRowData(0).ToFloatArray() };
}

auto RingArtifactWidget::Populate(const RingArtifactData& data) noexcept -> void {
    BrightRingWidthSpinBox->setValue(data.BrightRingWidth);
    DarkRingWidthSpinBox->setValue(data.DarkRingWidth);

    BrightIntensityValueSpinBox->setValue(data.BrightIntensityValue);
    DarkIntensityValueSpinBox->setValue(data.DarkIntensityValue);

    CenterPointWidget->SetRowData(0, CoordinateRowWidget::RowData(data.Center));
}
