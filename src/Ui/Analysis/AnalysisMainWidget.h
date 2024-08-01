#pragma once

#include "../Utils/OptionalWidget.h"
#include "../Utils/RenderWidget.h"
#include "../../PipelineGroups/Types.h"

#include <QMainWindow>

#include <memory>


class AnalysisDataWidget;
class ChartWidget;
class CtDataSource;
class ParameterSpaceStateRenderWidget;
class PipelineGroupList;

struct PipelineBatchListData;


class AnalysisMainWidget : public QMainWindow {
public:
    AnalysisMainWidget(PipelineGroupList const& pipelineGroups,
                       ChartWidget* chartWidget,
                       AnalysisDataWidget* dataWidget);
    ~AnalysisMainWidget() override;

    auto
    UpdateData() -> void;

private:
    PipelineGroupList const& GroupList;
    std::unique_ptr<PipelineBatchListData> BatchData;

protected:
    OptionalWidget<ChartWidget>* ChartWidget_;

private:
    ParameterSpaceStateRenderWidget* RenderWidget;
    AnalysisDataWidget* DataWidget;
};

class PcaMainWidget : public AnalysisMainWidget {
    Q_OBJECT

public:
    PcaMainWidget(PipelineGroupList const& pipelineGroups);
    ~PcaMainWidget() override;

public Q_SLOTS:
    void SelectPcaPoints(QString const& pointSetName, QList<QPointF> const& points);
};

class TsneMainWidget : public AnalysisMainWidget {
    Q_OBJECT

public:
    TsneMainWidget(PipelineGroupList const& pipelineGroups);
    ~TsneMainWidget() override;

Q_SIGNALS:
    void PcaPointsSelected(QString const& pointSetName, QList<QPointF> const& points);
};



class ParameterSpaceStateRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit ParameterSpaceStateRenderWidget();

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

public Q_SLOTS:
    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

private:
    PipelineBatchListData const* BatchListData;
    CtDataSource* DataSource;
    vtkSmartPointer<vtkImageData> CurrentImage;
};


