#include "MainWindow.h"

#include "Analysis/AnalysisWidget.h"
#include "Artifacts/ArtifactsWidget.h"
#include "Modeling/ModelingWidget.h"
#include "PipelineGroups/PipelineGroupsWidget.h"
#include "Segmentation/SegmentationWidget.h"

MainWindow::MainWindow(CtStructureTree& ctStructureTree,
                       CtDataSource& dataSource,
                       ThresholdFilter& thresholdFilter,
                       PipelineList& pipelineList,
                       PipelineGroupList& pipelineGroups) :
        DataSource(dataSource) {
    resize(1400, 700);

    setWindowTitle("CT Uncertainty Propagation");

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    auto* modelingWidget = new ModelingWidget(ctStructureTree, DataSource);
    auto* artifactsWidget = new ArtifactsWidget(pipelineList);
    auto* segmentationWidget = new SegmentationWidget(DataSource, thresholdFilter);
    auto* pipelineGroupsWidget = new PipelineGroupsWidget(pipelineGroups);
    auto* analysisWidget = new AnalysisWidget(pipelineGroups, DataSource);

    tabWidget->addTab(modelingWidget, "Modeling");
    tabWidget->addTab(artifactsWidget, "Artifacts");
    tabWidget->addTab(segmentationWidget, "Segmentation");
    tabWidget->addTab(pipelineGroupsWidget, "Pipeline Groups");
    tabWidget->addTab(analysisWidget, "Analysis");

    setCentralWidget(tabWidget);

    auto updateSegmentationDataSource = [=](int idx) {
        if (idx == tabWidget->indexOf(segmentationWidget))
            segmentationWidget->UpdateDataSource(artifactsWidget->GetCurrentFilter());
    };
    connect(tabWidget, &QTabWidget::currentChanged, this, updateSegmentationDataSource);

    auto updatePipelineGroupsPipelines = [=](int idx) {
        if (idx == tabWidget->indexOf(pipelineGroupsWidget))
            pipelineGroupsWidget->UpdatePipelineList();
    };
    connect(tabWidget, &QTabWidget::currentChanged, this, updatePipelineGroupsPipelines);

    auto updateAnalysisWidgetData = [=](int idx) {
        if (idx == tabWidget->indexOf(analysisWidget))
            analysisWidget->UpdateData();
    };
    connect(tabWidget, &QTabWidget::currentChanged, this, updateAnalysisWidgetData);
}
