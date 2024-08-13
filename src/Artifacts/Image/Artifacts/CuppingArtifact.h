#pragma once

#include "../../../Utils/LinearAlgebraTypes.h"
#include "../../../PipelineGroups/ObjectProperty.h"

#include <QWidget>

#include <vtkNew.h>

class DoubleCoordinateRowWidget;
class CuppingArtifactFilter;
class CuppingArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class CuppingArtifact {
public:
    using Data = CuppingArtifactData;

    CuppingArtifact();
    CuppingArtifact(CuppingArtifact const& other);
    auto operator= (CuppingArtifact const&) -> CuppingArtifact& = delete;
    CuppingArtifact(CuppingArtifact&&) noexcept ;
    auto operator= (CuppingArtifact&&) noexcept -> CuppingArtifact&;
    ~CuppingArtifact();

    [[nodiscard]] auto
    GetMinRadiodensityFactor() const noexcept -> float { return MinRadiodensityFactor; }

    auto
    SetMinRadiodensityFactor(float factor) -> void { MinRadiodensityFactor = factor; }

    [[nodiscard]] auto
    GetCenter() const noexcept -> FloatPoint { return Center; }

    auto
    SetCenter(FloatPoint center) -> void { Center = center; }

    auto
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetProperties() noexcept -> PipelineParameterProperties;

private:
    friend class CuppingArtifactData;

    float MinRadiodensityFactor = 0.0F;

    FloatPoint Center { 0.0F, 0.0F, 0.0F };

    vtkNew<CuppingArtifactFilter> Filter;
};


class CuppingArtifactWidget;

struct CuppingArtifactData {
    using Artifact = CuppingArtifact;
    using Widget = CuppingArtifactWidget;

    float MinRadiodensityFactor = 0.0F;

    FloatPoint Center = { 0.0F, 0.0F, 0.0F };

    auto
    PopulateFromArtifact(const CuppingArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(CuppingArtifact& artifact) const noexcept -> void;
};



class CuppingArtifactWidget : public QWidget {
public:
    using Data = CuppingArtifactData;

    CuppingArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> CuppingArtifactData;

    auto
    Populate(const CuppingArtifactData& data) noexcept -> void;

private:
    QDoubleSpinBox* MinRadiodensityFactorSpinBox;

    DoubleCoordinateRowWidget* CenterPointWidget;
};
