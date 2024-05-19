#include "RenderWidget.h"

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageAlgorithm.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

RenderWidget::RenderWidget(vtkImageAlgorithm& imageAlgorithm, QWidget* parent) :
        QVTKOpenGLNativeWidget(parent),
        ImageAlgorithm(&imageAlgorithm) {

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

    UpdateImageAlgorithm(imageAlgorithm);

    vtkNew<vtkVolume> volume;
    volume->SetMapper(VolumeMapper);
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
}

RenderWidget::~RenderWidget() = default;

auto RenderWidget::ResetCamera() const -> void {
    Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
    RenderWindowInteractor->Render();
}

auto RenderWidget::Render() const -> void {
    RenderWindowInteractor->Render();
}

auto RenderWidget::UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) -> void {
    ImageAlgorithm = &imageAlgorithm;
    VolumeMapper->SetInputConnection(ImageAlgorithm->GetOutputPort());
}

auto RenderWidget::GetCurrentFilter() -> vtkImageAlgorithm& {
    if (!ImageAlgorithm)
        throw std::runtime_error("Filter must not be null");

    return *ImageAlgorithm;
}
