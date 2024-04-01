#include "MainWindow.h"

#include "Modeling/UI/ModelingWidget.h"
#include "Artifacts/UI/ArtifactsWidget.h"

#include <QStatusBar>

MainWindow::MainWindow() {
    resize(1400, 700);

    setWindowTitle("CT Uncertainty Propagation");

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    auto* modelingWidget = new ModelingWidget();
    auto* artifactsWidget = new ArtifactsWidget();

    tabWidget->addTab(modelingWidget, "Implicit Data Modeling");
    tabWidget->addTab(artifactsWidget, "Artifacts Widget");

    tabWidget->setCurrentIndex(1);

    setCentralWidget(tabWidget);
}
