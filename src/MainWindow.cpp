#include "MainWindow.h"

#include "Modeling/UI/ModelingWidget.h"

#include <QPushButton>
#include <QStatusBar>

MainWindow::MainWindow() {
    resize(1400, 700);

    auto* statBar = statusBar();

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);

    auto* modelingWidget = new ModelingWidget();

    auto* testTab2Widget = new QPushButton();

    tabWidget->addTab(modelingWidget, "Implicit Data Modeling");
    tabWidget->addTab(testTab2Widget, "TestTab2");

    setCentralWidget(tabWidget);
}
