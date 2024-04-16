#pragma once

#include <vtkNew.h>
#include <vtkTransform.h>

#include <array>

class vtkTransform;

using vtkMTimeType = uint64_t;

using SimpleTransformData = std::array<std::array<float, 3>, 3>;

using Point = std::array<double, 3>;

constexpr int N4x4 = 16;

using RowMajor4x4Matrix = std::array<std::array<double, 4>, 4>;

class SimpleTransform {
public:
    SimpleTransform() = default;
    SimpleTransform(const SimpleTransform&) = delete;
    SimpleTransform(SimpleTransform&&) = default;
    auto operator=(const SimpleTransform&) -> SimpleTransform& = delete;
    auto operator=(SimpleTransform&&) -> SimpleTransform& = default;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

    auto
    Modified() noexcept -> void;

    [[nodiscard]] auto
    GetData() const noexcept -> SimpleTransformData;

    auto
    SetData(const SimpleTransformData& transformData) noexcept -> void;

    [[nodiscard]] inline auto
    TransformPoint(Point point) const noexcept -> Point;

private:
    std::array<float, 3> TranslationValues {};
    std::array<float, 3> RotationAngles {};
    std::array<float, 3> ScaleFactors { 1.0, 1.0, 1.0 };

    vtkNew<vtkTransform> Transform;
    RowMajor4x4Matrix Matrix { 1.0, 0.0, 0.0, 0.0,
                               0.0, 1.0, 0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0 };
};

auto SimpleTransform::TransformPoint(Point point) const noexcept -> Point {
    return { Matrix[0][0] * point[0] + Matrix[0][1] * point[1] + Matrix[0][2] * point[2],
             Matrix[1][0] * point[0] + Matrix[1][1] * point[1] + Matrix[1][2] * point[2],
             Matrix[2][0] * point[0] + Matrix[2][1] * point[1] + Matrix[2][2] * point[2] };
}

