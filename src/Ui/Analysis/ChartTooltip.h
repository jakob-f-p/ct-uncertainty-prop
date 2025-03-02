#pragma once

#include "Types.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsItem>
#include <QPen>

class QChart;


class ChartTooltip : public QGraphicsItem {
public:
    explicit ChartTooltip(QChart& parentChart,
                          QPointF anchorPoint,
                          QString text,
                          PenBrushPair const& foregroundBackgroundColors,
                          bool isChartPositionAnchor = false);

    [[nodiscard]] auto
    boundingRect() const -> QRectF override;

    auto
    paint(QPainter* painter, QStyleOptionGraphicsItem const* option, QWidget* widget) -> void override;

protected:
    auto
    mousePressEvent(QGraphicsSceneMouseEvent* event) -> void override;

private:
    [[nodiscard]] auto
    GetTooltipPath() const -> QPainterPath;

    auto
    AdjustPosition() noexcept -> void;

    QChart* Chart;

    QPointF Anchor;
    bool IsChartPositionAnchor;

    QString Text;
    QRectF TextRectangle;
    QRectF Rectangle;
    QFont Font;
    QBrush BackgroundBrush;
    QPen TextPen;
};
