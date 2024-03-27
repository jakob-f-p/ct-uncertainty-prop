#include "ModelingWidget.h"

#include "CtStructureTreeModel.h"
#include "CtStructureEditDialog.h"
#include "CtStructureDelegate.h"
#include "../CtStructureTree.h"
#include "../CtDataSource.h"
#include "../BasicStructure.h"
#include "../../App.h"

#include <vtkAxesActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkColorTransferFunction.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkTextActor.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <QDockWidget>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

ModelingWidget::ModelingWidget() :
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
        CtStructureCreateDialog(nullptr),
        DataSource(nullptr),
        DataTree(&App::GetInstance()->GetCtDataTree()),
        OrientationMarkerWidget(vtkOrientationMarkerWidget::New()),
        Renderer(vtkOpenGLRenderer::New()),
        RenderWindowInteractor(QVTKInteractor::New()),
        InitialCameraPosition { 0.0, 0.5, -1.0 },
        InitialCamera(vtkCamera::New()) {

    SetUpCentralWidgetForRendering();

    SetUpDockWidgetForImplicitCtDataModeling();
}

ModelingWidget::~ModelingWidget() {
    OrientationMarkerWidget->Delete();
    Renderer->Delete();
    RenderWindowInteractor->Delete();
}

void ModelingWidget::SetUpCentralWidgetForRendering() {
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

    Renderer->AddVolume(volume);
    Renderer->SetBackground(0.2, 0.2, 0.2);
    Renderer->ResetCamera();
    InitialCamera->DeepCopy(Renderer->GetActiveCamera());

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->SetWindowName("CT-Data");
    renderWindow->AddRenderer(Renderer);

    renderWindow->SetInteractor(RenderWindowInteractor);
    RenderWindowInteractor->Initialize();
    vtkNew<vtkInteractorStyleTrackballCamera> trackballCameraStyle;
    RenderWindowInteractor->SetInteractorStyle(trackballCameraStyle);

    auto* renderingWidget = new QVTKOpenGLNativeWidget();
    renderingWidget->setRenderWindow(renderWindow);

    renderWindow->Render();

    vtkNew<vtkAxesActor> axesActor;
    axesActor->SetTotalLength(20.0, 20.0, 20.0);
    OrientationMarkerWidget->SetOrientationMarker(axesActor);
    OrientationMarkerWidget->SetViewport(0.8, 0.0, 1.0, 0.2);
    OrientationMarkerWidget->SetInteractor(RenderWindowInteractor);
    OrientationMarkerWidget->EnabledOn();
    OrientationMarkerWidget->InteractiveOff();

    setCentralWidget(renderingWidget);

    vtkNew<vtkCallbackCommand> onDataChangedUpdater;
    onDataChangedUpdater->SetClientData(RenderWindowInteractor);
    onDataChangedUpdater->SetCallback([](vtkObject*, unsigned long, void* rwi, void*) {
        auto* renderWindowInteractor = static_cast<QVTKInteractor*>(rwi);
        renderWindowInteractor->Render();
    });
    DataTree->AddObserver(vtkCommand::ModifiedEvent, onDataChangedUpdater);
}

void ModelingWidget::SetUpDockWidgetForImplicitCtDataModeling() {
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
    connect(ResetCameraButton, &QPushButton::clicked, [&]() {
        Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
        RenderWindowInteractor->Render();
    });

    connect(AddStructureButton, &QPushButton::clicked, [&]() {
        OpenCreateDialog([&]() {
            BasicStructureDetails dialogData = CtStructureCreateDialog->GetBasicStructureData();
            QModelIndex siblingIndex = SelectionModel->currentIndex();
            QModelIndex newIndex = TreeModel->AddBasicStructure(dialogData, siblingIndex);
            SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
        });
    });

    connect(CombineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenCreateDialog([&]() {
            BasicStructureDetails dialogData = CtStructureCreateDialog->GetBasicStructureData();
            TreeModel->CombineWithBasicStructure(dialogData);
            TreeView->expandAll();
        });
    });

    connect(RefineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenCreateDialog([&]() {
            BasicStructureDetails dialogData = CtStructureCreateDialog->GetBasicStructureData();
            QModelIndex index = SelectionModel->currentIndex();
            TreeModel->RefineWithBasicStructure(dialogData, index);
            TreeView->expandAll();
        });
    });

    connect(RemoveStructureButton, &QPushButton::clicked, [&]() {
        QModelIndex structureIndex = SelectionModel->currentIndex();
        TreeModel->RemoveBasicStructure(structureIndex);
        TreeView->expandAll();
    });

    connect(SelectionModel, &QItemSelectionModel::currentChanged, [&](const QModelIndex& current) {
        bool isBasicStructure = current.isValid() && static_cast<CtStructure*>(current.internalPointer())->IsBasicStructure();
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

void ModelingWidget::OpenCreateDialog(const std::function<const void()>& onAccepted) {
    CtStructureCreateDialog = new CtStructureEditDialog(CtStructureEditDialog::CREATE, CtStructure::SubType::BASIC,
                                                        BasicStructure::ImplicitFunctionType::INVALID, this);
    CtStructureCreateDialog->show();

    connect(CtStructureCreateDialog, &CtStructureEditDialog::accepted, onAccepted);
}
