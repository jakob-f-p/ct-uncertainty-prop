#include "RingArtifact.h"

#include "../Filters/RingArtifactFilter.h"
#include "../../../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

RingArtifact::RingArtifact() = default;
RingArtifact::RingArtifact(RingArtifact const& other) :
        InnerRadius(other.InnerRadius),
        RingWidth(other.RingWidth),
        RadiodensityFactor(other.RadiodensityFactor),
        Center(other.Center) {}
RingArtifact::RingArtifact(RingArtifact&&) noexcept = default;
auto RingArtifact::operator= (RingArtifact&&) noexcept -> RingArtifact& = default;

RingArtifact::~RingArtifact() = default;

auto RingArtifact::UpdateFilterParameters() -> void {
    Filter->SetInnerRadius(InnerRadius);
    Filter->SetRingWidth(RingWidth);
    Filter->SetRadiodensityFactor(RadiodensityFactor);

    Filter->SetCenterPoint(Center);
}

auto RingArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto RingArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    PipelineParameterProperties properties;
    properties.Add(FloatObjectProperty("Inner Radius",
                                       [this] { return InnerRadius; },
                                       [this](float radius) { this->SetInnerRadius(radius);
                                           this->UpdateFilterParameters(); },
                                       FloatObjectProperty::PropertyRange{ 0.0, 100.0 }));
    properties.Add(FloatObjectProperty("Ring Width",
                                       [this] { return RingWidth; },
                                       [this](float width) { this->SetRingWidth(width);
                                                                    this->UpdateFilterParameters(); },
                                       FloatObjectProperty::PropertyRange{ 0.0, 100.0 }));
    properties.Add(FloatObjectProperty("Radiodensity Factor",
                                       [this] { return RadiodensityFactor; },
                                       [this](float factor) { this->SetRadiodensityFactor(factor);
                                           this->UpdateFilterParameters(); },
                                       FloatObjectProperty::PropertyRange{ 0.0, 100.0, 0.1, 2 }));
    properties.Add(FloatPointObjectProperty("Center",
                                            [this] { return Center; },
                                            [this](FloatPoint center) { this->SetCenter(center);
                                                this->UpdateFilterParameters(); },
                                            {}));
    return properties;
}

auto RingArtifactData::PopulateFromArtifact(const RingArtifact& artifact) noexcept -> void {
    InnerRadius = artifact.InnerRadius;
    RingWidth = artifact.RingWidth;
    RadiodensityFactor = artifact.RadiodensityFactor;

    Center = artifact.Center;
}

auto RingArtifactData::PopulateArtifact(RingArtifact& artifact) const noexcept -> void {
    artifact.InnerRadius = InnerRadius;
    artifact.RingWidth = RingWidth;
    artifact.RadiodensityFactor = RadiodensityFactor;

    artifact.Center = Center;

    artifact.UpdateFilterParameters();
}

RingArtifactWidget::RingArtifactWidget() :
        InnerRadiusSpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.0, 100.0);
            spinBox->setSingleStep(1.0);
            return spinBox;
        }()),
        RingWidthSpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.0, 100.0);
            spinBox->setSingleStep(1.0);
            return spinBox;
        }()),
        RadiodensityFactorSpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.0, 100.0);
            spinBox->setSingleStep(0.1);
            return spinBox;
        }()),
        CenterPointWidget(new DoubleCoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 },
                                                        "Center")) {

    auto* fLayout = new QFormLayout(this);
    fLayout->setHorizontalSpacing(15);

    fLayout->addRow("Inner Radius", InnerRadiusSpinBox);
    fLayout->addRow("Ring Width", RingWidthSpinBox);
    fLayout->addRow("Radiodensity Factor", RadiodensityFactorSpinBox);
    fLayout->addRow(RadiodensityFactorSpinBox);
}

auto RingArtifactWidget::GetData() noexcept -> RingArtifactData {
    return { static_cast<float>(InnerRadiusSpinBox->value()),
             static_cast<float>(RingWidthSpinBox->value()),
             static_cast<float>(RadiodensityFactorSpinBox->value()),
             CenterPointWidget->GetRowData(0).ToFloatArray() };
}

auto RingArtifactWidget::Populate(const RingArtifactData& data) noexcept -> void {
    InnerRadiusSpinBox->setValue(data.InnerRadius);
    RingWidthSpinBox->setValue(data.RingWidth);
    RadiodensityFactorSpinBox->setValue(data.RadiodensityFactor);

    CenterPointWidget->SetRowData(0, DoubleCoordinateRowWidget::RowData(data.Center));
}
