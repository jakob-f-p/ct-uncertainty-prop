#include "CuppingArtifact.h"

#include "../Filters/CuppingArtifactFilter.h"
#include "../../../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

CuppingArtifact::CuppingArtifact() = default;
CuppingArtifact::CuppingArtifact(CuppingArtifact const& other)  :
        MinRadiodensityFactor(other.MinRadiodensityFactor),
        Center(other.Center) {}
CuppingArtifact::CuppingArtifact(CuppingArtifact&&) noexcept = default;
auto CuppingArtifact::operator= (CuppingArtifact&&) noexcept -> CuppingArtifact& = default;

CuppingArtifact::~CuppingArtifact() = default;

auto CuppingArtifact::UpdateFilterParameters() -> void {
    Filter->SetMinRadiodensityFactor(MinRadiodensityFactor);

    Filter->SetCenterPoint(Center);
}

auto CuppingArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto CuppingArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    PipelineParameterProperties properties;
    properties.Add(FloatObjectProperty("Minimum Radiodensity Factor",
                                       [this] { return GetMinRadiodensityFactor(); },
                                       [this](float intensity) {
                                               this->SetMinRadiodensityFactor(intensity);
                                               this->UpdateFilterParameters(); },
                                       FloatObjectProperty::PropertyRange { 0.0, 1.0, 0.1, 2 }));
    properties.Add(FloatPointObjectProperty("Center",
                                            [this] { return GetCenter(); },
                                            [this](FloatPoint center) { this->SetCenter(center);
                                                                        this->UpdateFilterParameters(); },
                                            {}));
    return properties;
}

auto CuppingArtifactData::PopulateFromArtifact(const CuppingArtifact& artifact) noexcept -> void {
    MinRadiodensityFactor = artifact.MinRadiodensityFactor;

    Center = artifact.Center;
}

auto CuppingArtifactData::PopulateArtifact(CuppingArtifact& artifact) const noexcept -> void {
    artifact.MinRadiodensityFactor = MinRadiodensityFactor;

    artifact.Center = Center;

    artifact.UpdateFilterParameters();
}

CuppingArtifactWidget::CuppingArtifactWidget() :
        MinRadiodensityFactorSpinBox (new QDoubleSpinBox()),
        CenterPointWidget(new DoubleCoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 }, "Center")) {

    auto* fLayout = new QFormLayout(this);
    fLayout->setHorizontalSpacing(15);

    MinRadiodensityFactorSpinBox->setRange(0.0, 1.0);
    MinRadiodensityFactorSpinBox->setSingleStep(0.10);
    MinRadiodensityFactorSpinBox->setDecimals(2);

    fLayout->addRow("Min Radiodensity Factor", MinRadiodensityFactorSpinBox);

    fLayout->addRow(CenterPointWidget);
}

auto CuppingArtifactWidget::GetData() noexcept -> CuppingArtifactData {
    return { static_cast<float>(MinRadiodensityFactorSpinBox->value()),
             CenterPointWidget->GetRowData(0).ToFloatArray() };
}

auto CuppingArtifactWidget::Populate(const CuppingArtifactData& data) noexcept -> void {
    MinRadiodensityFactorSpinBox->setValue(data.MinRadiodensityFactor);

    CenterPointWidget->SetRowData(0, DoubleCoordinateRowWidget::RowData(data.Center));
}
