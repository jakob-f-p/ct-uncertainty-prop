#include "MotionArtifact.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>

auto MotionArtifactData::PopulateFromArtifact(MotionArtifact const& artifact) noexcept -> void {
    Transform = artifact.Transform.GetData();
    CtNumberFactor = artifact.CtNumberFactor;
}

auto MotionArtifactData::PopulateArtifact(MotionArtifact& artifact) const noexcept -> void {
    artifact.Transform.SetData(Transform);
    artifact.CtNumberFactor = CtNumberFactor;
}

MotionArtifactWidget::MotionArtifactWidget() :
            Layout(new QFormLayout(this)),
            CtNumberFactorSpinBox(new QDoubleSpinBox()),
            TransformWidget(new SimpleTransformWidget()) {

        Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
        Layout->setHorizontalSpacing(15);

        CtNumberFactorSpinBox->setRange(0.01, 20.0);
        CtNumberFactorSpinBox->setValue(1.0);
        CtNumberFactorSpinBox->setSingleStep(0.1);
        Layout->addRow("CT number multiplier", CtNumberFactorSpinBox);

        auto* transformGroup = new QGroupBox("Move Transform");
        auto* transformLayout = new QVBoxLayout(transformGroup);
        transformLayout->addWidget(TransformWidget);
        Layout->addRow(transformGroup);
}

auto MotionArtifactWidget::GetData() noexcept -> MotionArtifactWidget::Data {
    Data data {};

    data.Transform = TransformWidget->GetData();
    data.CtNumberFactor = static_cast<float>(CtNumberFactorSpinBox->value());

    return data;
}

auto MotionArtifactWidget::Populate(MotionArtifactWidget::Data const& data) noexcept -> void {
    TransformWidget->SetData(data.Transform);
    CtNumberFactorSpinBox->setValue(data.CtNumberFactor);
}
