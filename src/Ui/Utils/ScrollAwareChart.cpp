#include "ScrollAwareChart.h"

#include <QGraphicsSceneWheelEvent>


template <typename T>
[[nodiscard]] auto
sgn(T val) noexcept -> int {
    return (T(0) < val) - (val < T(0));
}

auto ScrollAwareChart::wheelEvent(QGraphicsSceneWheelEvent* event) -> void {
    bool const validPhase = event->phase() == Qt::ScrollPhase::NoScrollPhase
            || event->phase() == Qt::ScrollPhase::ScrollUpdate;
    if (!validPhase || event->orientation() != Qt::Orientation::Vertical)
        return;

    if (event->delta() == 0 && event->pixelDelta().y() == 0)
        return;

    int const numberOfSteps = event->delta() != 0
                                ? event->delta() / (8 * 15)
                                : sgn(event->pixelDelta().y());

    Q_EMIT wheelRotated(numberOfSteps);
}