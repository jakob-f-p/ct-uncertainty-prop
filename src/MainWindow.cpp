#include "MainWindow.h"

#include "Modeling/CtDataSource.h"
#include "Modeling/UI/ModelingWidget.h"
#include "Artifacts/UI/ArtifactsWidget.h"

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

    tabWidget->addTab(modelingWidget, "Implicit Modeling");
    tabWidget->addTab(artifactsWidget, "Artifacts Widget");

    tabWidget->setCurrentIndex(1);

    setCentralWidget(tabWidget);
}
