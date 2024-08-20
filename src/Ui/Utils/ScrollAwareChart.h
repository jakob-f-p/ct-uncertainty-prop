#pragma once

#include <QChart>


class ScrollAwareChart : public QChart {
    Q_OBJECT

public:
    using QChart::QChart;

Q_SIGNALS:
    auto
    wheelRotated(int numberOfSteps) -> void;

protected:
    auto
    wheelEvent(QGraphicsSceneWheelEvent* event) -> void override;
};
