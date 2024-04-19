#include "GaussianArtifact.h"

#include "Filters/GaussianArtifactFilter.h"

#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QFormLayout>
#include <QWidget>

#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(GaussianArtifact)

Artifact::SubType GaussianArtifact::GetArtifactSubType() const {
    return SubType::IMAGE_GAUSSIAN;
}

vtkImageAlgorithm& GaussianArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) {
    if (!Filter)
        Filter = GaussianArtifactFilter::New();
    else
        Filter->RemoveAllInputs();

    Filter->SetMean(Mean);
    Filter->SetSd(Sd);

    Filter->SetInputConnection(inputAlgorithm.GetOutputPort());

    return *Filter;
}

void GaussianArtifactData::AddSubTypeData(const ImageArtifact& imageArtifact) {
    auto& artifact = dynamic_cast<const GaussianArtifact&>(imageArtifact);
    Mean = artifact.Mean;
    Sd = artifact.Sd;
}

void GaussianArtifactData::SetSubTypeData(ImageArtifact& imageArtifact) const {
    auto& artifact = dynamic_cast<GaussianArtifact&>(imageArtifact);
    artifact.Mean = Mean;
    artifact.Sd = Sd;
}

void GaussianArtifactUi::AddSubTypeWidgets(QFormLayout* fLayout) {
    auto* group = new QGroupBox("Gaussian");
    auto* hLayout = new QHBoxLayout(group);

    auto* meanLabel = new QLabel("Mean (μ)");
    meanLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hLayout->addWidget(meanLabel);
    auto* meanSpinBox = new QDoubleSpinBox();
    meanSpinBox->setObjectName(MeanSpinBoxObjectName);
    meanSpinBox->setRange(-2000.0, 2000.0);
    meanSpinBox->setSingleStep(10.0);
    hLayout->addWidget(meanSpinBox);

    hLayout->addSpacing(5);
    hLayout->addStretch();

    auto* sdLabel= new QLabel("SD (σ)");
    sdLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hLayout->addWidget(sdLabel);
    auto* sdSpinBox = new QDoubleSpinBox();
    sdSpinBox->setObjectName(SdSpinBoxObjectName);
    sdSpinBox->setRange(0.0, 1000.0);
    sdSpinBox->setSingleStep(10.0);
    hLayout->addWidget(sdSpinBox);

    fLayout->addRow(group);
}

void GaussianArtifactUi::AddSubTypeWidgetsData(QWidget* widget, GaussianArtifactData& data) {
    auto* meanSpinBox = widget->findChild<QDoubleSpinBox*>(MeanSpinBoxObjectName);
    auto* sdSpinBox = widget->findChild<QDoubleSpinBox*>(SdSpinBoxObjectName);

    data.Mean = static_cast<float>(meanSpinBox->value());
    data.Sd = static_cast<float>(sdSpinBox->value());
}

void GaussianArtifactUi::SetSubTypeWidgetsData(QWidget* widget, const GaussianArtifactData& data) {
    auto* meanSpinBox = widget->findChild<QDoubleSpinBox*>(MeanSpinBoxObjectName);
    auto* sdSpinBox = widget->findChild<QDoubleSpinBox*>(SdSpinBoxObjectName);

    meanSpinBox->setValue(data.Mean);
    sdSpinBox->setValue(data.Sd);
}

const QString GaussianArtifactUi::MeanSpinBoxObjectName = "MeanSpinBox";
const QString GaussianArtifactUi::SdSpinBoxObjectName = "SdSpinBox";
