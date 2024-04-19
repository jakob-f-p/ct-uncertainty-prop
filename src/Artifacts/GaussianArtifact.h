#pragma once

#include "ImageArtifact.h"

#include <vtkSmartPointer.h>

class GaussianArtifactFilter;

class GaussianArtifact : public ImageArtifact {
public:
    static auto New() -> GaussianArtifact*;

    auto GetArtifactSubType() const -> SubType override;

    auto
    AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& override;

    GaussianArtifact(const GaussianArtifact&) = delete;
    void operator=(const GaussianArtifact&) = delete;

private:
    GaussianArtifact() = default;
    ~GaussianArtifact() override = default;

    friend class GaussianArtifactData;

    float Mean = 0.0F;
    float Sd = 0.0F;

    vtkSmartPointer<GaussianArtifactFilter> Filter;
};



struct GaussianArtifactData : ImageArtifactData {
    float Mean = 0.0F;
    float Sd = 0.0F;

    ~GaussianArtifactData() override = default;

protected:
    void AddSubTypeData(const ImageArtifact& imageArtifact) override;

    void SetSubTypeData(ImageArtifact& imageArtifact) const override;
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
