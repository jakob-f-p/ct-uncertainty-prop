#include "RangeSlider.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>
#include <QVBoxLayout>


RangeSlider::RangeSlider(SliderSettings sliderSettings, QWidget* parent) :
        QSlider(parent),
        Range_([sliderSettings]() {
            int const diff = sliderSettings.Max - sliderSettings.Min;
            int const diff_quarter = diff / 4;
            return Range { sliderSettings.Min + diff_quarter, sliderSettings.Max - diff_quarter };
        }()),
        PressedControl(QStyle::SC_None),
        ActiveSlider_(ActiveSlider::INVALID),
        LastPos(0) {

    setOrientation(Qt::Orientation::Horizontal);
    setTickPosition(TickPosition::NoTicks);
    setMinimum(sliderSettings.Min);
    setMaximum(sliderSettings.Max);
    setSingleStep(sliderSettings.Step);
}

auto RangeSlider::GetValue() const noexcept -> RangeSlider::Range { return Range_; }

auto RangeSlider::SetValue(RangeSlider::Range range) -> void {
    if (range.Min >= range.Max || range.Min < minimum() || range.Max > maximum())
        return;

    Range_ = range;

    Q_EMIT ValueChanged(Range_);
}

void RangeSlider::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter { this };

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    opt.sliderValue = 0;
    opt.sliderPosition = 0;
    opt.subControls = QStyle::SC_SliderGroove;

    // Draw groove
    style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);
    QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);

    // Draw span
    opt.sliderPosition = Range_.Min;
    QRect const lowRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    opt.sliderPosition = Range_.Max;
    QRect const highRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    int const lowPos  = lowRect.center().x();
    int const highPos = highRect.center().x();
    int const minPos = std::min(lowPos, highPos);
    int const maxPos = std::max(lowPos, highPos);

    QPoint const center = QRect(lowRect.center(), highRect.center()).center();
    QRect const spanRect { QPoint(minPos, center.y() - 2), QPoint(maxPos, center.y() + 1) };
    groove.adjust(0,0,-1,0);

    QColor const highlight = palette().color(QPalette::WindowText);
    painter.setBrush(QBrush(highlight));
    painter.setPen(QPen(highlight,0));
    painter.drawRect(spanRect.intersected(groove));

    opt.subControls = QStyle::SC_SliderHandle;
    if (PressedControl != QStyle::SC_None)
        opt.activeSubControls = PressedControl;
    opt.sliderPosition = Range_.Min;
    opt.sliderValue = Range_.Min;
    style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);

    if (PressedControl != QStyle::SC_None)
        opt.activeSubControls = PressedControl;
    opt.sliderPosition = Range_.Max;
    opt.sliderValue = Range_.Max;
    style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);
}

void RangeSlider::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    event->accept();

    InitialRange = Range_;

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    ActiveSlider_ = ActiveSlider::INVALID;

    opt.sliderPosition = Range_.Min;
    QStyle::SubControl hit = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, event->pos(), this);
    if (hit == QStyle::SC_SliderHandle) {
        ActiveSlider_ = ActiveSlider::LOW;
        PressedControl = hit;
        triggerAction(SliderMove);
        setRepeatAction(SliderNoAction);
        setSliderDown(true);
    }

    opt.sliderPosition = Range_.Max;
    hit = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, event->pos(), this);
    if (hit == QStyle::SC_SliderHandle) {
        ActiveSlider_ = ActiveSlider::HIGH;
        PressedControl = hit;
        triggerAction(SliderMove);
        setRepeatAction(SliderNoAction);
        setSliderDown(true);
    }

    if (ActiveSlider_ == ActiveSlider::INVALID) {
        PressedControl = hit;
        LastPos = PixelPosToRangeValue(event->pos().x());
        triggerAction(SliderMove);
        setRepeatAction(SliderNoAction);
    }

    QSlider::mousePressEvent(event);
}

void RangeSlider::mouseMoveEvent(QMouseEvent* event) {
    if (PressedControl != QStyle::SC_SliderHandle && PressedControl != QStyle::SC_SliderGroove) {
        event->ignore();
        return;
    }

    event->accept();

    int newPos = PixelPosToRangeValue(event->pos().x());
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    switch (ActiveSlider_) {
        case ActiveSlider::INVALID: {
            Range_ += newPos - LastPos;
            Range_ += std::max(minimum() - Range_.Min, 0);
            Range_ += std::min(maximum() - Range_.Max, 0);
            break;
        }

        case ActiveSlider::LOW: {
            newPos = std::min(newPos, Range_.Max - 1);
            Range_.Min = newPos;
            break;
        }

        case ActiveSlider::HIGH: {
            newPos = std::max(newPos, Range_.Min + 1);
            Range_.Max = newPos;
            break;
        }
    }

    LastPos = newPos;
    update();

    Q_EMIT ValueChanged(Range_);
}

auto RangeSlider::mouseReleaseEvent(QMouseEvent* event) -> void {
    if (Range_ != InitialRange)
            Q_EMIT ValueChanged(Range_);

    QSlider::mouseReleaseEvent(event);
}

auto RangeSlider::PixelPosToRangeValue(int pos) -> int {
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect const grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect const handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    int const slider_length = handleRect.width();
    int const slider_min = grooveRect.x();
    int const slider_max = grooveRect.right() - slider_length + 1;

    return style()->sliderValueFromPosition(minimum(), maximum(), pos-slider_min, slider_max-slider_min);
}


LabeledRangeSlider::LabeledRangeSlider(QString const& labelText,
                                       RangeSlider::SliderSettings sliderSettings,
                                       QWidget* parent) :
        Slider(new RangeSlider(sliderSettings)),
        LowLabel(new QLabel("")),
        HighLabel(new QLabel("")),
        TextLabel(new QLabel(labelText)) {

    auto* labelHLayout = new QHBoxLayout();
    labelHLayout->setSpacing(5);
    labelHLayout->addWidget(LowLabel);
    labelHLayout->addStretch();
    labelHLayout->addWidget(TextLabel);
    labelHLayout->addStretch();
    labelHLayout->addWidget(HighLabel);

    auto* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(Slider);
    vLayout->addLayout(labelHLayout);

    connect(Slider, &RangeSlider::ValueChanged, this, [this](RangeSlider::Range range) {
        LowLabel->setText(QString::number(range.Min));
        HighLabel->setText(QString::number(range.Max));

        Q_EMIT ValueChanged(range);
    });

    Slider->SetValue(Slider->GetValue());
}

auto LabeledRangeSlider::GetValue() const noexcept -> RangeSlider::Range { return Slider->GetValue(); }

auto LabeledRangeSlider::SetValue(RangeSlider::Range range) -> void { Slider->SetValue(range); }
