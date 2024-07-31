#include "AnalysisMainWidget.h"

#include "AnalysisDataWidget.h"
#include "ChartWidget.h"
#include "../../Modeling/CtDataSource.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QDockWidget>
#include <QSplitter>

#include <vtkImageData.h>


AnalysisMainWidget::AnalysisMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource,
                                       ChartWidget* chartWidget, AnalysisDataWidget* dataWidget) :
        GroupList(pipelineGroups),
        BatchData([&pipelineGroups]() {
            auto batchData = pipelineGroups.GetBatchData();
            return batchData
                    ? std::make_unique<PipelineBatchListData>(std::move(*batchData))
                    : nullptr;
        }()),
        ChartWidget_(new OptionalWidget<ChartWidget>("Please generate the data first", chartWidget)),
        RenderWidget(new ParameterSpaceStateRenderWidget(dataSource)),
        DataWidget(dataWidget) {

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setWidget(DataWidget);
    dockWidget->setWindowTitle("Data");
    auto margins = dockWidget->contentsMargins();
    margins.setTop(0);
    dockWidget->setContentsMargins(margins);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);

    auto* splitter = new QSplitter();
    splitter->addWidget(ChartWidget_);
    splitter->addWidget(RenderWidget);
    splitter->setSizes({ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() });
    setCentralWidget(splitter);

    connect(&ChartWidget_->Widget(), &ChartWidget::SamplePointChanged,
            RenderWidget, &ParameterSpaceStateRenderWidget::UpdateSample);

    connect(&ChartWidget_->Widget(), &ChartWidget::SamplePointChanged,
            DataWidget, &AnalysisDataWidget::UpdateSample);

    UpdateData();
}

AnalysisMainWidget::~AnalysisMainWidget() = default;

auto AnalysisMainWidget::UpdateData() -> void {
    auto optionalBatchData = GroupList.GetBatchData();
    BatchData = optionalBatchData
            ? std::make_unique<PipelineBatchListData>(std::move(*optionalBatchData))
            : nullptr;

    PipelineBatchListData const* data = BatchData.get();
    RenderWidget->UpdateData(data);
    ChartWidget_->Widget().UpdateData(data);
    DataWidget->UpdateData(data);

    if (data)
        ChartWidget_->ShowWidget();
    else
        ChartWidget_->HideWidget();

    Q_EMIT ChartWidget_->Widget().SamplePointChanged(std::nullopt);
}

PcaMainWidget::PcaMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        AnalysisMainWidget(pipelineGroups, dataSource, new PcaChartWidget(), new PcaDataWidget()) {}

PcaMainWidget::~PcaMainWidget() = default;

void PcaMainWidget::SelectPcaPoints(QString const& pointSetName, QList<QPointF> const& points) {
    dynamic_cast<PcaChartWidget&>(ChartWidget_->Widget()).SelectPcaPoints(pointSetName, points);
}

TsneMainWidget::TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        AnalysisMainWidget(pipelineGroups, dataSource, new TsneChartWidget(), new TsneDataWidget()) {

    connect(&dynamic_cast<TsneChartWidget&>(ChartWidget_->Widget()), &TsneChartWidget::PcaPointsSelected,
            this, &TsneMainWidget::PcaPointsSelected);
}

TsneMainWidget::~TsneMainWidget() = default;


ParameterSpaceStateRenderWidget::ParameterSpaceStateRenderWidget(CtDataSource& dataSource) :
        RenderWidget(dataSource),
        DataSource(dataSource),
        BatchListData(nullptr) {}

auto ParameterSpaceStateRenderWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto ParameterSpaceStateRenderWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        UpdateImageAlgorithm(DataSource);
    else {
        auto const& parameterSpaceStateData = BatchListData->GetSpaceStateData(*sampleId);

        CurrentImage = parameterSpaceStateData.ImageHandle.Read();

        UpdateImageAlgorithm(*CurrentImage);
    }

    Render();
}
