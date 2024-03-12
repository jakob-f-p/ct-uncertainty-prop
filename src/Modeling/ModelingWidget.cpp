#include <QDockWidget.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeView>

#include "ModelingWidget.h"

ModelingWidget::ModelingWidget() {
    auto* renderingWidget = new QVTKOpenGLNativeWidget();

    setCentralWidget(renderingWidget);

//    QPalette greenPal = QPalette();
//    greenPal.setColor(QPalette::Window, Qt::green);
//    widget->setAutoFillBackground(true);
//    widget->setPalette(greenPal);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);

    auto* dockWidgetContent = new QWidget();

    auto* buttonBarWidget = new QWidget();
    auto* horizontalLayout = new QHBoxLayout(buttonBarWidget);
    auto* addStructureButton = new QPushButton("Add Structure");
    auto* combineWithStructure = new QPushButton("Combine With Structure");
    auto* removeStructureButton = new QPushButton("Remove Structure");
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(addStructureButton);
    horizontalLayout->addWidget(combineWithStructure);
    horizontalLayout->addWidget(removeStructureButton);

    auto* treeView = new QTreeView();

    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);
    verticalLayout->addWidget(buttonBarWidget);
    verticalLayout->addWidget(treeView);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}
