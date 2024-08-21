#pragma once

#include "../Utils/OptionalWidget.h"

#include <QGraphicsView>
#include <QSplitter>

#include <span>


class ChartTooltip;
class ScrollAwareChart;
class PcaExplainedVariancePieChartView;
class PcaFeaturesChartView;

struct PipelineBatchListData;

class QChart;
class QChartView;
class QPieSlice;


class PcaSecondaryChartWidget : public QSplitter {
    Q_OBJECT

public:
    PcaSecondaryChartWidget();

public Q_SLOTS:
    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

protected:
    auto
    showEvent(QShowEvent* event) -> void override;

private:
    PipelineBatchListData const* BatchData;

    PcaExplainedVariancePieChartView* ExplainedVarianceChartView;
    OptionalWidget<PcaFeaturesChartView>* PrincipalAxesChartView;
};



class PcaExplainedVariancePieChartView : public QGraphicsView {
    Q_OBJECT

public:
    PcaExplainedVariancePieChartView();
    ~PcaExplainedVariancePieChartView() override;

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void;

Q_SIGNALS:
    auto
    PrincipalComponentSelected(QString const& pcName, size_t pcIdx) -> void;

protected:
    auto
    resizeEvent(QResizeEvent *event) -> void override;

private Q_SLOTS:
    void ToggleTooltip(QPieSlice* slice, bool entered);

private:
    QGraphicsScene* GraphicsScene;
    QChart* Chart;
    ChartTooltip* Tooltip;
};



class PcaFeaturesChartView : public QGraphicsView {
    Q_OBJECT

public:
    PcaFeaturesChartView();
    ~PcaFeaturesChartView() override;

    auto
    UpdateData(PipelineBatchListData const* batchListData, QString const& pcName, size_t pcIdx) -> void;

protected:
    auto
    resizeEvent(QResizeEvent* event) -> void override;

private Q_SLOTS:
    auto
    ToggleTooltip(bool entered, int barIdx) -> void;

    auto
    MoveRange(int numberOfSteps) -> void;

private:
    struct Feature {
        double Coefficient = 0.0;
        std::reference_wrapper<std::string const> Name;
    };

    auto
    UpdateChart(std::span<Feature> visibleFeatures) -> void;

    auto
    ResizeChart(QSize size) -> void;

    QGraphicsScene* GraphicsScene;
    ScrollAwareChart* Chart;
    ChartTooltip* Tooltip;

    std::vector<Feature> Features;
    QString PcName;
};
