#include "../App.h"
#include "ModelingWidget.h"
#include "CtDataCsgTreeModel.h"
#include "CtStructureEditDialog.h"
#include "CtStructureDelegate.h"

#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QDockWidget.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeView>

ModelingWidget::ModelingWidget() {

    SetUpRenderingWidgetForShowingImplicitData();

    SetUpDockWidgetForImplicitCsgTreeModeling();
}

void ModelingWidget::SetUpRenderingWidgetForShowingImplicitData() {
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->SetWindowName("CT-Data");

    auto* renderingWidget = new QVTKOpenGLNativeWidget();
    renderingWidget->setRenderWindow(renderWindow);

    setCentralWidget(renderingWidget);
}

void ModelingWidget::SetUpDockWidgetForImplicitCsgTreeModeling() {
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
    auto* treeModel = new CtDataCsgTreeModel(*App::GetInstance()->GetCtDataCsgTree());
    auto* treeDelegate = new CtStructureDelegate();
    treeView->setModel(treeModel);
    treeView->setItemDelegate(treeDelegate);

//    connect(treeView, &QTreeView::doubleClicked, [treeModel, treeView](const QModelIndex& index) {
//        CtStructureEditDialog editDialog(*treeModel, index, treeView);
//        editDialog.exec();
//    });


    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);
    verticalLayout->addWidget(buttonBarWidget);
    verticalLayout->addWidget(treeView);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

//    QPalette greenPal = QPalette();
//    greenPal.setColor(QPalette::Window, Qt::green);
//    widget->setAutoFillBackground(true);
//    widget->setPalette(greenPal);

