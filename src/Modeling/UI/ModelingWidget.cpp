#include "ModelingWidget.h"
#include "CtStructureEditDialog.h"
#include "CtStructureDelegate.h"
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

#include <qdockwidget.h>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

ModelingWidget::ModelingWidget() :
        ResetCameraButton(new QPushButton("Reset Camera")),
        AddStructureButton(new QPushButton("Add Structure")),
        CombineWithStructureButton(new QPushButton("Combine With Structure")),
        RefineWithStructureButton(new QPushButton("Refine With Structure")),
        RemoveStructureButton(new QPushButton("Remove Structure")),
        TreeModel(new CtDataCsgTreeModel(*App::GetInstance()->GetCtDataCsgTree())),
        TreeView(new QTreeView()),
        SelectionModel(nullptr),
        CtStructureCreateDialog(nullptr),
        DataSource(nullptr),
        DataTree(App::GetInstance()->GetCtDataCsgTree()),
        OrientationMarkerWidget(vtkOrientationMarkerWidget::New()),
        Renderer(vtkOpenGLRenderer::New()),
        RenderWindowInteractor(QVTKInteractor::New()),
        InitialCameraPosition { 0.0, 0.5, -1.0 },
        InitialCamera(vtkCamera::New()) {

    SetUpCentralWidgetForRendering();

    SetUpDockWidgetForImplicitCsgTreeModeling();
}

ModelingWidget::~ModelingWidget() {
    OrientationMarkerWidget->Delete();
    Renderer->Delete();
    RenderWindowInteractor->Delete();
}

void ModelingWidget::OpenDialog(const std::function<const void()>& onAccepted) {
    CtStructureCreateDialog = new CtStructureEditDialog(this, true);
    CtStructureCreateDialog->HideImplicitStructureCombinationSection();
    CtStructureCreateDialog->setModal(true);
    CtStructureCreateDialog->show();

    connect(CtStructureCreateDialog, &CtStructureEditDialog::accepted, onAccepted);
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

void ModelingWidget::SetUpDockWidgetForImplicitCsgTreeModeling() {
    auto* treeDelegate = new CtStructureDelegate();
    TreeView->setModel(TreeModel);
    TreeView->setItemDelegate(treeDelegate);
    SelectionModel = TreeView->selectionModel();

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
    ConnectButtons();

    verticalLayout->addWidget(treeButtonBarWidget);
    verticalLayout->addWidget(TreeView);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

void ModelingWidget::ConnectButtons() {
    connect(ResetCameraButton, &QPushButton::clicked, [&]() {
        Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
        RenderWindowInteractor->Render();
    });

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
