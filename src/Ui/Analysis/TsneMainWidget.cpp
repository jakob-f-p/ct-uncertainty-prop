#include "TsneMainWidget.h"

#include "TsneChartWidget.h"
#include "../../Modeling/CtDataSource.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QSplitter>
#include <QPushButton>

#include <vtkImageData.h>


TsneMainWidget::TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        GroupList(pipelineGroups),
        BatchData(std::make_unique<std::optional<PipelineBatchListData>>(pipelineGroups.GetBatchData())),
        ChartWidget(new OptionalWidget<TsneChartWidget>("Please generate the data first", new TsneChartWidget())),
        RenderWidget(new ParameterSpaceStateRenderWidget(dataSource)),
        DataWidget(new OptionalWidget<TsneDataWidget>("Please select a sample point", new TsneDataWidget())) {

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setWidget(DataWidget);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);

    auto* splitter = new QSplitter();
    splitter->addWidget(ChartWidget);
    splitter->addWidget(RenderWidget);
    splitter->setSizes({ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() });
    setCentralWidget(splitter);

    connect(&DataWidget->Widget(), &TsneDataWidget::RequestResetCamera,
            RenderWidget, &ParameterSpaceStateRenderWidget::ResetCamera);

    UpdateData();
}

auto TsneMainWidget::UpdateData() -> void {
    BatchData = std::make_unique<std::optional<PipelineBatchListData>>(GroupList.GetBatchData());

    PipelineBatchListData const* data = *BatchData
            ? &**BatchData
            : nullptr;
    RenderWidget->UpdateData(data);
    ChartWidget->Widget().UpdateData(data);
    DataWidget->Widget().UpdateData(data);

    if (data)
        ChartWidget->ShowWidget();
    else
        ChartWidget->HideWidget();
}

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

        UpdateImageAlgorithm(*parameterSpaceStateData.ImageData);
    }

    Render();
}

TsneDataWidget::TsneDataWidget() :
        ResetCameraButton(new QPushButton("Reset Camera")),
        BatchListData(nullptr) {

    auto* vLayout = new QVBoxLayout(this);

    auto* renderingButtonBarWidget = new QWidget();
    auto* renderingHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingHorizontalLayout->addWidget(ResetCameraButton);
    renderingHorizontalLayout->addStretch();
    vLayout->addWidget(renderingButtonBarWidget);

    connect(ResetCameraButton, &QPushButton::clicked, this, &TsneDataWidget::RequestResetCamera);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vLayout->addWidget(line);
}

auto TsneDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto TsneDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {

}
