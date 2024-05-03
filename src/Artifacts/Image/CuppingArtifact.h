#pragma once

#include <QWidget>

#include <vtkNew.h>

class CoordinateRowWidget;
class CuppingArtifactFilter;
class CuppingArtifactData;

class QDoubleSpinBox;
class QFormLayout;

class vtkImageAlgorithm;


class CuppingArtifact {
public:
    using Data = CuppingArtifactData;

    CuppingArtifact();
    CuppingArtifact(CuppingArtifact const&) = delete;
    auto operator= (CuppingArtifact const&) -> CuppingArtifact& = delete;
    CuppingArtifact(CuppingArtifact&&);
    auto operator= (CuppingArtifact&&) -> CuppingArtifact&;
    ~CuppingArtifact();

    auto
    SetDarkIntensity(float darkIntensityValue) -> void { DarkIntensityValue = darkIntensityValue; }

    auto
    SetCenter(std::array<float, 3> center) -> void { Center = center; }

    auto
    UpdateFilterParameters() -> void;

    [[nodiscard]] auto
    GetFilter() -> vtkImageAlgorithm&;

private:
    friend class CuppingArtifactData;

    float DarkIntensityValue = 0.0F;

    std::array<float, 3> Center { 0.0F, 0.0F, 0.0F };

    vtkNew<CuppingArtifactFilter> Filter;
};


class CuppingArtifactWidget;

struct CuppingArtifactData {
    using Artifact = CuppingArtifact;
    using Widget = CuppingArtifactWidget;

    float DarkIntensityValue = 0.0F;

    std::array<float, 3> Center = { 0.0F, 0.0F, 0.0F };

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
    QDoubleSpinBox* DarkIntensityValueSpinBox;

    CoordinateRowWidget* CenterPointWidget;
};
