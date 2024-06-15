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

    ~TsneChartView();

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void;

    enum struct Theme : uint8_t {
        LIGHT = 0,
        DARK
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

    PipelineBatchListData const* BatchListData;
    QGraphicsScene* GraphicsScene;
    QChart* Chart;
    TsneChartTooltip* Tooltip;
};


class TsneChartTooltip : public QGraphicsItem {
public:
    explicit TsneChartTooltip(QChart& parentChart, QString const& text, QPointF anchorPoint);

    auto boundingRect() const -> QRectF override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QChart* Chart;

    QPointF Anchor;

    QString Text;
    QRectF TextRectangle;
    QRectF Rectangle;
    QFont Font;
};
