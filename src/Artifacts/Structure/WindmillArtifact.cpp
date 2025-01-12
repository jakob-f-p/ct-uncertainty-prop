#include "WindmillArtifact.h"

#include "../../Modeling/CtStructureTree.h"

#include <QDoubleSpinBox>
#include <QFormLayout>

#include <vtkVector.h>

#include <numbers>


auto WindmillArtifactData::PopulateFromArtifact(WindmillArtifact const& artifact) noexcept -> void {
    RadiodensityFactor = artifact.RadiodensityFactor;
    AngularWidth = artifact.AngularWidth;
    RotationPerSlice = artifact.RotationPerSlice;
    Length = artifact.Length;
}

auto WindmillArtifactData::PopulateArtifact(WindmillArtifact& artifact) const noexcept -> void {
    artifact.SetRadiodensityFactor(RadiodensityFactor);
    artifact.SetAngularWidth(AngularWidth);
    artifact.SetRotationPerSlice(RotationPerSlice);
    artifact.SetLength(Length);
}

WindmillArtifactWidget::WindmillArtifactWidget() :
        Layout(new QFormLayout(this)),
        RadiodensityFactorSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.01, 10.0);
            spinBox->setValue(1.0);
            spinBox->setSingleStep(0.1);
            spinBox->setDecimals(2);
            return spinBox;
        }()),
        AngularWidthSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.0, 360.0);
            spinBox->setValue(0.0);
            spinBox->setSingleStep(1.0);
            spinBox->setDecimals(2);
            spinBox->setSuffix("°");
            return spinBox;
        }()),
        RotationPerSliceSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.0, 360.0);
            spinBox->setValue(0.0);
            spinBox->setSingleStep(0.1);
            spinBox->setDecimals(2);
            spinBox->setSuffix("°");
            return spinBox;
        }()),
        LengthSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.0, 500.0);
            spinBox->setValue(0.0);
            spinBox->setSingleStep(2.0);
            spinBox->setDecimals(1);
            return spinBox;
        }()) {

    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    Layout->addRow("Radiodensity Factor", RadiodensityFactorSpinBox);
    Layout->addRow("Angular Width", AngularWidthSpinBox);
    Layout->addRow("Rotation per Slice", RotationPerSliceSpinBox);
    Layout->addRow("Length", LengthSpinBox);
}

auto WindmillArtifactWidget::GetData() const noexcept -> Data {
    Data data {};

    data.RadiodensityFactor = static_cast<float>(RadiodensityFactorSpinBox->value());
    data.AngularWidth = static_cast<float>(vtkMath::RadiansFromDegrees(AngularWidthSpinBox->value()));
    data.RotationPerSlice = static_cast<float>(vtkMath::RadiansFromDegrees(RotationPerSliceSpinBox->value()));
    data.Length = static_cast<float>(LengthSpinBox->value());

    return data;
}

auto WindmillArtifactWidget::Populate(Data const& data) const noexcept -> void {
    RadiodensityFactorSpinBox->setValue(data.RadiodensityFactor);
    AngularWidthSpinBox->setValue(vtkMath::DegreesFromRadians(data.AngularWidth));
    RotationPerSliceSpinBox->setValue(vtkMath::DegreesFromRadians(data.RotationPerSlice));
    LengthSpinBox->setValue(data.Length);
}

auto WindmillArtifact::SetRadiodensityFactor(float factor) noexcept -> void {
    if (RadiodensityFactor == factor)
        return;

    RadiodensityFactor = factor;
    Modified();
}

auto WindmillArtifact::SetAngularWidth(float angularWidth) noexcept -> void {
    if (AngularWidth == angularWidth)
        return;

    AngularWidth = angularWidth;
    Modified();
}

auto WindmillArtifact::SetRotationPerSlice(float rotationPerSlice) noexcept -> void {
    if (RotationPerSlice == rotationPerSlice)
        return;

    RotationPerSlice = rotationPerSlice;
    Modified();
}

auto WindmillArtifact::SetLength(float length) noexcept -> void {
    if (Length == length)
        return;

    Length = length;
    Modified();
}

auto WindmillArtifact::EvaluateAtPosition(DoublePoint const& point,
                                          float const maxRadiodensity,
                                          bool const pointOccupiedByStructure,
                                          CtStructureTree const& structureTree,
                                          CtStructureVariant const& structure,
                                          std::array<double, 3> const& spacing) const noexcept -> float {
    if (pointOccupiedByStructure || maxRadiodensity < 0.0F)
        return 0.0F;

    auto const closestXYPoint = structureTree.ClosestPointOnXYPlane(point, structure);
    if (!closestXYPoint || point == *closestXYPoint)
        return 0.0F;

    if (auto const radiodensity = structureTree.FunctionValueAndRadiodensity(point, &structure).Radiodensity;
        radiodensity <= 0.0F)
        return 0.0F;

    vtkVector2 const difference { static_cast<float>((*closestXYPoint)[0] - point[0]),
                                         static_cast<float>((*closestXYPoint)[1] - point[1]) };
    float const distance = std::sqrt(difference.SquaredNorm());
    float const distanceFactor = std::max(1.0F - distance / Length, 0.0F);

    float const omega = std::numbers::pi / AngularWidth;
    float const angle = std::atan2(difference[1], difference[0]);
    static constexpr float startOffset = 0.75 * std::numbers::pi;
    float const zOffset = point[2] / spacing[2] * RotationPerSlice;
    float const cosine = cos(omega * angle + startOffset + zOffset);
    float const directionFactor = cosine * cosine;

    return maxRadiodensity * distanceFactor * directionFactor * RadiodensityFactor;
}

auto WindmillArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    PipelineParameterProperties properties;
    properties.Add(
            FloatObjectProperty("Radiodensity Factor",
                                [this] { return GetRadiodensityFactor(); },
                                [this](float factor) { this->SetRadiodensityFactor(factor); },
                                { 0.01, 10.00, 0.01, 2 }));
    properties.Add(
            FloatObjectProperty("Angular Width",
                                [this] { return vtkMath::DegreesFromRadians(GetAngularWidth()); },
                                [this](float angularWidth) {
                this->SetAngularWidth(vtkMath::RadiansFromDegrees(angularWidth));
                },
                                { 0.0, 360.0, 1.0, 2 }));
    properties.Add(
            FloatObjectProperty("Rotation per Slice",
                                [this] { return vtkMath::DegreesFromRadians(GetRotationPerSlice()); },
                                [this](float rotationPerSlice) {
                this->SetRotationPerSlice(vtkMath::RadiansFromDegrees(rotationPerSlice));
                },
                                { 0.0, 360.0, 1.0, 2 }));
    properties.Add(
            FloatObjectProperty("Length",
                                [this] { return GetLength(); },
                                [this](float length) { this->SetLength(length); },
                                { 0.0, 500.0, 2.0, 1 }));
    return properties;
}
