#include "ArtifactsWidget.h"

#include "PipelinesWidget.h"
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
        RenderButton(new QPushButton("Render")) {
//        ImageArtifactConcatenation(App::GetInstance()->Get)
    SetUpCentralRenderingWidget();

    SetUpDockWidget();
}

void ArtifactsWidget::SetUpCentralRenderingWidget() {
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
    renderWindow->SetWindowName("CT-TData");
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
}

void ArtifactsWidget::SetUpDockWidget() {
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

    connect(ResetCameraButton, &QPushButton::clicked, [&]() {
        Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
        RenderWindowInteractor->Render();
    });

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    auto* pipelineWidget = new PipelinesWidget();
    verticalLayout->addWidget(pipelineWidget);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}
