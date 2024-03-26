#pragma once

#include "ImageArtifact.h"

class GaussianArtifact : public ImageArtifact {
public:
    static GaussianArtifact* New();

    SubType GetArtifactSubType() const override;

    QVariant Data() override;

    ImageArtifactDetails GetImageArtifactEditWidgetData(QWidget* widget) const override;

    GaussianArtifact(const GaussianArtifact&) = delete;
    void operator=(const GaussianArtifact&) = delete;

protected:
    GaussianArtifact();
    ~GaussianArtifact() override = default;

    QWidget* GetChildEditWidget() const override;
    void SetImageArtifactChildEditWidgetData(QWidget* widget, const ImageArtifactDetails& details) const override;
    void SetImageArtifactChildData(const ImageArtifactDetails& details) override;

    float Mean;
    float Sd;

    const QString MeanSpinBoxObjectName;
    const QString SdSpinBoxObjectName;
};

struct GaussianArtifactDetails {
    float Mean;
    float Sd;
};
