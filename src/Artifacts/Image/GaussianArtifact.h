#pragma once

#include <QWidget>

#include <vtkNew.h>

class GaussianArtifactFilter;
class GaussianArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class GaussianArtifact {
public:
    using Data = GaussianArtifactData;

    GaussianArtifact();
    GaussianArtifact(GaussianArtifact const&) = delete;
    auto operator= (GaussianArtifact const&) -> GaussianArtifact& = delete;
    GaussianArtifact(GaussianArtifact&&);
    auto operator= (GaussianArtifact&&) -> GaussianArtifact&;
    ~GaussianArtifact();

    auto
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

private:
    friend class GaussianArtifactData;

    float Mean = 0.0F;
    float Sd = 0.0F;

    vtkNew<GaussianArtifactFilter> Filter;
};


class GaussianArtifactWidget;

struct GaussianArtifactData {
    using Artifact = GaussianArtifact;
    using Widget = GaussianArtifactWidget;

    float Mean = 0.0F;
    float Sd = 0.0F;

    auto
    PopulateFromArtifact(const GaussianArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(GaussianArtifact& artifact) const noexcept -> void;
};



class GaussianArtifactWidget : public QWidget {
public:
    using Data = GaussianArtifactData;

    GaussianArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> GaussianArtifactData;

    auto
    Populate(const GaussianArtifactData& data) noexcept -> void;

private:
    QDoubleSpinBox* MeanSpinBox;
    QDoubleSpinBox* SdSpinBox;
};
