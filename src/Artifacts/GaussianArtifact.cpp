#include "GaussianArtifact.h"

#include "ImageArtifactDetails.h"

#include <QLayout>
#include <QWidget>

#include <vtkObjectFactory.h>
#include <QLabel>
#include <QDoubleSpinBox>

vtkStandardNewMacro(GaussianArtifact)

GaussianArtifact::GaussianArtifact() :
        Mean(0.0f),
        Sd(1.0f),
        MeanSpinBoxObjectName("meanSpinBox"),
        SdSpinBoxObjectName("sdSpinBox") {
}

Artifact::SubType GaussianArtifact::GetArtifactSubType() const {
    return IMAGE_GAUSSIAN;
}

QVariant GaussianArtifact::Data() {
    ImageArtifactDetails details = GetImageArtifactDetails();
    details.Gaussian = { Mean, Sd };
    return QVariant::fromValue(details);
}

ImageArtifactDetails GaussianArtifact::GetImageArtifactEditWidgetData(QWidget* widget) const {
    auto* meanSpinBox = widget->findChild<QDoubleSpinBox*>(MeanSpinBoxObjectName);
    auto* sdSpinBox = widget->findChild<QDoubleSpinBox*>(SdSpinBoxObjectName);

    return {
        GetArtifactEditWidgetData(widget),
        {},
        { static_cast<float>(meanSpinBox->value()), static_cast<float>(sdSpinBox->value()) }
    };
}

QWidget* GaussianArtifact::GetChildEditWidget() const {
    auto* widget = new QWidget();

    auto* hLayout = new QHBoxLayout(widget);

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

    return widget;
}

void GaussianArtifact::SetImageArtifactChildEditWidgetData(QWidget* widget, const ImageArtifactDetails& details) const {
    auto* meanSpinBox = widget->findChild<QDoubleSpinBox*>(MeanSpinBoxObjectName);
    meanSpinBox->setValue(details.Gaussian.Mean);

    auto* sdSpinBox = widget->findChild<QDoubleSpinBox*>(SdSpinBoxObjectName);
    sdSpinBox->setValue(details.Gaussian.Sd);
}

void GaussianArtifact::SetImageArtifactChildData(const ImageArtifactDetails& details) {
    Mean = details.Gaussian.Mean;
    Sd = details.Gaussian.Sd;
}
