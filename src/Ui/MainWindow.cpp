#include "MainWindow.h"

#include "Modeling/ModelingWidget.h"
#include "Artifacts/ArtifactsWidget.h"
#include "Segmentation/SegmentationWidget.h"

MainWindow::MainWindow(CtStructureTree& ctStructureTree,
                       CtDataSource& dataSource,
                       PipelineList& pipelineList) :
        DataSource(dataSource) {
    resize(1400, 700);

    setWindowTitle("CT Uncertainty Propagation");

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    auto* modelingWidget = new ModelingWidget(ctStructureTree, DataSource);
    auto* artifactsWidget = new ArtifactsWidget(pipelineList, DataSource);
    auto* segmentationWidget = new SegmentationWidget(DataSource);

    tabWidget->addTab(modelingWidget, "Modeling");
    tabWidget->addTab(artifactsWidget, "Artifacts");
    tabWidget->addTab(segmentationWidget, "Segmentation");

    setCentralWidget(tabWidget);

    auto updateSegmentationDataSource = [=](int idx) {
        if (idx == tabWidget->indexOf(segmentationWidget))
            segmentationWidget->UpdateDataSource(artifactsWidget->GetCurrentFilter());
    };
    connect(tabWidget, &QTabWidget::currentChanged, this, updateSegmentationDataSource);
}
