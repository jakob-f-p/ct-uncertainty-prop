#pragma once

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QWidget>

#include "../../Utils/Types.h"

struct PipelineBatchListData;

class TsneChartTooltip;
class TsneChartView;

class QButtonGroup;
class QChart;
class QPushButton;
class QScatterSeries;
class QValueAxis;


class TsneChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit TsneChartWidget();

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void;

Q_SIGNALS:
    void SamplePointChanged(std::optional<SampleId> sampleId);

private:
    TsneChartView* ChartView;
    QPushButton* LightThemeButton;
    QPushButton* DarkThemeButton;
    QButtonGroup* ThemeButtonGroup;
    QPushButton* ExportButton;
};


class TsneChartView : public QGraphicsView {
    Q_OBJECT

public:
    explicit TsneChartView();

    ~TsneChartView() override;

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void;

    enum struct Theme : uint8_t {
        LIGHT = 0,
        DARK
    };

    struct PenBrushPair {
        QPen Pen;
        QBrush Brush;
    };

    auto
    UpdateTheme(Theme theme) -> void;

public Q_SLOTS:
    auto
    ExportChart() -> void;

Q_SIGNALS:
    void SamplePointChanged(std::optional<SampleId> sampleId);

protected:
    auto
    resizeEvent(QResizeEvent *event) -> void override;

private Q_SLOTS:
    void ToggleTooltip(QPointF const& point, bool entered);

private:
    [[nodiscard]] static auto
    GetAxisRange(QValueAxis* axis) -> std::pair<int, int>;

    [[nodiscard]] static auto
    GetAxisTickInterval(QValueAxis* axis) -> double;

    [[nodiscard]] auto
    CreateScatterSeries() noexcept -> std::vector<QScatterSeries*>;

    [[nodiscard]] auto
    GetCurrentForegroundBackground() const -> PenBrushPair;

    PipelineBatchListData const* BatchListData;
    QGraphicsScene* GraphicsScene;
    QChart* Chart;
    TsneChartTooltip* Tooltip;
    Theme CurrentTheme;
};


class TsneChartTooltip : public QGraphicsItem {
public:
    explicit TsneChartTooltip(QChart& parentChart,
                              QPointF anchorPoint,
                              TsneChartView::PenBrushPair const& foregroundBackgroundColors);

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

    QString Text;
    QRectF TextRectangle;
    QRectF Rectangle;
    QFont Font;
    QBrush BackgroundBrush;
    QPen TextPen;
};
