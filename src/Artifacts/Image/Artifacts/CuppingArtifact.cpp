#include "CuppingArtifact.h"

#include "../Filters/CuppingArtifactFilter.h"
#include "../../../Ui/Utils/CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QFormLayout>

CuppingArtifact::CuppingArtifact() = default;
CuppingArtifact::CuppingArtifact(CuppingArtifact const& other)  :
            DarkIntensityValue(other.DarkIntensityValue),
            Center(other.Center) {}
CuppingArtifact::CuppingArtifact(CuppingArtifact&&) noexcept = default;
auto CuppingArtifact::operator= (CuppingArtifact&&) noexcept -> CuppingArtifact& = default;

CuppingArtifact::~CuppingArtifact() = default;

auto CuppingArtifact::UpdateFilterParameters() -> void {
    Filter->SetDarkIntensityValue(DarkIntensityValue);

    Filter->SetCenterPoint(Center);
}

auto CuppingArtifact::GetFilter() -> vtkImageAlgorithm& {
    UpdateFilterParameters();

    return *Filter;
}

auto CuppingArtifactData::PopulateFromArtifact(const CuppingArtifact& artifact) noexcept -> void {
    DarkIntensityValue = artifact.DarkIntensityValue;

    Center = artifact.Center;
}

auto CuppingArtifactData::PopulateArtifact(CuppingArtifact& artifact) const noexcept -> void {
    artifact.DarkIntensityValue = DarkIntensityValue;

    artifact.Center = Center;

    artifact.UpdateFilterParameters();
}

CuppingArtifactWidget::CuppingArtifactWidget() :
        DarkIntensityValueSpinBox (new QDoubleSpinBox()),
        CenterPointWidget(new DoubleCoordinateRowWidget({ -100.0, 100.0, 1.0, 0.0 }, "Center")) {

    auto* fLayout = new QFormLayout(this);
    fLayout->setHorizontalSpacing(15);

    DarkIntensityValueSpinBox->setRange(-1000.0, 1000.0);
    DarkIntensityValueSpinBox->setSingleStep(10.0);

    fLayout->addRow("Dark Intensity", DarkIntensityValueSpinBox);

    fLayout->addRow(CenterPointWidget);
}

auto CuppingArtifactWidget::GetData() noexcept -> CuppingArtifactData {
    return { static_cast<float>(DarkIntensityValueSpinBox->value()),
             CenterPointWidget->GetRowData(0).ToFloatArray() };
}

auto CuppingArtifactWidget::Populate(const CuppingArtifactData& data) noexcept -> void {
    DarkIntensityValueSpinBox->setValue(data.DarkIntensityValue);

    CenterPointWidget->SetRowData(0, DoubleCoordinateRowWidget::RowData(data.Center));
}
