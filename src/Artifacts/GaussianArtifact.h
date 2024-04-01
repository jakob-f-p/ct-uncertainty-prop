#pragma once

#include "ImageArtifact.h"

class GaussianArtifact : public ImageArtifact {
public:
    static GaussianArtifact* New();

    SubType GetArtifactSubType() const override;

    GaussianArtifact(const GaussianArtifact&) = delete;
    void operator=(const GaussianArtifact&) = delete;

protected:
    GaussianArtifact();
    ~GaussianArtifact() override = default;

    friend class GaussianArtifactData;

    float Mean;
    float Sd;
};



struct GaussianArtifactData : ImageArtifactData {
    struct GaussianData {
        float Mean = 0.0f;
        float Sd = 0.0f;
    };
    GaussianData Gaussian;

    ~GaussianArtifactData() override = default;

protected:
    friend struct ImageArtifactData;

    static void AddSubTypeData(const GaussianArtifact& artifact, GaussianArtifactData& data);

    static void SetSubTypeData(GaussianArtifact& artifact, const GaussianArtifactData& data);
};



class GaussianArtifactUi : public ImageArtifactUi {
protected:
    friend struct ImageArtifactUi;

    static void AddSubTypeWidgets(QFormLayout* fLayout);

    static void AddSubTypeWidgetsData(QWidget* widget, GaussianArtifactData& data);

    static void SetSubTypeWidgetsData(QWidget* widget, const GaussianArtifactData& data);

private:
    static const QString MeanSpinBoxObjectName;
    static const QString SdSpinBoxObjectName;
};
