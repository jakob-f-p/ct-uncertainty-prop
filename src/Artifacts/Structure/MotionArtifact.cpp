#include "MotionArtifact.h"

#include "../../Modeling/CtStructureTree.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>

#include <numbers>


auto MotionArtifactData::PopulateFromArtifact(MotionArtifact const& artifact) noexcept -> void {
    RadiodensityFactor = artifact.RadiodensityFactor;
    KernelRadius = artifact.KernelRadius;
    KernelSd = artifact.KernelSd;
    Transform = artifact.Transform.GetData();
}

auto MotionArtifactData::PopulateArtifact(MotionArtifact& artifact) const noexcept -> void {
    artifact.SetRadiodensityFactor(RadiodensityFactor);
    artifact.SetBlurKernelRadius(KernelRadius);
    artifact.SetBlurKernelStandardDeviation(KernelSd);
    artifact.SetTransform(Transform);
}

MotionArtifactWidget::MotionArtifactWidget() :
        Layout(new QFormLayout(this)),
        RadiodensitySpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.01, 20.0);
            spinBox->setValue(1.0);
            spinBox->setSingleStep(0.1);
            return spinBox;
        }()),
        KernelRadiusSpinBox([]() {
            auto* spinBox = new QSpinBox();
            spinBox->setRange(0, 20);
            spinBox->setValue(4);
            spinBox->setSingleStep(1);
            return spinBox;
        }()),
        KernelSdSpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setRange(0.01, 20.0);
            spinBox->setValue(1.0);
            spinBox->setSingleStep(0.1);
            return spinBox;
        }()),
        TransformWidget(new SimpleTransformWidget()) {

    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    Layout->addRow("Radiodensity Factor", RadiodensitySpinBox);

    Layout->addRow("Blur Kernel Radius", KernelRadiusSpinBox);
    Layout->addRow("Blur Kernel Standard Deviation", KernelSdSpinBox);

    auto* transformGroup = new QGroupBox("Move Transform");
    auto* transformLayout = new QVBoxLayout(transformGroup);
    transformLayout->addWidget(TransformWidget);
    Layout->addRow(transformGroup);
}

auto MotionArtifactWidget::GetData() noexcept -> MotionArtifactWidget::Data {
    Data data {};

    data.RadiodensityFactor = static_cast<float>(RadiodensitySpinBox->value());
    data.KernelRadius = static_cast<uint16_t>(KernelRadiusSpinBox->value());
    data.KernelSd = static_cast<float>(KernelSdSpinBox->value());
    data.Transform = TransformWidget->GetData();

    return data;
}

auto MotionArtifactWidget::Populate(MotionArtifactWidget::Data const& data) noexcept -> void {
    RadiodensitySpinBox->setValue(data.RadiodensityFactor);
    KernelRadiusSpinBox->setValue(data.KernelRadius);
    KernelSdSpinBox->setValue(data.KernelSd);
    TransformWidget->SetData(data.Transform);
}

auto MotionArtifact::SetRadiodensityFactor(float factor) noexcept -> void {
    if (RadiodensityFactor == factor)
        return;

    RadiodensityFactor = factor;
    Modified();
}

auto MotionArtifact::SetBlurKernelRadius(uint16_t kernelRadius) noexcept -> void {
    if (KernelRadius == kernelRadius)
        return;

    KernelRadius = kernelRadius;
    UpdateKernel();
    Modified();
}

auto MotionArtifact::SetBlurKernelStandardDeviation(float sd) noexcept -> void {
    if (KernelSd == sd)
        return;

    KernelSd = sd;
    UpdateKernel();
    Modified();
}

auto MotionArtifact::SetTransform(SimpleTransformData const& data) noexcept -> void {
    if (Transform.GetData() == data)
        return;

    Transform.SetData(data);
    Modified();
}

auto MotionArtifact::EvaluateAtPosition(DoublePoint const& point,
                                        float /*maxRadiodensity*/,
                                        bool pointOccupiedByStructure,
                                        CtStructureTree const& structureTree,
                                        CtStructureVariant const& structure,
                                        std::array<double, 3> spacing) const noexcept -> float {
    if (pointOccupiedByStructure)
        return 0.0F;

    DoublePoint const transformedPoint = Transform.TransformPoint(point);

    auto const functionValueRadiodensity = structureTree.FunctionValueAndRadiodensity(transformedPoint, &structure);

    float const result = functionValueRadiodensity.FunctionValue < 0.0F
            ? functionValueRadiodensity.Radiodensity * RadiodensityFactor
            : 0.0F;

    if (KernelRadius < 1 || KernelSd <= 0.0F)
        return result;

    std::vector<float> convolvedValues { Kernel2D.size(), std::allocator<float>{} };
    size_t const kernel1DSize = KernelRadius * 2 + 1;
    {
        DoublePoint pointOther { transformedPoint[0],
                                 transformedPoint[1] - (KernelRadius * spacing[1]),
                                 transformedPoint[2] };

        for (size_t iOther = 0; iOther < kernel1DSize; ++iOther) {
            DoublePoint pointBlur { pointOther[0] - (KernelRadius * spacing[0]),
                                    pointOther[1],
                                    pointOther[2] };

            for (size_t iBlur = 0; iBlur < kernel1DSize; ++iBlur) {
                auto const kernelResult = structureTree.FunctionValueAndRadiodensity(pointBlur, &structure);
                size_t const idx = iOther * kernel1DSize + iBlur;
                convolvedValues[idx] = kernelResult.FunctionValue < 0.0F
                                               ? kernelResult.Radiodensity * RadiodensityFactor
                                               : 0.0F;
                convolvedValues[idx] *= Kernel2D[idx];

                pointBlur[0] += spacing[0];
            }

            pointOther[1] += spacing[1];
        }
    }

    return std::reduce(convolvedValues.begin(), convolvedValues.end());
}

auto MotionArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    PipelineParameterProperties properties;
    properties.Add(
            FloatObjectProperty("Radiodensity Factor",
                                [this] { return GetRadiodensityFactor(); },
                                [this](float factor) { this->SetRadiodensityFactor(factor); },
                                { 0.01, 10.00, 0.01, 2 }));
    properties.Add(
            FloatObjectProperty("Blur Standard Deviation",
                                [this] { return GetBlurKernelStandardDeviation(); },
                                [this](float factor) { this->SetBlurKernelStandardDeviation(factor); },
                                { 0.01, 10.00, 0.01, 2 }));
    return properties;
}

auto MotionArtifact::UpdateKernel() noexcept -> void {
    static constexpr auto gaussianFunction = [](double sd, double x, double y) {
        double const twoVariance = 2.0 * sd * sd;
        return (1.0 / (std::sqrt(std::numbers::pi * twoVariance)) * std::exp(-(x * x + y * y / twoVariance)));
    };

    size_t const kernel1DSize = KernelRadius * 2 + 1;

    Kernel2D.clear();
    Kernel2D.resize(kernel1DSize * kernel1DSize);

    for (int i = 0; i < kernel1DSize; ++i)
        for (int j = 0; j < kernel1DSize; ++j)
            Kernel2D[i * kernel1DSize + j] = static_cast<float>(
                    gaussianFunction(KernelSd,
                                     static_cast<double>(i - KernelRadius),
                                     static_cast<double>(j - KernelRadius)));

    float const kernelSum = std::reduce(Kernel2D.begin(), Kernel2D.end());

    for (float& value : Kernel2D)
        value /= kernelSum;
}
