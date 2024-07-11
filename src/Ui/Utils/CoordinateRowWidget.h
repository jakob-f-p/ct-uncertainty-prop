#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QGridLayout;
class QSpinBox;


class CoordinateRowWidgetSignals : public QWidget {
    Q_OBJECT

Q_SIGNALS:
    void ValueChanged();
};


template<typename ValueT, typename SpinBoxT>
class CoordinateRowWidget : public CoordinateRowWidgetSignals {
public:
    struct NumericSettings {
        ValueT Min {};
        ValueT Max {};
        ValueT StepSize {};
        ValueT DefaultValue {};
        QString Suffix;
    };

    explicit CoordinateRowWidget(bool hasLabel);
    explicit CoordinateRowWidget(NumericSettings numericSettings, const QString& labelText = {});

    auto
    AppendCoordinatesRow(const NumericSettings& numericSettings, const QString& labelText = {}) -> void;

    struct RowData {
        ValueT X;
        ValueT Y;
        ValueT Z;

        RowData(ValueT x, ValueT y, ValueT z) : X(x), Y(y), Z(z) {};
        explicit RowData(std::array<ValueT, 3> const& array) : X(array[0]), Y(array[1]), Z(array[2]) {};

        [[nodiscard]] auto
        ToArray() const noexcept -> std::array<ValueT, 3> { return { X, Y, Z }; }
    };

    [[nodiscard]] auto
    GetRowData(uint8_t rowIdx) const -> RowData;

    [[nodiscard]] auto
    GetRowData() const noexcept -> std::vector<RowData>;

    auto
    SetRowData(uint8_t rowIdx, RowData data) -> void;

private:
    struct SpinBoxRow {
        SpinBoxT* X;
        SpinBoxT* Y;
        SpinBoxT* Z;

        auto
        SetSpinBox(uint8_t idx, SpinBoxT& spinBox) -> void {
            switch (idx) {
                case 0: X = &spinBox; break;
                case 1: Y = &spinBox; break;
                case 2: Z = &spinBox; break;
                default: return;
            }
        }
        auto operator[] (uint8_t idx) -> SpinBoxT* {
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

template<>
struct CoordinateRowWidget<double, QDoubleSpinBox>::NumericSettings {
    double Min = -100.0;
    double Max = 100.0;
    double StepSize = 1.0;
    double DefaultValue = 0.0;
    uint8_t Decimals = 1;
    QString Suffix = "";
};

template<>
struct CoordinateRowWidget<double, QDoubleSpinBox>::RowData {
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

class DoubleCoordinateRowWidget : public CoordinateRowWidget<double, QDoubleSpinBox> {
    Q_OBJECT

public:
    using CoordinateRowWidget::CoordinateRowWidget;
};

class IntegerCoordinateRowWidget : public CoordinateRowWidget<int, QSpinBox> {
    Q_OBJECT

public:
    using CoordinateRowWidget::CoordinateRowWidget;
};
