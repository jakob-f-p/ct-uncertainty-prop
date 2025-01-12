#pragma once

#include <QSlider>
#include <QStyle>

class QLabel;


class RangeSlider : public QSlider {
    Q_OBJECT

public:
    struct Range {
        int Min;
        int Max;

        auto
        operator+= (int n) noexcept -> Range& {
            Min += n;
            Max += n;
            return *this;
        }

        [[nodiscard]] auto
        operator== (Range const&) const noexcept -> bool = default;
    };

    struct SliderSettings {
        int Min;
        int Max;
        int Step;
    };

    explicit RangeSlider(SliderSettings sliderSettings, QWidget* parent = nullptr);

    [[nodiscard]] auto
    GetValue() const noexcept -> Range;

    auto
    SetValue(Range range) -> void;

Q_SIGNALS:
    auto
    ValueChanged(Range range) -> void;

protected:
    auto
    paintEvent(QPaintEvent* event) -> void override;

    auto
    mousePressEvent(QMouseEvent* event) -> void override;

    auto
    mouseMoveEvent(QMouseEvent* event) -> void override;

    auto
    mouseReleaseEvent(QMouseEvent* event) -> void override;

private:
    [[nodiscard]] auto
    PixelPosToRangeValue(int pos) const -> int;

    enum struct ActiveSlider { INVALID, LOW, HIGH };

    Range Range_;
    Range InitialRange;
    QStyle::SubControl PressedControl;
    ActiveSlider ActiveSlider_;
    int LastPos;
};


class LabeledRangeSlider : public QWidget {
    Q_OBJECT

public:
    explicit LabeledRangeSlider(QString const& labelText,
                                RangeSlider::SliderSettings sliderSettings,
                                QWidget* parent = nullptr);

    [[nodiscard]] auto
    GetValue() const noexcept -> RangeSlider::Range;

    auto
    SetValue(RangeSlider::Range range) const -> void;

Q_SIGNALS:
    auto
    ValueChanged(RangeSlider::Range range) -> void;

private:
    RangeSlider* Slider;
    QLabel* LowLabel;
    QLabel* HighLabel;
    QLabel* TextLabel;
};