#pragma once

#include <vtkNew.h>
#include <vtkTransform.h>

#include <array>

class vtkTransform;

using vtkMTimeType = uint64_t;

using SimpleTransformData = std::array<std::array<float, 3>, 3>;

using Point = std::array<double, 3>;

struct RowMajor4x3Matrix {

    RowMajor4x3Matrix() = default;

    RowMajor4x3Matrix(RowMajor4x3Matrix&& other) noexcept {
        std::copy(other.Matrix.cbegin(), other.Matrix.cend(), Matrix.begin());
    };

    auto operator=(RowMajor4x3Matrix&& other) noexcept -> RowMajor4x3Matrix& {
        std::copy(other.Matrix.cbegin(), other.Matrix.cend(), Matrix.begin());
        return *this;
    };

    [[nodiscard]] auto
    operator() (uint8_t rowIdx, uint8_t colIdx) noexcept -> double& { return Matrix[4 * rowIdx + colIdx]; };

    [[nodiscard]] auto
    operator() (uint8_t rowIdx, uint8_t colIdx) const noexcept -> const double& { return Matrix[4 * rowIdx + colIdx]; };

    [[nodiscard]] constexpr auto
    data() noexcept -> double* { return Matrix.data(); }

private:
    std::array<double, 12> Matrix = { 1.0, 0.0, 0.0, 0.0,
                                      0.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0, 1.0, 0.0 };
};

class SimpleTransform {
public:
    SimpleTransform() = default;
    SimpleTransform(const SimpleTransform&) = delete;
    SimpleTransform(SimpleTransform&& other) = default;
    auto operator=(const SimpleTransform&) -> SimpleTransform& = delete;
    auto operator=(SimpleTransform&&) -> SimpleTransform& = default;
    ~SimpleTransform() = default;

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
    RowMajor4x3Matrix Matrix;
};

auto SimpleTransform::TransformPoint(Point point) const noexcept -> Point {
    return { Matrix(0, 0) * point[0] + Matrix(0, 1) * point[1] + Matrix(0, 2) * point[2] + Matrix(0, 3),
             Matrix(1, 0) * point[0] + Matrix(1, 1) * point[1] + Matrix(1, 2) * point[2] + Matrix(1, 3),
             Matrix(2, 0) * point[0] + Matrix(2, 1) * point[1] + Matrix(2, 2) * point[2] + Matrix(2, 3) };
}
