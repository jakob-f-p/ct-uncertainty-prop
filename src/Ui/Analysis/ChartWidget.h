#pragma once

#include "Types.h"
#include "../../PipelineGroups/Types.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QWidget>

struct PipelineBatchListData;

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

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void override;

private:
    PipelineBatchListData const* BatchListData = nullptr;
    QComboBox* SelectPipelineGroupBox;
};

class TsneChartWidget : public ChartWidget {
    Q_OBJECT

public:
    explicit TsneChartWidget();
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
    CreateScatterSeries() noexcept -> std::vector<QScatterSeries*> = 0;

private Q_SLOTS:
    void ToggleTooltip(QPointF const& point, bool entered);

private:
    auto
    ConnectScatterSeries(std::vector<QScatterSeries*> const& scatterSeriesVector) noexcept -> void;

    [[nodiscard]] static auto
    GetAxisRange(QValueAxis* axis) -> std::pair<double, double>;

    [[nodiscard]] static auto
    GetAxisTickInterval(QValueAxis* axis) -> double;

    [[nodiscard]] auto
    GetCurrentForegroundBackground() const -> PenBrushPair;


protected:
    PipelineBatchListData const* BatchListData;

private:
    QString ChartName;
    QGraphicsScene* GraphicsScene;
    QChart* Chart;
    ChartTooltip* Tooltip;
    Theme CurrentTheme;
};

class PcaChartView : public ChartView {
    Q_OBJECT

public:
    explicit PcaChartView();

    auto
    SetBatchData(PipelineBatchData const* batchData) noexcept -> void;

protected:
    [[nodiscard]] auto
    CreateScatterSeries() noexcept -> std::vector<QScatterSeries*> override;

private:
    PipelineBatchData const* BatchData = nullptr;
};


class TsneChartView : public ChartView {
    Q_OBJECT

public:
    explicit TsneChartView();

protected:
    [[nodiscard]] auto
    CreateScatterSeries() noexcept -> std::vector<QScatterSeries*> override;
};
