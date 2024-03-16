#include "../App.h"
#include "ModelingWidget.h"
#include "CtStructureEditDialog.h"
#include "CtStructureDelegate.h"

#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QDockWidget.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>

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
    AddStructureButton = new QPushButton("Add Structure");
    CombineWithStructureButton = new QPushButton("Combine With Structure");
    RefineWithStructureButton = new QPushButton("Refine With Structure");
    RemoveStructureButton = new QPushButton("Remove Structure");
    std::array<QPushButton*, 4> buttons { AddStructureButton,
                                          CombineWithStructureButton,
                                          RefineWithStructureButton,
                                          RemoveStructureButton };
    std::for_each(buttons.begin(),
                  buttons.end(),
                  [](QPushButton* button) { button->setEnabled(false); });
    horizontalLayout->addWidget(AddStructureButton);
    horizontalLayout->addWidget(CombineWithStructureButton);
    horizontalLayout->addWidget(RefineWithStructureButton);
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(RemoveStructureButton);

    TreeView = new QTreeView();
    TreeModel = new CtDataCsgTreeModel(*App::GetInstance()->GetCtDataCsgTree());
    auto* treeDelegate = new CtStructureDelegate();
    TreeView->setModel(TreeModel);
    TreeView->setItemDelegate(treeDelegate);

    SelectionModel = TreeView->selectionModel();
    connect(SelectionModel, &QItemSelectionModel::currentChanged,
            [&](const QModelIndex& current) {
        if (!current.isValid()) {
            AddStructureButton->setEnabled(!TreeModel->HasRoot());
            CombineWithStructureButton->setEnabled(false);
            RefineWithStructureButton->setEnabled(false);
            RemoveStructureButton->setEnabled(false);
            return;
        }

        bool isImplicitCtStructure = current.data(Qt::UserRole).toBool();

        AddStructureButton->setEnabled(isImplicitCtStructure || !TreeModel->hasIndex(0, 0));
        CombineWithStructureButton->setEnabled(!current.parent().isValid());
        RefineWithStructureButton->setEnabled(static_cast<CtStructure*>(current.internalPointer())->IsImplicitCtStructure());
        RemoveStructureButton->setEnabled(isImplicitCtStructure);
    });

    connect(AddStructureButton, &QPushButton::clicked, [&]() {
        CtStructureCreateDialog = new CtStructureEditDialog(this, true);
        CtStructureCreateDialog->HideImplicitStructureCombinationSection();
        CtStructureCreateDialog->setModal(true);
        CtStructureCreateDialog->show();

        connect(CtStructureCreateDialog, &CtStructureEditDialog::accepted, [&]() {
            ImplicitCtStructureDetails dialogData = CtStructureCreateDialog->GetImplicitCtStructureData();
            QModelIndex siblingIndex = SelectionModel->currentIndex();
            QModelIndex newIndex = TreeModel->AddImplicitCtStructure(dialogData, siblingIndex);
            SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
        });
    });

    connect(CombineWithStructureButton, &QPushButton::clicked, [&]() {
        CtStructureCreateDialog = new CtStructureEditDialog(this, true);
        CtStructureCreateDialog->HideImplicitStructureCombinationSection();
        CtStructureCreateDialog->setModal(true);
        CtStructureCreateDialog->show();

        connect(CtStructureCreateDialog, &CtStructureEditDialog::accepted, [&]() {
            ImplicitCtStructureDetails dialogData = CtStructureCreateDialog->GetImplicitCtStructureData();
            TreeModel->CombineWithImplicitCtStructure(dialogData);
            SelectionModel->clearCurrentIndex();
            TreeView->expandAll();
        });
    });

    connect(RefineWithStructureButton, &QPushButton::clicked, [&]() {
        CtStructureCreateDialog = new CtStructureEditDialog(this, true);
        CtStructureCreateDialog->HideImplicitStructureCombinationSection();
        CtStructureCreateDialog->setModal(true);
        CtStructureCreateDialog->show();

        connect(CtStructureCreateDialog, &CtStructureEditDialog::accepted, [&]() {
            ImplicitCtStructureDetails dialogData = CtStructureCreateDialog->GetImplicitCtStructureData();
            QModelIndex index = SelectionModel->currentIndex();
            TreeModel->RefineWithImplicitStructure(dialogData, index);
            SelectionModel->clearCurrentIndex();
            TreeView->expandAll();
        });
    });

    connect(RemoveStructureButton, &QPushButton::clicked, [&]() {
        QModelIndex structureIndex = SelectionModel->currentIndex();
        TreeModel->RemoveImplicitCtStructure(structureIndex);
        SelectionModel->clearCurrentIndex();
        SelectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Select);
        emit SelectionModel->currentChanged(QModelIndex(), QModelIndex());
        TreeView->expandAll();
    });

    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);
    verticalLayout->addWidget(buttonBarWidget);
    verticalLayout->addWidget(TreeView);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}
