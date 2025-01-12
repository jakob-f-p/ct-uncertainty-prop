#include "WindMillArtifact.h"

#include "../Filters/WindMillArtifactFilter.h"
#include "../../../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

WindMillArtifact::WindMillArtifact() = default;
WindMillArtifact::WindMillArtifact(WindMillArtifact const& other) :
        BrightAngularWidth(other.BrightAngularWidth),
        DarkAngularWidth(other.DarkAngularWidth),
        BrightIntensityValue(other.BrightIntensityValue),
        DarkIntensityValue(other.DarkIntensityValue) {}
WindMillArtifact::WindMillArtifact(WindMillArtifact&&) noexcept = default;
auto WindMillArtifact::operator= (WindMillArtifact&&) noexcept -> WindMillArtifact& = default;

WindMillArtifact::~WindMillArtifact() = default;

auto WindMillArtifact::UpdateFilterParameters() const -> void {
    Filter->SetBrightAngularWidth(BrightAngularWidth);
    Filter->SetDarkAngularWidth(DarkAngularWidth);

    Filter->SetBrightIntensityValue(BrightIntensityValue);
    Filter->SetDarkIntensityValue(DarkIntensityValue);

    Filter->SetCenterPoint(Center);
}

auto WindMillArtifact::GetFilter() const -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto WindMillArtifactData::PopulateFromArtifact(const WindMillArtifact& artifact) noexcept -> void {
    BrightAngularWidth = artifact.BrightAngularWidth;
    DarkAngularWidth = artifact.DarkAngularWidth;

    BrightIntensityValue = artifact.BrightIntensityValue;
    DarkIntensityValue = artifact.DarkIntensityValue;

    Center = artifact.Center;
}

auto WindMillArtifactData::PopulateArtifact(WindMillArtifact& artifact) const noexcept -> void {
    artifact.BrightAngularWidth = BrightAngularWidth;
    artifact.DarkAngularWidth = DarkAngularWidth;

    artifact.BrightIntensityValue = BrightIntensityValue;
    artifact.DarkIntensityValue = DarkIntensityValue;

    artifact.Center = Center;

    artifact.UpdateFilterParameters();
}

WindMillArtifactWidget::WindMillArtifactWidget() :
        BrightAngularWidthSpinBox(new QDoubleSpinBox()),
        DarkAngularWidthSpinBox  (new QDoubleSpinBox()),
        BrightIntensityValueSpinBox(new QDoubleSpinBox()),
        DarkIntensityValueSpinBox (new QDoubleSpinBox()),
        CenterPointWidget(new DoubleCoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 }, "Center")) {

    auto* gLayout = new QGridLayout(this);
    gLayout->setHorizontalSpacing(15);

    std::vector<QString> const labels { "Bright", "Dark" };
    std::vector const widthSpinBoxes { BrightAngularWidthSpinBox, DarkAngularWidthSpinBox };
    std::vector const intensitySpinBoxes { BrightIntensityValueSpinBox, DarkIntensityValueSpinBox };
    std::vector const ranges { 0.0, 1000.0, -1000.0, 0.0 };

    for (int i = 0; i < 2; i++) {
        auto* label = new QLabel(labels.at(i));
        gLayout->addWidget(label, i, 0);

        auto* amountLabel = new QLabel("Angular Width");
        amountLabel->setMinimumWidth(15);
        amountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gLayout->addWidget(amountLabel, i, 1);

        widthSpinBoxes[i]->setRange(0.0, 360.0);
        widthSpinBoxes[i]->setSingleStep(1.0);
        widthSpinBoxes[i]->setSuffix("°");
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

auto WindMillArtifactWidget::GetData() const noexcept -> WindMillArtifactData {
    return { static_cast<float>(BrightAngularWidthSpinBox->value()),
             static_cast<float>(DarkAngularWidthSpinBox->value()),
             static_cast<float>(BrightIntensityValueSpinBox->value()),
             static_cast<float>(DarkIntensityValueSpinBox->value()),
             CenterPointWidget->GetRowData(0).ToFloatArray() };
}

auto WindMillArtifactWidget::Populate(const WindMillArtifactData& data) const noexcept -> void {
    BrightAngularWidthSpinBox->setValue(data.BrightAngularWidth);
    DarkAngularWidthSpinBox->setValue(data.DarkAngularWidth);

    BrightIntensityValueSpinBox->setValue(data.BrightIntensityValue);
    DarkIntensityValueSpinBox->setValue(data.DarkIntensityValue);

    CenterPointWidget->SetRowData(0, DoubleCoordinateRowWidget::RowData(data.Center));
}
