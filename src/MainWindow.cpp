#include "MainWindow.h"

#include "Modeling/CtDataSource.h"
#include "Modeling/UI/ModelingWidget.h"
#include "Artifacts/UI/ArtifactsWidget.h"
#include "Segmentation/SegmentationWidget.h"

MainWindow::MainWindow(CtStructureTree& ctStructureTree, PipelineList& pipelineList) :
        DataSource([&]() {
            vtkNew<CtDataSource> dataSource;
            dataSource->SetDataTree(&ctStructureTree);
            return dataSource;
        }()) {
    resize(1400, 700);

    setWindowTitle("CT Uncertainty Propagation");

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    auto* modelingWidget = new ModelingWidget(ctStructureTree, *DataSource);
    auto* artifactsWidget = new ArtifactsWidget(pipelineList, *DataSource);
    auto* segmentationWidget = new SegmentationWidget(*DataSource);

    tabWidget->addTab(modelingWidget, "Modeling");
    tabWidget->addTab(artifactsWidget, "Artifacts");
    tabWidget->addTab(segmentationWidget, "Segmentation");

    tabWidget->setCurrentIndex(1);

    setCentralWidget(tabWidget);

    auto updateSegmentationDataSource = [=](int idx) {
        if (idx == tabWidget->indexOf(segmentationWidget))
            segmentationWidget->UpdateDataSource(artifactsWidget->GetCurrentFilter());
    };
    connect(tabWidget, &QTabWidget::currentChanged, this, updateSegmentationDataSource);
}
