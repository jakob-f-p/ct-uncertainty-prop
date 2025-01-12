#include "ChartLassoSelectionTool.h"

#include <QChart>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QScatterSeries>


ChartLassoSelectionTool::ChartLassoSelectionTool(QChart& parentChart, PenBrushPair const& foregroundBackgroundColors) :
        QGraphicsObject(&parentChart),
        Chart(&parentChart),
        TextPen(foregroundBackgroundColors.Pen) {

    setPos(Chart->boundingRect().topLeft());
}

auto ChartLassoSelectionTool::boundingRect() const -> QRectF {
    return Chart->boundingRect();
}

void ChartLassoSelectionTool::paint(QPainter* painter,
                                    QStyleOptionGraphicsItem const* /*option*/,
                                    QWidget* /*widget*/) {
    painter->setPen(TextPen);

    painter->drawPolyline(Lasso);
}

void ChartLassoSelectionTool::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    event->accept();

    SelectionIsInProgress = true;

    Lasso.append(event->pos());
}

auto ChartLassoSelectionTool::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*event*/) -> void {
    if (Lasso.size() > 2) {
        auto const lassoRect = Lasso.boundingRect();
        float const circumference = 2.0F * (lassoRect.width() + lassoRect.height());
        float const closing_tolerance = circumference / 10.0F;

        auto const& firstPoint = Lasso.first();
        auto const& lastPoint = Lasso.last();
        auto const distanceVector = firstPoint - lastPoint;

        if (float const distance = std::sqrt(QPointF::dotProduct(distanceVector, distanceVector));
            distance < closing_tolerance) {
            std::vector<QScatterSeries const*> scatterSeries { static_cast<size_t>(Chart->series().size()) };
            std::transform(Chart->series().cbegin(), Chart->series().cend(), scatterSeries.begin(),
                           [](QAbstractSeries const* series) { return dynamic_cast<QScatterSeries const*>(series); });

            QList<QPointF> selectedPoints;
            for (auto const* series : scatterSeries)
                std::copy_if(series->points().cbegin(), series->points().cend(), std::back_inserter(selectedPoints),
                             [this](QPointF const& point) {
                    QPointF const chartPoint = Chart->mapToPosition(point);
                    QPointF const scenePoint = Chart->mapToScene(chartPoint);
                    QPointF const localPoint = mapFromScene(scenePoint);
                    return Lasso.containsPoint(localPoint, Qt::FillRule::OddEvenFill);
                });

            if (!selectedPoints.isEmpty()) {
                Q_EMIT PointsSelected(selectedPoints);
            }
        }
    }

    SelectionIsInProgress = false;

    prepareGeometryChange();

    Lasso.clear();
}

auto ChartLassoSelectionTool::mouseMoveEvent(QGraphicsSceneMouseEvent* event) -> void {
    Lasso.append(event->pos());

    AdjustPosition();
}

auto ChartLassoSelectionTool::AdjustPosition() noexcept -> void {
    QRectF const sceneRect = mapFromScene(scene()->sceneRect()).boundingRect();
    for (auto& point : Lasso) {
        float const clampedX = static_cast<float>(std::clamp(point.x(), sceneRect.left(), sceneRect.right()));
        float const clampedY = static_cast<float>(std::clamp(point.y(), sceneRect.top(), sceneRect.bottom()));

        point.setX(clampedX);
        point.setY(clampedY);
    }

    prepareGeometryChange();
}
