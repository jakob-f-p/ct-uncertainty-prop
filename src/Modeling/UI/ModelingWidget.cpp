#include "ModelingWidget.h"

#include "CtStructureDialog.h"
#include "CtStructureDelegate.h"
#include "CtStructureTreeModel.h"
#include "ModelingRenderWidget.h"
#include "../../App.h"

#include <QDockWidget>
#include <QItemSelectionModel>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>


ModelingWidget::ModelingWidget(QWidget* parent) :
        QMainWindow(parent),
        RenderWidget(new ModelingRenderWidget(App::GetInstance()->GetCtDataTree())),
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
        CtStructureCreateDialog = new BasicStructureDialog(CtStructureDialog::DialogMode::CREATE, this);
        CtStructureCreateDialog->show();

        connect(CtStructureCreateDialog, &CtStructureDialog::accepted, [&]() {
            const BasicStructureData dialogData = BasicStructureWidget::GetWidgetData(CtStructureCreateDialog);
            const QModelIndex siblingIndex = SelectionModel->currentIndex();
            const QModelIndex newIndex = TreeModel->AddBasicStructure(dialogData, siblingIndex);
            SelectionModel->clearSelection();
            SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
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
            const QModelIndex index = SelectionModel->currentIndex();
            TreeModel->RefineWithBasicStructure(basicStructureData, combinedStructureData, index);
        });
    });

    connect(RemoveStructureButton, &QPushButton::clicked, [&]() {
        const QModelIndex structureIndex = SelectionModel->currentIndex();
        TreeModel->RemoveBasicStructure(structureIndex);
        SelectionModel->clearSelection();

        UpdateButtonStates({}, {});

        TreeView->expandAll();
    });

    connect(SelectionModel, &QItemSelectionModel::selectionChanged, this, &ModelingWidget::UpdateButtonStates);
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
        BasicStructureData basicStructureData = dialog->GetBasicWidget().GetData();
        CombinedStructureData combinedStructureData = dialog->GetCombinedWidget().GetData();

        onAccepted(basicStructureData, combinedStructureData);

        TreeView->expandAll();
    });
}

void ModelingWidget::UpdateButtonStates(const QItemSelection& selected, const QItemSelection&) {
    auto modelIndexList = selected.indexes();
    if (modelIndexList.size() > 1)
        throw std::runtime_error("Invalid selection");

    const QModelIndex selectedIndex = modelIndexList.size() == 1
                                ? selected.indexes()[0]
                                : QModelIndex();

    const bool isBasicStructure = selectedIndex.isValid()
                                  && selectedIndex.data(TreeModelRoles::IS_BASIC_STRUCTURE).toBool();
    AddStructureButton->setEnabled((isBasicStructure && selectedIndex.parent().isValid()) || !TreeModel->HasRoot());
    CombineWithStructureButton->setEnabled(selectedIndex.isValid() && !selectedIndex.parent().isValid());
    RefineWithStructureButton->setEnabled(isBasicStructure);
    RemoveStructureButton->setEnabled(isBasicStructure);
}
