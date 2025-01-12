#pragma once

#include "LinearAlgebraTypes.h"

#include <QWidget>

#include <vtkNew.h>
#include <vtkType.h>

#include <array>

class DoubleCoordinateRowWidget;

class QDoubleSpinBox;
class QGridLayout;
class vtkTransform;

using SimpleTransformData = std::array<DoubleVector, 3>;

using Point = DoubleVector;

struct RowMajor4x3Matrix {

    [[nodiscard]] auto
    operator() (uint8_t rowIdx, uint8_t colIdx) noexcept -> double& { return Matrix[4 * rowIdx + colIdx]; }

    [[nodiscard]] auto
    operator() (uint8_t rowIdx, uint8_t colIdx) const noexcept -> const double& { return Matrix[4 * rowIdx + colIdx]; }

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
    Modified() const noexcept -> void;

    [[nodiscard]] auto
    GetData() const noexcept -> SimpleTransformData;

    auto
    SetData(SimpleTransformData const& transformData) noexcept -> void;

    [[nodiscard]] inline auto
    TransformPoint(Point const& point) const noexcept -> Point;

    [[nodiscard]] inline auto
    InverseTransformPoint(Point const& point) const noexcept -> Point;

    [[nodiscard]] auto
    LeftMultiply(SimpleTransform const& other) const noexcept -> SimpleTransform;

    [[nodiscard]] auto
    RightMultiply(SimpleTransform const& other) const noexcept -> SimpleTransform;

private:
    std::array<double, 3> TranslationValues {};
    std::array<double, 3> RotationAngles {};
    std::array<double, 3> ScaleFactors { 1.0, 1.0, 1.0 };

    vtkNew<vtkTransform> Transform;
    RowMajor4x3Matrix Matrix;

    vtkNew<vtkTransform> InverseTransform;
    RowMajor4x3Matrix InverseMatrix;
};

auto SimpleTransform::TransformPoint(Point const& point) const noexcept -> Point {
    return { Matrix(0, 0) * point[0] + Matrix(0, 1) * point[1] + Matrix(0, 2) * point[2] + Matrix(0, 3),
             Matrix(1, 0) * point[0] + Matrix(1, 1) * point[1] + Matrix(1, 2) * point[2] + Matrix(1, 3),
             Matrix(2, 0) * point[0] + Matrix(2, 1) * point[1] + Matrix(2, 2) * point[2] + Matrix(2, 3) };
}

auto SimpleTransform::InverseTransformPoint(Point const& point) const noexcept -> Point {
    return { InverseMatrix(0, 0) * point[0] + InverseMatrix(0, 1) * point[1] + InverseMatrix(0, 2) * point[2] + InverseMatrix(0, 3),
             InverseMatrix(1, 0) * point[0] + InverseMatrix(1, 1) * point[1] + InverseMatrix(1, 2) * point[2] + InverseMatrix(1, 3),
             InverseMatrix(2, 0) * point[0] + InverseMatrix(2, 1) * point[1] + InverseMatrix(2, 2) * point[2] + InverseMatrix(2, 3) };
}



class SimpleTransformWidget : public QWidget {
    Q_OBJECT

public:
    SimpleTransformWidget();

    [[nodiscard]] auto
    GetData() const noexcept -> SimpleTransformData;

    auto
    SetData(const SimpleTransformData& data) const noexcept -> void;

private:
    DoubleCoordinateRowWidget* TransformRows;
};