#include <QStatusBar>
#include <QPushButton>
#include "MainWindow.h"
#include "Modeling/ModelingWidget.h"

MainWindow::MainWindow() {
    resize(1400, 700);

    auto* statBar = statusBar();

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    auto* modelingWidget = new ModelingWidget();

    auto* testTab1Widget = new QPushButton();
    auto* testTab2Widget = new QPushButton();

    tabWidget->addTab(testTab1Widget, "TestTab1");
    tabWidget->addTab(modelingWidget, "Implicit Data Modeling");
    tabWidget->addTab(testTab2Widget, "TestTab2");

    setCentralWidget(tabWidget);
}
