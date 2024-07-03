#pragma once

#include "Types.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsObject>
#include <QPen>

class QChart;


class ChartLassoSelectionTool : public QGraphicsObject {
    Q_OBJECT

public:
    explicit ChartLassoSelectionTool(QChart& parentChart, PenBrushPair const& foregroundBackgroundColors);

    [[nodiscard]] auto
    boundingRect() const -> QRectF override;

    auto
    paint(QPainter* painter, QStyleOptionGraphicsItem const* option, QWidget* widget) -> void override;

Q_SIGNALS:
    void PointsSelected(QList<QPointF> const& points);

protected:
    auto
    mousePressEvent(QGraphicsSceneMouseEvent* event) -> void override;

    auto
    mouseReleaseEvent(QGraphicsSceneMouseEvent* event) -> void override;

    auto
    mouseMoveEvent(QGraphicsSceneMouseEvent* event) -> void override;

private:
    auto
    AdjustPosition() noexcept -> void;

    QChart* Chart;

    QPolygonF Lasso;
    bool SelectionIsInProgress = false;

    QPen TextPen;
};
