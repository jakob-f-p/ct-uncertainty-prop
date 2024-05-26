#pragma once

#include "Types.h"

#include <QWidget>

#include <vtkNew.h>

#include <array>

class CoordinateRowWidget;

class QDoubleSpinBox;
class QGridLayout;
class vtkTransform;

using SimpleTransformData = std::array<DoubleVector, 3>;

using Point = DoubleVector;

struct RowMajor4x3Matrix {

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
    SimpleTransform(SimpleTransform const& other);
    auto operator= (SimpleTransform const&) -> SimpleTransform& = delete;
    SimpleTransform(SimpleTransform&&) = default;
    auto operator= (SimpleTransform&&) -> SimpleTransform& = default;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

    auto
    Modified() noexcept -> void;

    [[nodiscard]] auto
    GetData() const noexcept -> SimpleTransformData;

    auto
    SetData(SimpleTransformData const& transformData) noexcept -> void;

    [[nodiscard]] inline auto
    TransformPoint(Point point) const noexcept -> Point;

private:
    std::array<double, 3> TranslationValues {};
    std::array<double, 3> RotationAngles {};
    std::array<double, 3> ScaleFactors { 1.0, 1.0, 1.0 };

    vtkNew<vtkTransform> Transform;
    RowMajor4x3Matrix Matrix;
};

auto SimpleTransform::TransformPoint(Point point) const noexcept -> Point {
    return { Matrix(0, 0) * point[0] + Matrix(0, 1) * point[1] + Matrix(0, 2) * point[2] + Matrix(0, 3),
             Matrix(1, 0) * point[0] + Matrix(1, 1) * point[1] + Matrix(1, 2) * point[2] + Matrix(1, 3),
             Matrix(2, 0) * point[0] + Matrix(2, 1) * point[1] + Matrix(2, 2) * point[2] + Matrix(2, 3) };
}


class SimpleTransformWidget : public QWidget {
    Q_OBJECT

public:
    SimpleTransformWidget();

    [[nodiscard]] auto
    GetData() noexcept -> SimpleTransformData;

    auto
    SetData(const SimpleTransformData& data) noexcept -> void;

private:
    CoordinateRowWidget* TransformRows;
};