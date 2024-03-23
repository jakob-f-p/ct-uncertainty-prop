#include "ArtifactsWidget.h"
#include "ImageArtifactConcatenationModel.h"
#include "ImageArtifactsDelegate.h"
#include "../../App.h"
#include "../../Modeling/UI/CtStructureDelegate.h"

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

#include <QDockWidget.h>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

ArtifactsWidget::ArtifactsWidget() :
        Pipelines(App::GetInstance()->GetPipelineList()),
        CurrentPipelineIndex(Pipelines->IsEmpty() ? -1 : 0),
        CurrentPipeline(nullptr),
        PipelineTitle(new QLabel()),
        StructureArtifactModelingWidget(new QWidget()),
        ImageArtifactModelingWidget(new QWidget()),
        ResetCameraButton(new QPushButton("Reset Camera")),
        RenderButton(new QPushButton("Render")),
        PreviousPipelineButton(new QPushButton("")),
        NextPipelineButton(new QPushButton("")),
        AddPipelineButton(new QPushButton(QIcon(QPixmap(":/Plus.png")), "")),
        RemovePipelineButton(new QPushButton(QIcon(QPixmap(":/Minus.png")), "")),
        OrientationMarkerWidget(vtkOrientationMarkerWidget::New()),
        Renderer(vtkOpenGLRenderer::New()),
        RenderWindowInteractor(QVTKInteractor::New()),
        InitialCameraPosition{ 0.0, 0.5, -1.0 },
        InitialCamera(vtkCamera::New()) {

    SetUpCentralWidgetForRendering();

    SetUpDockWidgetForAddingArtifacts();
}

ArtifactsWidget::~ArtifactsWidget() {
    OrientationMarkerWidget->Delete();
    Renderer->Delete();
    RenderWindowInteractor->Delete();
}

void ArtifactsWidget::SetUpCentralWidgetForRendering() {
//    DataSource = CtDataSource::New();
//    DataSource->SetDataTree(DataTree);
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

//    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
//    volumeMapper->SetInputConnection(DataSource->GetOutputPort());

    vtkNew<vtkVolume> volume;
//    volume->SetMapper(volumeMapper);
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
//    DataTree->AddObserver(vtkCommand::ModifiedEvent, onDataChangedUpdater);
}

void ArtifactsWidget::SetUpDockWidgetForAddingArtifacts() {
    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);

    auto* dockWidgetContent = new QWidget();
    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);

    auto* renderingButtonBarWidget = new QWidget();
    auto* renderingHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingHorizontalLayout->addWidget(ResetCameraButton);
    renderingHorizontalLayout->addWidget(RenderButton);
    renderingHorizontalLayout->addStretch();
    verticalLayout->addWidget(renderingButtonBarWidget);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    auto* artifactsTitleBarWidget = new QWidget();
    auto* artifactsTitleBarHorizontalLayout = new QHBoxLayout(artifactsTitleBarWidget);
    artifactsTitleBarHorizontalLayout->setContentsMargins(0, 11, 0, 0);
    PipelineTitle->setStyleSheet("font-size: 14px; font-weight: bold");
    artifactsTitleBarHorizontalLayout->addWidget(PipelineTitle);
    artifactsTitleBarHorizontalLayout->addStretch();
    PreviousPipelineButton->setIcon(GenerateIcon("ArrowLeft"));
    NextPipelineButton->setIcon(GenerateIcon("ArrowRight"));
    AddPipelineButton->setIcon(GenerateIcon("Plus"));
    RemovePipelineButton->setIcon(GenerateIcon("Minus"));
    artifactsTitleBarHorizontalLayout->addWidget(PreviousPipelineButton);
    artifactsTitleBarHorizontalLayout->addWidget(NextPipelineButton);
    artifactsTitleBarHorizontalLayout->addWidget(AddPipelineButton);
    artifactsTitleBarHorizontalLayout->addWidget(RemovePipelineButton);
    ConnectButtons();
    verticalLayout->addWidget(artifactsTitleBarWidget);

    auto* structureArtifactModelingVerticalLayout = new QVBoxLayout(StructureArtifactModelingWidget);
    auto* structureArtifactModelingTitle = new QLabel("Structure Artifacts");
    structureArtifactModelingTitle->setStyleSheet("font-size: 14px; font-weight: bold");
    structureArtifactModelingVerticalLayout->addWidget(structureArtifactModelingTitle);
    verticalLayout->addWidget(StructureArtifactModelingWidget);

    auto* imageArtifactModelingVerticalLayout = new QVBoxLayout(ImageArtifactModelingWidget);
    auto* imageArtifactModelingTitle = new QLabel("Image Artifacts");
    imageArtifactModelingTitle->setStyleSheet("font-size: 14px; font-weight: bold");
    imageArtifactModelingVerticalLayout->addWidget(imageArtifactModelingTitle);
    verticalLayout->addWidget(ImageArtifactModelingWidget);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);

    InitializeViews();
    UpdatePipelineView();
}

void ArtifactsWidget::ConnectButtons() {
    connect(ResetCameraButton, &QPushButton::clicked, [&]() {
        Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
        RenderWindowInteractor->Render();
    });

    connect(AddPipelineButton, &QPushButton::clicked, this, &ArtifactsWidget::AddPipeline);
    connect(RemovePipelineButton, &QPushButton::clicked, this, &ArtifactsWidget::RemovePipeline);

    connect(PreviousPipelineButton, &QPushButton::clicked, this, &ArtifactsWidget::PreviousPipeline);
    connect(NextPipelineButton, &QPushButton::clicked, this, &ArtifactsWidget::NextPipeline);
}

void ArtifactsWidget::AddPipeline() {
    auto* newPipeline = Pipeline::New();

    auto* newDataTree = CtDataCsgTree::New();
    newDataTree->DeepCopy(App::GetInstance()->GetCtDataCsgTree());
    newPipeline->SetCtDataTree(newDataTree);
    Pipelines->AddPipeline(newPipeline);
    CurrentPipelineIndex = Pipelines->GetSize() - 1;

    newDataTree->FastDelete();
    newPipeline->FastDelete();

    auto* newStructureArtifactsView = new QTreeView();
    auto* newStructureArtifactsModel = new CtDataCsgTreeModel(*Pipelines->Get(CurrentPipelineIndex)->GetCtDataTree());
    newStructureArtifactsView->setModel(newStructureArtifactsModel);
    auto* newCtDataTreeDelegate = new CtStructureDelegate();
    newStructureArtifactsView->setItemDelegate(newCtDataTreeDelegate);
    StructureArtifactModelingWidget->layout()->addWidget(newStructureArtifactsView);

    auto* newImageArtifactsView = new QTreeView();
    auto* newImageArtifactsModel = new ImageArtifactConcatenationModel(Pipelines->Get(CurrentPipelineIndex)->GetImageArtifactConcatenation());
    newImageArtifactsView->setModel(newImageArtifactsModel);
    auto* newImageArtifactsDelegate = new ImageArtifactsDelegate();
    newImageArtifactsView->setItemDelegate(newImageArtifactsDelegate);
    ImageArtifactModelingWidget->layout()->addWidget(newImageArtifactsView);

    StructureArtifactsViews.push_back(newStructureArtifactsView);
    ImageArtifactsViews.push_back(newImageArtifactsView);

    UpdatePipelineView();
}

void ArtifactsWidget::RemovePipeline() {
    Pipelines->RemovePipeline(CurrentPipeline);

    auto* structureArtifactsViewToRemove = StructureArtifactsViews.at(CurrentPipelineIndex);
    StructureArtifactModelingWidget->layout()->removeWidget(structureArtifactsViewToRemove);
    StructureArtifactsViews.erase(std::next(StructureArtifactsViews.begin(), CurrentPipelineIndex));
    delete structureArtifactsViewToRemove;

    auto* imageArtifactsViewToRemove = ImageArtifactsViews.at(CurrentPipelineIndex);
    ImageArtifactModelingWidget->layout()->removeWidget(imageArtifactsViewToRemove);
    ImageArtifactsViews.erase(std::next(ImageArtifactsViews.begin(), CurrentPipelineIndex));
    delete imageArtifactsViewToRemove;

    if (Pipelines->IsEmpty()) {
        CurrentPipelineIndex = -1;
    } else if (CurrentPipelineIndex == Pipelines->GetSize()) {
        CurrentPipelineIndex--;
    }

    UpdatePipelineView();
}

void ArtifactsWidget::UpdatePipelineView() {
    bool currentPipelineIndexIsValid = CurrentPipelineIndex >= 0 && CurrentPipelineIndex < Pipelines->GetSize();
    QString pipelineTitleString = !currentPipelineIndexIsValid
            ? ""
            : (Pipelines->Get(CurrentPipelineIndex)->GetName().empty()
                    ? QString::fromStdString("Pipeline " + std::to_string(CurrentPipelineIndex + 1))
                    : QString::fromStdString(Pipelines->Get(CurrentPipelineIndex)->GetName()));
    PipelineTitle->setText(pipelineTitleString);

    CurrentPipeline = !currentPipelineIndexIsValid
            ? nullptr
            : Pipelines->Get(CurrentPipelineIndex);

    PreviousPipelineButton->setEnabled(CurrentPipelineIndex > 0);
    NextPipelineButton->setEnabled(CurrentPipelineIndex < Pipelines->GetSize() - 1);
    AddPipelineButton->setEnabled(Pipelines->GetSize() < 10);
    RemovePipelineButton->setEnabled(CurrentPipelineIndex > -1);

    for (int i = 0; i < Pipelines->NumberOfPipelines(); ++i) {
        if (i == CurrentPipelineIndex) {
            StructureArtifactsViews[i]->show();
            ImageArtifactsViews[i]->show();
        } else {
            StructureArtifactsViews[i]->hide();
            ImageArtifactsViews[i]->hide();
        }
    }
}

void ArtifactsWidget::PreviousPipeline() {
    if (CurrentPipelineIndex == 0) {
        qWarning("Cannot decrease pipeline index further");
        return;
    }

    CurrentPipelineIndex--;
    UpdatePipelineView();
}

void ArtifactsWidget::NextPipeline() {
    if (CurrentPipelineIndex == Pipelines->GetSize() - 1) {
        qWarning("Cannot decrease pipeline index further");
        return;
    }

    CurrentPipelineIndex++;
    UpdatePipelineView();
}

QIcon ArtifactsWidget::GenerateIcon(const std::string &filePrefix) {
    QIcon icon;
    QString qFilePrefix = QString::fromStdString(filePrefix);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Normal.png"), QIcon::Normal);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Disabled.png"), QIcon::Disabled);
    return icon;
}

void ArtifactsWidget::InitializeViews() {
    for (int i = 0; i < Pipelines->NumberOfPipelines(); ++i) {
        auto* newStructureArtifactsView = new QTreeView();
        auto* newStructureArtifactsModel = new CtDataCsgTreeModel( *Pipelines->Get(i)->GetCtDataTree());
        newStructureArtifactsView->setModel(newStructureArtifactsModel);
        auto* newCtDataTreeDelegate = new CtStructureDelegate();
        newStructureArtifactsView->setItemDelegate(newCtDataTreeDelegate);
        StructureArtifactModelingWidget->layout()->addWidget(newStructureArtifactsView);

        auto* newImageArtifactsView = new QTreeView();
        auto* newImageArtifactsModel = new ImageArtifactConcatenationModel( Pipelines->Get(i)->GetImageArtifactConcatenation());
        newImageArtifactsView->setModel(newImageArtifactsModel);
        auto* newImageArtifactsDelegate = new ImageArtifactsDelegate();
        newImageArtifactsView->setItemDelegate(newImageArtifactsDelegate);
        ImageArtifactModelingWidget->layout()->addWidget(newImageArtifactsView);

        StructureArtifactsViews.push_back(newStructureArtifactsView);
        ImageArtifactsViews.push_back(newImageArtifactsView);
    }
}
