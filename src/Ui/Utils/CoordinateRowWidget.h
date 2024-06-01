#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QGridLayout;

class CoordinateRowWidget : public QWidget {
    Q_OBJECT

public:
    struct NumericSettings {
        double Min = -100.0;
        double Max = 100.0;
        double StepSize = 1.0;
        double DefaultValue = 0.0;
        uint8_t Decimals = 1;
        QString Suffix = "";
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
        explicit RowData(const std::array<float, 3>& array) : X(array[0]), Y(array[1]), Z(array[2]) {};

        [[nodiscard]] auto
        ToArray() const noexcept -> std::array<double, 3> { return { X, Y, Z }; }

        [[nodiscard]] auto
        ToFloatArray() const noexcept -> std::array<float, 3> { return { static_cast<float>(X),
                                                                         static_cast<float>(Y),
                                                                         static_cast<float>(Z) }; }
    };

    [[nodiscard]] auto
    GetRowData(uint8_t rowIdx) const -> RowData;

    [[nodiscard]] auto
    GetRowData() const noexcept -> std::vector<RowData>;

    auto
    SetRowData(uint8_t rowIdx, RowData data) -> void;

Q_SIGNALS:
    void ValueChanged();

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

