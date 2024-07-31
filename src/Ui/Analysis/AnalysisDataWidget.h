#pragma once

#include "../Utils/OptionalWidget.h"
#include "../../PipelineGroups/Types.h"

#include <QGraphicsView>
#include <QWidget>

class AnalysisSampleDataWidget;
class ChartTooltip;
class NameLineEdit;
class PcaAnalysisDataWidget;
class PcaFeaturesChartView;
class PipelineParameterSpaceStateView;

struct ParameterSpaceStateData;
struct PipelineBatchListData;

class QChart;
class QChartView;
class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;


class AnalysisDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisDataWidget(AnalysisSampleDataWidget* sampleDataWidget);

public Q_SLOTS:
    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

protected:
    virtual auto
    UpdateDataDerived(PipelineBatchListData const* batchData) -> void;

    QVBoxLayout* VLayout;

private:
    OptionalWidget<AnalysisSampleDataWidget>* SampleDataWidget;
};

class PcaDataWidget : public AnalysisDataWidget {
    Q_OBJECT

public:
    PcaDataWidget();

protected:
    auto
    UpdateDataDerived(PipelineBatchListData const* batchData) -> void override;

private:
    OptionalWidget<PcaAnalysisDataWidget>* PcaAnalysisChartWidget;
};

class TsneDataWidget : public AnalysisDataWidget {
    Q_OBJECT

public:
    TsneDataWidget();
};



class AnalysisSampleDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisSampleDataWidget(QString const& analysisName);

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

public Q_SLOTS:
    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

protected:
    struct XYCoords { double X, Y; };

    [[nodiscard]] virtual auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords = 0;

private:
    QString AnalysisName;
    PipelineBatchListData const* BatchListData;

    NameLineEdit* PipelineGroupNameEdit;
    NameLineEdit* BasePipelineNameEdit;
    QSpinBox* NumberOfGroupPipelinesSpinBox;
    QDoubleSpinBox* PointXSpinBox;
    QDoubleSpinBox* PointYSpinBox;
    OptionalWidget<PipelineParameterSpaceStateView>* ParameterSpaceStateView;
};

class PcaSampleDataWidget : public AnalysisSampleDataWidget {
public:
    PcaSampleDataWidget();

protected:
    [[nodiscard]] auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords override;
};

class TsneSampleDataWidget : public AnalysisSampleDataWidget {
public:
    TsneSampleDataWidget();

protected:
    [[nodiscard]] auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords override;
};



class PcaAnalysisDataWidget : public QWidget {
    Q_OBJECT

public:
    PcaAnalysisDataWidget();

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

private:
    auto
    UpdateExplainedVarianceChart() -> void;

    friend class PcaFeaturesChartView;

    PipelineBatchListData const* BatchData;

    QChartView* ExplainedVarianceChartView;
    OptionalWidget<PcaFeaturesChartView>* PrincipalAxesChartView;
};

class PcaFeaturesChartView : public QGraphicsView {
    Q_OBJECT

public:
    PcaFeaturesChartView();
    ~PcaFeaturesChartView() override;

    auto
    UpdateData(PcaAnalysisDataWidget* parentWidget, int barIdx) -> void;

protected:
    auto
    resizeEvent(QResizeEvent *event) -> void override;

private Q_SLOTS:
    void ToggleTooltip(bool entered, int barIdx);

private:
    QGraphicsScene* GraphicsScene;
    QChart* Chart;
    ChartTooltip* Tooltip;
};
