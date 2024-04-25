#include "ArtifactsWidget.h"

#include "PipelinesWidget.h"
#include "../ImageArtifactConcatenation.h"
#include "../PipelineList.h"
#include "../../Modeling/CtDataSource.h"
#include "../../App.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>

ArtifactsWidget::ArtifactsWidget() :
        ResetCameraButton(new QPushButton("Reset Camera")),
        RenderButton(new QPushButton("Render")),
        RenderWidget(new ArtifactRenderWidget(App::GetInstance()->GetPipelines().Get(0).GetImageArtifactConcatenation())),
        PipelineWidget(new PipelinesWidget(*RenderWidget)) {

    RenderWidget = new ArtifactRenderWidget(PipelineWidget->GetCurrentPipeline().GetImageArtifactConcatenation());
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
    auto* renderingHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingHorizontalLayout->addWidget(ResetCameraButton);
    renderingHorizontalLayout->addWidget(RenderButton);
    renderingHorizontalLayout->addStretch();
    verticalLayout->addWidget(renderingButtonBarWidget);

    connect(ResetCameraButton, &QPushButton::clicked, [&]() { RenderWidget->ResetCamera(); });
    connect(RenderButton, &QPushButton::clicked, [&]() {
        RenderWidget->UpdateImageArtifactFiltersOnPipelineChange(
                PipelineWidget->GetCurrentPipeline().GetImageArtifactConcatenation()); });

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    verticalLayout->addWidget(PipelineWidget);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}



ArtifactRenderWidget::ArtifactRenderWidget(ImageArtifactConcatenation& imageArtifactConcatenation, QWidget* parent) :
        QVTKOpenGLNativeWidget(parent),
        Pipelines(App::GetInstance()->GetPipelines()) {
    DataSource->SetDataTree(&App::GetInstance()->GetCtDataTree());

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

    imageArtifactConcatenation.UpdateArtifactFilter();
    VolumeMapper->SetInputConnection(imageArtifactConcatenation.GetArtifactFilter().GetOutputPort());

    vtkNew<vtkVolume> volume;
    volume->SetMapper(VolumeMapper);
    volume->SetProperty(volumeProperty);

    Renderer->AddVolume(volume);
    Renderer->SetBackground(0.2, 0.2, 0.2);
    Renderer->ResetCamera();
    InitialCamera->DeepCopy(Renderer->GetActiveCamera());

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->SetWindowName("Artifacts");
    renderWindow->AddRenderer(Renderer);

    renderWindow->SetInteractor(RenderWindowInteractor);
    RenderWindowInteractor->Initialize();
    vtkNew<vtkInteractorStyleTrackballCamera> const trackballCameraStyle;
    RenderWindowInteractor->SetInteractorStyle(trackballCameraStyle);

    setRenderWindow(renderWindow);

    renderWindow->Render();

    vtkNew<vtkAxesActor> axesActor;
    axesActor->SetTotalLength(20.0, 20.0, 20.0);
    OrientationMarkerWidget->SetOrientationMarker(axesActor);
    OrientationMarkerWidget->SetViewport(0.8, 0.0, 1.0, 0.2);
    OrientationMarkerWidget->SetInteractor(RenderWindowInteractor);
    OrientationMarkerWidget->EnabledOn();
    OrientationMarkerWidget->InteractiveOff();

    Pipelines.AddPipelineEventCallback([renderWindowInteractor = RenderWindowInteractor.Get()]()
                                  { renderWindowInteractor->Render(); });
}

void ArtifactRenderWidget::ResetCamera() const {
    Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
    RenderWindowInteractor->Render();
}

auto ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange(ImageArtifactConcatenation& imageArtifactConcatenation) -> void {
    imageArtifactConcatenation.UpdateArtifactFilter();
    auto& filter = imageArtifactConcatenation.GetArtifactFilter();
    VolumeMapper->SetInputConnection(filter.GetOutputPort());
    RenderWindowInteractor->Render();
}
