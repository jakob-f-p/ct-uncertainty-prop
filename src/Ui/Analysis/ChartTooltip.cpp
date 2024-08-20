#include "ChartTooltip.h"

#include <QChart>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>


ChartTooltip::ChartTooltip(QChart& parentChart,
                           QPointF anchorPoint,
                           QString text,
                           PenBrushPair const& foregroundBackgroundColors,
                           bool isChartPositionAnchor) :
        QGraphicsItem(&parentChart),
        Chart(&parentChart),
        Anchor(anchorPoint),
        IsChartPositionAnchor(isChartPositionAnchor),
        Text(std::move(text)),
        Font(QFontDatabase::systemFont(QFontDatabase::FixedFont)),
        TextPen(foregroundBackgroundColors.Pen),
        BackgroundBrush(foregroundBackgroundColors.Brush) {

    QFontMetrics const metrics { Font };
    TextRectangle = metrics.boundingRect(QRect(0, 0, 150, 150), Qt::AlignLeft, Text);
    TextRectangle.translate(5, 5);
    Rectangle = TextRectangle.adjusted(-5, -5, 5, 5);

    setZValue(11);

    AdjustPosition();
}

auto ChartTooltip::boundingRect() const -> QRectF {
    QPointF const chartPositionAnchor = IsChartPositionAnchor
                                                ? Anchor
                                                : Chart->mapToPosition(Anchor);
    QPointF const anchor = mapFromParent(chartPositionAnchor);

    return QRectF { QPointF { qMin(Rectangle.left(), anchor.x()), qMin(Rectangle.top(), anchor.y()) },
                    QPointF { qMax(Rectangle.right(), anchor.x()), qMax(Rectangle.bottom(), anchor.y()) } };
}

void ChartTooltip::paint(QPainter* painter, QStyleOptionGraphicsItem const* /*option*/, QWidget* /*widget*/) {
    QPainterPath const tooltipPath = GetTooltipPath();

    painter->setPen(TextPen);
    painter->setFont(Font);

    painter->setBrush(BackgroundBrush);
    painter->drawPath(tooltipPath);

    painter->drawText(TextRectangle, Text);
}

void ChartTooltip::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    event->ignore();
}

auto ChartTooltip::GetTooltipPath() const -> QPainterPath {
    QPainterPath path;
    path.addRoundedRect(Rectangle, 5, 5);

    QPointF const chartPositionAnchor = IsChartPositionAnchor
                                        ? Anchor
                                        : Chart->mapToPosition(Anchor);
    QPointF const anchor = mapFromParent(chartPositionAnchor);
    if (!Rectangle.contains(anchor) && !chartPositionAnchor.isNull()) {
        // establish the position of the anchor point in relation to rectangle
        bool const above = anchor.y() <= Rectangle.top();
        bool const aboveCenter = anchor.y() > Rectangle.top() && anchor.y() <= Rectangle.center().y();
        bool const belowCenter = anchor.y() > Rectangle.center().y() && anchor.y() <= Rectangle.bottom();
        bool const below = anchor.y() > Rectangle.bottom();

        bool const onLeft = anchor.x() <= Rectangle.left();
        bool const leftOfCenter = anchor.x() > Rectangle.left() && anchor.x() <= Rectangle.center().x();
        bool const rightOfCenter = anchor.x() > Rectangle.center().x() && anchor.x() <= Rectangle.right();
        bool const onRight = anchor.x() > Rectangle.right();

        // get the nearest rectangle corner
        qreal const x = onRight || rightOfCenter
                        ? Rectangle.width()
                        : 0;
        qreal const y = below || belowCenter
                        ? Rectangle.height()
                        : 0;
        bool const cornerCase = (above && onLeft) || (above && onRight) || (below && onLeft) || (below && onRight);
        bool const vertical = qAbs(anchor.x() - x) > qAbs(anchor.y() - y);

        qreal const x1 = x + leftOfCenter * 10 - rightOfCenter * 20 + cornerCase * !vertical * (onLeft * 10 - onRight * 20);
        qreal const y1 = y + aboveCenter * 10 - belowCenter * 20 + cornerCase * vertical * (above * 10 - below * 20);
        QPointF const point1 { x1, y1 };

        qreal const x2 = x + leftOfCenter * 20 - rightOfCenter * 10 + cornerCase * !vertical * (onLeft * 20 - onRight * 10);
        qreal const y2 = y + aboveCenter * 20 - belowCenter * 10 + cornerCase * vertical * (above * 20 - below * 10);
        QPointF const point2 { x2, y2 };

        path.moveTo(point1);
        path.lineTo(anchor);
        path.lineTo(point2);
        path = path.simplified();
    }

    return path;
}

auto ChartTooltip::AdjustPosition() noexcept -> void {
    prepareGeometryChange();

    QPointF const chartPositionAnchor = IsChartPositionAnchor
                                        ? Anchor
                                        : Chart->mapToPosition(Anchor);
    setPos(chartPositionAnchor + QPoint(10, -50));

    auto rect = mapRectToParent(boundingRect());
    auto parentRect = parentItem()->boundingRect();

    if (!parentRect.contains(rect)) {
        if (parentRect.top() > rect.top())
            moveBy(0, parentRect.top() - rect.top());

        if (parentRect.bottom() < rect.bottom())
            moveBy(0, parentRect.bottom() - rect.bottom());

        if (parentRect.left() > rect.left())
            moveBy(parentRect.left() - rect.left(), 0);

        if (parentRect.right() < rect.right())
            moveBy(parentRect.right() - rect.right(), 0);
    }

    auto newRect = mapRectToParent(boundingRect());
    if (!parentRect.contains(newRect))
        qWarning("Tooltip too large for chart");
}
