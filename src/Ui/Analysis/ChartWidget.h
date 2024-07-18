#pragma once

#include "Types.h"
#include "../../PipelineGroups/Types.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QWidget>

struct PipelineBatchListData;

class ChartLassoSelectionTool;
class ChartTooltip;
class ChartView;
class PipelineBatchData;

class QComboBox;
class QButtonGroup;
class QChart;
class QPushButton;
class QScatterSeries;
class QValueAxis;
class QVBoxLayout;


class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(ChartView* chartView);

    virtual auto
    UpdateData(PipelineBatchListData const* batchListData) -> void;

Q_SIGNALS:
    void SamplePointChanged(std::optional<SampleId> sampleId);

protected:
    QVBoxLayout* VLayout;
    ChartView* View;

private:
    QPushButton* LightThemeButton;
    QPushButton* DarkThemeButton;
    QButtonGroup* ThemeButtonGroup;
    QPushButton* ExportButton;
};

class PcaChartWidget : public ChartWidget {
    Q_OBJECT

public:
    explicit PcaChartWidget();
    ~PcaChartWidget() override;

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void override;

    auto
    SelectPcaPoints(QString const& name, QList<QPointF> const& points) -> void;

private:
    PipelineBatchListData const* BatchListData = nullptr;
    QComboBox* SelectPointSetComboBox;
    std::map<QString, std::unique_ptr<PipelineBatchListData>> PcaSelectionMap;
};

class TsneChartWidget : public ChartWidget {
    Q_OBJECT

public:
    explicit TsneChartWidget();

Q_SIGNALS:
    void PcaPointsSelected(QString const& name, QList<QPointF> const& points);

private Q_SLOTS:
    void OnPointsSelected(QList<QPointF> const& points);
};



class ChartView : public QGraphicsView {
    Q_OBJECT

public:
    explicit ChartView(QString const& chartName);
    ~ChartView() override;

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

    [[nodiscard]] virtual auto
    CreateScatterSeries() noexcept -> std::map<uint16_t, QScatterSeries*> = 0;

    [[nodiscard]] auto
    GetCurrentForegroundBackground() const -> PenBrushPair;

    virtual auto
    RemoveItemsFromSceneOnUpdateData() -> void;

    virtual auto
    AddItemsFromSceneOnUpdateData() -> void;

private Q_SLOTS:
    void ToggleTooltip(QPointF const& point, bool entered);

private:
    auto
    ConnectScatterSeries(std::map<uint16_t, QScatterSeries*> const& indexScatterSeriesMap) noexcept -> void;

    [[nodiscard]] static auto
    GetAxisRange(QValueAxis* axis) -> std::pair<double, double>;

    [[nodiscard]] static auto
    GetAxisTickInterval(QValueAxis* axis) -> double;

protected:
    PipelineBatchListData const* BatchListData;
    QChart* Chart;
    QGraphicsScene* GraphicsScene;

private:
    QString ChartName;
    ChartTooltip* Tooltip;
    Theme CurrentTheme;
};

class PcaChartView : public ChartView {
    Q_OBJECT

public:
    explicit PcaChartView();

    auto
    SetBatchListData(PipelineBatchListData const& batchListData) noexcept -> void;

protected:
    [[nodiscard]] auto
    CreateScatterSeries() noexcept -> std::map<uint16_t, QScatterSeries*> override;

private:
    PipelineBatchListData const* PcaBatchListData;
};


class TsneChartView : public ChartView {
    Q_OBJECT

public:
    explicit TsneChartView();

Q_SIGNALS:
    void PointsSelected(QList<QPointF> const& points);

protected:
    [[nodiscard]] auto
    CreateScatterSeries() noexcept -> std::map<uint16_t, QScatterSeries*> override;

    auto
    AddItemsFromSceneOnUpdateData() -> void override;

private:
    ChartTooltip* Tooltip;
    ChartLassoSelectionTool* LassoTool;
};
