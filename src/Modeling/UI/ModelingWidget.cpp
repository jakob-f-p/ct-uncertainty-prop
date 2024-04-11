#include "ModelingWidget.h"

#include "CtStructureTreeModel.h"
#include "CtStructureDialog.h"
#include "CtStructureDelegate.h"
#include "../BasicStructure.h"
#include "../CombinedStructure.h"
#include "ModelingRenderWidget.h"
#include "../../App.h"

#include <QItemSelectionModel>
#include <QDockWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>


ModelingWidget::ModelingWidget(QWidget* parent) :
        QMainWindow(parent),
        RenderWidget(new ModelingRenderWidget()),
        ResetCameraButton(new QPushButton("Reset Camera")),
        AddStructureButton(new QPushButton("Add Structure")),
        CombineWithStructureButton(new QPushButton("Combine With Structure")),
        RefineWithStructureButton(new QPushButton("Refine With Structure")),
        RemoveStructureButton(new QPushButton("Remove Structure")),
        CtStructureButtons { AddStructureButton,
                             CombineWithStructureButton,
                             RefineWithStructureButton,
                             RemoveStructureButton },
        TreeModel(new CtStructureTreeModel(App::GetInstance()->GetCtDataTree())),
        TreeView(new QTreeView()),
        SelectionModel(nullptr),
        CtStructureCreateDialog(nullptr) {

    setCentralWidget(RenderWidget);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);

    auto* dockWidgetContent = new QWidget();
    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);

    auto* renderingButtonBarWidget = new QWidget();
    auto* renderingButtonBarHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingButtonBarHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingButtonBarHorizontalLayout->addWidget(ResetCameraButton);
    renderingButtonBarHorizontalLayout->addStretch();
    verticalLayout->addWidget(renderingButtonBarWidget);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    auto* treeButtonBarWidget = new QWidget();
    auto* treeButtonBarHorizontalLayout = new QHBoxLayout(treeButtonBarWidget);
    treeButtonBarHorizontalLayout->setContentsMargins(0, 11, 0, 0);
    treeButtonBarHorizontalLayout->addWidget(AddStructureButton);
    treeButtonBarHorizontalLayout->addWidget(CombineWithStructureButton);
    treeButtonBarHorizontalLayout->addWidget(RefineWithStructureButton);
    treeButtonBarHorizontalLayout->addStretch();
    treeButtonBarHorizontalLayout->addWidget(RemoveStructureButton);
    DisableButtons();
    verticalLayout->addWidget(treeButtonBarWidget);

    auto* treeDelegate = new CtStructureDelegate();
    TreeView->setModel(TreeModel);
    TreeView->setItemDelegate(treeDelegate);
    SelectionModel = TreeView->selectionModel();
    verticalLayout->addWidget(TreeView);

    ConnectButtons();

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

void ModelingWidget::ConnectButtons() {
    connect(ResetCameraButton, &QPushButton::clicked, [&]() { RenderWidget->ResetCamera(); });

    connect(AddStructureButton, &QPushButton::clicked, [&]() {
        CtStructureCreateDialog = new BasicStructureDialog(CtStructureDialog::CREATE, this);
        CtStructureCreateDialog->show();

        connect(CtStructureCreateDialog, &CtStructureDialog::accepted, [&]() {
            BasicStructureData dialogData = BasicStructureUi::GetWidgetData(CtStructureCreateDialog);
            QModelIndex siblingIndex = SelectionModel->currentIndex();
            QModelIndex newIndex = TreeModel->AddBasicStructure(dialogData, siblingIndex);
            SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
        });
    });

    connect(CombineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenBasicAndCombinedStructureCreateDialog([&](const BasicStructureData& basicStructureData,
                                                      const CombinedStructureData& combinedStructureData) {
            TreeModel->CombineWithBasicStructure(basicStructureData, combinedStructureData);
        });
    });

    connect(RefineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenBasicAndCombinedStructureCreateDialog([&](const BasicStructureData& basicStructureData,
                                                      const CombinedStructureData& combinedStructureData) {
            QModelIndex index = SelectionModel->currentIndex();
            TreeModel->RefineWithBasicStructure(basicStructureData, combinedStructureData, index);
        });
    });

    connect(RemoveStructureButton, &QPushButton::clicked, [&]() {
        QModelIndex structureIndex = SelectionModel->currentIndex();
        TreeModel->RemoveBasicStructure(structureIndex);
        TreeView->expandAll();
    });

    connect(SelectionModel, &QItemSelectionModel::currentChanged, [&](const QModelIndex& current) {
        bool isBasicStructure = current.isValid() && static_cast<CtStructure*>(current.internalPointer())->IsBasic();
        AddStructureButton->setEnabled(isBasicStructure || !TreeModel->HasRoot());
        CombineWithStructureButton->setEnabled(!current.parent().isValid());
        RefineWithStructureButton->setEnabled(isBasicStructure);
        RemoveStructureButton->setEnabled(isBasicStructure);
    });

    connect(TreeModel, &QAbstractItemModel::modelReset, [&]() {
        DisableButtons();
        SelectionModel->clear();
    });
}

void ModelingWidget::DisableButtons() {
    for (const auto& button: CtStructureButtons)
        button->setEnabled(false);
}

void ModelingWidget::OpenBasicAndCombinedStructureCreateDialog(
        const std::function<const void(const BasicStructureData&, const CombinedStructureData&)>& onAccepted) {
    auto* dialog = new BasicAndCombinedStructureCreateDialog(this);
    CtStructureCreateDialog = dialog;
    CtStructureCreateDialog->show();

    connect(CtStructureCreateDialog, &CtStructureDialog::accepted, [&, dialog, onAccepted]() {
        BasicStructureData basicStructureData = dialog->GetBasicStructureData();
        CombinedStructureData combinedStructureData = dialog->GetCombinedStructureData();

        onAccepted(basicStructureData, combinedStructureData);

        TreeView->expandAll();
    });
}
