#include "../App.h"
#include "ModelingWidget.h"
#include "CtStructureEditDialog.h"
#include "CtStructureDelegate.h"

#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <QDockWidget.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>

ModelingWidget::ModelingWidget() :
        AddStructureButton(nullptr),
        CombineWithStructureButton(nullptr),
        RefineWithStructureButton(nullptr),
        RemoveStructureButton(nullptr),
        TreeModel(nullptr),
        TreeView(nullptr),
        SelectionModel(nullptr),
        CtStructureCreateDialog(nullptr),
        DataSource(nullptr),
        DataTree(App::GetInstance()->GetCtDataCsgTree()) {

    SetUpRenderingWidgetForShowingImplicitData();

    SetUpDockWidgetForImplicitCsgTreeModeling();
}

void ModelingWidget::OpenDialog(const std::function<const void()>& onAccepted) {
    CtStructureCreateDialog = new CtStructureEditDialog(this, true);
    CtStructureCreateDialog->HideImplicitStructureCombinationSection();
    CtStructureCreateDialog->setModal(true);
    CtStructureCreateDialog->show();

    connect(CtStructureCreateDialog, &CtStructureEditDialog::accepted, onAccepted);
}

void ModelingWidget::SetUpRenderingWidgetForShowingImplicitData() {
    DataSource = CtDataSource::New();
    DataSource->SetDataTree(DataTree);
    vtkNew<vtkPiecewiseFunction> opacityMappingFunction;
    opacityMappingFunction->AddPoint(-1000.0, 0.005);
    opacityMappingFunction->AddPoint(2000.0, 0.05);

    vtkNew<vtkColorTransferFunction> colorTransferFunction;
    colorTransferFunction->AddRGBPoint(-1000.0, 0.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(2000.0, 3 * 1.0, 3 * 1.0, 3 * 1.0);

    vtkNew<vtkVolumeProperty> volumeProperty;
    volumeProperty->SetColor(colorTransferFunction.GetPointer());
    volumeProperty->SetScalarOpacity(opacityMappingFunction.GetPointer());
    volumeProperty->ShadeOn();
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->SetAmbient(0.3);

    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
    volumeMapper->SetInputConnection(DataSource->GetOutputPort());

    vtkNew<vtkVolume> volume;
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    vtkNew<vtkRenderer> volumeRenderer;
    volumeRenderer->AddVolume(volume);
    volumeRenderer->SetBackground(0.1, 0.1, 0.1);

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->SetWindowName("CT-Data");
    renderWindow->AddRenderer(volumeRenderer);

    auto* renderingWidget = new QVTKOpenGLNativeWidget();
    renderingWidget->setRenderWindow(renderWindow);

    renderWindow->Render();

    setCentralWidget(renderingWidget);

    vtkNew<vtkCallbackCommand> onDataChangedUpdater;
    onDataChangedUpdater->SetClientData(renderWindow);
    onDataChangedUpdater->SetCallback([](vtkObject*, unsigned long, void* renderW, void*) {
        auto* renderWin = static_cast<vtkGenericOpenGLRenderWindow*>(renderW);
        renderWin->Render();
    });
    DataTree->AddObserver(vtkCommand::ModifiedEvent, onDataChangedUpdater);
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
    DisableButtons();
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

    ConnectButtons();

    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);
    verticalLayout->addWidget(buttonBarWidget);
    verticalLayout->addWidget(TreeView);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

void ModelingWidget::ConnectButtons() {
    connect(AddStructureButton, &QPushButton::clicked, [&]() {
        OpenDialog([&]() {
            ImplicitCtStructureDetails dialogData = CtStructureCreateDialog->GetImplicitCtStructureData();
            QModelIndex siblingIndex = SelectionModel->currentIndex();
            QModelIndex newIndex = TreeModel->AddImplicitCtStructure(dialogData, siblingIndex);
            SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
        });
    });

    connect(CombineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenDialog([&]() {
            ImplicitCtStructureDetails dialogData = CtStructureCreateDialog->GetImplicitCtStructureData();
            TreeModel->CombineWithImplicitCtStructure(dialogData);
            TreeView->expandAll();
        });
    });

    connect(RefineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenDialog([&]() {
            ImplicitCtStructureDetails dialogData = CtStructureCreateDialog->GetImplicitCtStructureData();
            QModelIndex index = SelectionModel->currentIndex();
            TreeModel->RefineWithImplicitStructure(dialogData, index);
            TreeView->expandAll();
        });
    });

    connect(RemoveStructureButton, &QPushButton::clicked, [&]() {
        QModelIndex structureIndex = SelectionModel->currentIndex();
        TreeModel->RemoveImplicitCtStructure(structureIndex);
        TreeView->expandAll();
    });

    connect(SelectionModel, &QItemSelectionModel::currentChanged, [&](const QModelIndex& current) {
        bool isImplicitCtStructure = current.data(Qt::UserRole).toBool();
        AddStructureButton->setEnabled(isImplicitCtStructure || !TreeModel->HasRoot());
        CombineWithStructureButton->setEnabled(!current.parent().isValid());
        RefineWithStructureButton->setEnabled(isImplicitCtStructure);
        RemoveStructureButton->setEnabled(isImplicitCtStructure);
    });

    connect(TreeModel, &QAbstractItemModel::modelReset, [&]() {
        DisableButtons();
        SelectionModel->clear();
    });
}

void ModelingWidget::DisableButtons() {
    std::array<QPushButton*, 4> buttons {
            AddStructureButton,
            CombineWithStructureButton,
            RefineWithStructureButton,
            RemoveStructureButton
    };
    std::for_each(buttons.begin(),
                  buttons.end(),
                  [](QPushButton* button) { button->setEnabled(false); });
}
