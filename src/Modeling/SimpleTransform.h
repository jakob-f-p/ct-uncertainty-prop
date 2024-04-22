#pragma once

#include <QWidget>

#include <vtkNew.h>

#include <array>

class QDoubleSpinBox;
class QGridLayout;
class vtkTransform;

using vtkMTimeType = uint64_t;

using SimpleTransformData = std::array<std::array<double, 3>, 3>;

using Point = std::array<double, 3>;

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

class CoordinateRowWidget : public QWidget {
    Q_OBJECT

public:
    struct NumericSettings {
        double Min;
        double Max;
        double StepSize;
        double DefaultValue;
        QString Suffix;
    };

    explicit CoordinateRowWidget(bool hasLabel);
    explicit CoordinateRowWidget(NumericSettings numericSettings, const QString& labelText = {});

    auto
    AppendCoordinatesRow(const NumericSettings& numericSettings, const QString& labelText = {}) -> void;

    struct RowData {
        double X;
        double Y;
        double Z;

        RowData(double x, double y, double z) : X(x), Y(y), Z(z) {};
        explicit RowData(const std::array<double, 3>& array) : X(array[0]), Y(array[1]), Z(array[2]) {};

        [[nodiscard]] auto
        ToArray() const noexcept -> std::array<double, 3> { return { X, Y, Z }; }
    };

    [[nodiscard]] auto
    GetRowData(uint8_t rowIdx) const -> RowData;

    [[nodiscard]] auto
    GetRowData() const noexcept -> std::vector<RowData>;

    auto
    SetRowData(uint8_t rowIdx, RowData data) -> void;

private:
    struct SpinBoxRow {
        QDoubleSpinBox* X;
        QDoubleSpinBox* Y;
        QDoubleSpinBox* Z;

        auto
        SetSpinBox(uint8_t idx, QDoubleSpinBox& spinBox) -> void {
            switch (idx) {
                case 0: X = &spinBox; break;
                case 1: Y = &spinBox; break;
                case 2: Z = &spinBox; break;
            }
        }
        auto operator[] (uint8_t idx) -> QDoubleSpinBox* {
            switch (idx) {
                case 0: return X;
                case 1: return Y;
                case 2: return Z;
                default: return nullptr;
            }
        }
    };

    std::vector<SpinBoxRow> Rows;

    bool HasLabel;
    QGridLayout* GLayout;

    const QStringList AxisNames = { "x", "y", "z" };
};

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