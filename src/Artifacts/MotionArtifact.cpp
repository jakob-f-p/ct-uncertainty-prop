#include "MotionArtifact.h"

auto MotionArtifact::EvaluateAtPosition(const FloatPoint& point) const noexcept -> float {
    qWarning("TODO: Implement");
    return 0;
}

auto MotionArtifactData::PopulateFromArtifact(const MotionArtifact& artifact) noexcept -> void {
    qWarning("Todo");
}

auto MotionArtifactData::PopulateArtifact(MotionArtifact& artifact) const noexcept -> void {
    qWarning("Todo");
}

MotionArtifactWidget::MotionArtifactWidget() {

}

auto MotionArtifactWidget::GetData() noexcept -> MotionArtifactWidget::Data {
    return MotionArtifactWidget::Data();
}

auto MotionArtifactWidget::Populate(const MotionArtifactWidget::Data& data) noexcept -> void {

}
