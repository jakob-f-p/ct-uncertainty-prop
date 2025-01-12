#include "MetalArtifact.h"

#include "../../Modeling/CtStructureTree.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>

auto MetalArtifactData::PopulateFromArtifact(MetalArtifact const& artifact) noexcept -> void {
    DirectionHighestAttenuation = artifact.DirectionHighestAttenuation;
    MaxAttenuationFactor = artifact.MaxAttenuationFactor;
    Length = artifact.Length;
}

auto MetalArtifactData::PopulateArtifact(MetalArtifact& artifact) const noexcept -> void {
    artifact.SetMaxAttenuationFactor(MaxAttenuationFactor);
    artifact.SetDirectionOfHighestAttenuation(DirectionHighestAttenuation);
    artifact.SetLength(Length);
}

MetalArtifactWidget::MetalArtifactWidget() :
        Layout(new QFormLayout(this)),
        DirectionXSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(-1.0, 1.0);
            spinBox->setSingleStep(0.1);
            spinBox->setDecimals(2);
            spinBox->setValue(1.0);
            return spinBox;
        }()),
        DirectionYSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(-1.0, 1.0);
            spinBox->setSingleStep(0.1);
            spinBox->setDecimals(2);
            spinBox->setValue(0.0);
            return spinBox;
        }()),
        MaxAttenuationFactorSpinBox([] {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.01, 1.0);
            spinBox->setValue(1.0);
            spinBox->setSingleStep(0.1);
            spinBox->setDecimals(2);
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

    auto* directionHLayout = new QHBoxLayout();
    directionHLayout->setSpacing(10);
    directionHLayout->addWidget(new QLabel("x"));
    directionHLayout->addWidget(DirectionXSpinBox);
    directionHLayout->addSpacing(10);
    directionHLayout->addWidget(new QLabel("y"));
    directionHLayout->addWidget(DirectionYSpinBox);
    Layout->addRow("Direction Highest Attenuation", directionHLayout);

    Layout->addRow("Max Attenuation Factor", MaxAttenuationFactorSpinBox);

    Layout->addRow("Length", LengthSpinBox);
}

auto MetalArtifactWidget::GetData() const noexcept -> Data {
    Data data {};

    data.DirectionHighestAttenuation = { static_cast<float>(DirectionXSpinBox->value()),
                                         static_cast<float>(DirectionYSpinBox->value()) };
    data.MaxAttenuationFactor = static_cast<float>(MaxAttenuationFactorSpinBox->value());
    data.Length = static_cast<float>(LengthSpinBox->value());

    return data;
}

auto MetalArtifactWidget::Populate(Data const& data) const noexcept -> void {
    DirectionXSpinBox->setValue(data.DirectionHighestAttenuation.GetX());
    DirectionYSpinBox->setValue(data.DirectionHighestAttenuation.GetY());
    MaxAttenuationFactorSpinBox->setValue(data.MaxAttenuationFactor);
    LengthSpinBox->setValue(data.Length);
}

auto MetalArtifact::SetDirectionOfHighestAttenuation(vtkVector2<float> direction) noexcept -> void {
    if (DirectionHighestAttenuation == direction)
        return;

    DirectionHighestAttenuation = direction;
    DirectionHighestAttenuationNormed = { direction.Normalized()[0], direction.Normalized()[1] };
    Modified();
}

auto MetalArtifact::SetMaxAttenuationFactor(float factor) noexcept -> void {
    if (factor == MaxAttenuationFactor)
        return;

    MaxAttenuationFactor = factor;

    MaxAttenuationChangeFactor = MaxAttenuationFactor - 1.0F;

    FactorRange = -2.0F * MaxAttenuationChangeFactor;

    Modified();
}

auto MetalArtifact::SetLength(float length) noexcept -> void {
    if (length == Length)
        return;

    Length = length;
    Modified();
}

auto MetalArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    PipelineParameterProperties properties;
    properties.Add(
            FloatObjectProperty("Maximum Attenuation Factor",
                                [this] { return GetMaxAttenuationFactor(); },
                                [this](float factor) { this->SetMaxAttenuationFactor(factor); },
                                { 0.01, 1.00, 0.01, 2 }));
    properties.Add(
            FloatObjectProperty("Length",
                                [this] { return GetLength(); },
                                [this](float length) { this->SetLength(length); },
                                { 0.1, 500.0, 2.0, 1 }));
    return properties;
}

auto MetalArtifact::EvaluateAtPosition(DoublePoint const& point,
                                       float maxRadiodensity,
                                       bool pointOccupiedByStructure,
                                       CtStructureTree const& structureTree,
                                       CtStructureVariant const& structure,
                                       std::array<double, 3> /*spacing*/) const noexcept -> float {
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

    float const cosAngle = difference.Dot(DirectionHighestAttenuationNormed) / distance;
    float const inverseCosAngleSquared = 1.0F - cosAngle * cosAngle;
    float const directionFactor = MaxAttenuationChangeFactor + inverseCosAngleSquared * FactorRange;

    return maxRadiodensity * distanceFactor * directionFactor;
}
