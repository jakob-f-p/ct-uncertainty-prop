#pragma once

#include <QWidget>

#include <array>

using FloatPoint = std::array<float, 3>;

struct MotionArtifactData;

class MotionArtifact {
public:
    using Data = MotionArtifactData;

    [[nodiscard]] auto
    EvaluateAtPosition(const FloatPoint& point) const noexcept -> float;
};


struct MotionArtifactData {
    using Artifact = MotionArtifact;

    float abc;

    auto
    PopulateFromArtifact(const MotionArtifact& artifact) noexcept -> void;

    auto
    PopulateArtifact(MotionArtifact& artifact) const noexcept -> void;
};


class MotionArtifactWidget : public QWidget {
public:
    using Data = MotionArtifactData;

    MotionArtifactWidget();

    [[nodiscard]] auto
    GetData() noexcept -> Data;

    auto
    Populate(const Data& data) noexcept -> void;
};
