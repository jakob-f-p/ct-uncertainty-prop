#include "RenderWidget.h"

#include <QFileDialog>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

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


RenderWidget::RenderWidget(vtkImageAlgorithm& imageAlgorithm, Controls controls, QWidget* parent) :
        VtkRenderWidget(new CtRenderWidget(imageAlgorithm, parent)) {

    setFrameShape(QFrame::Shape::StyledPanel);
    setFrameShadow(QFrame::Shadow::Sunken);

    auto* vLayout = new QVBoxLayout(this);

    vLayout->addWidget(VtkRenderWidget);

    if (!controls)
        return;

    auto* controlBarWidget = new QWidget();
    auto* controlBarHLayout = new QHBoxLayout(controlBarWidget);

    if (controls.Render) {
        RenderButton = new QPushButton("Render");
        controlBarHLayout->addWidget(RenderButton);
        connect(RenderButton, &QPushButton::clicked, VtkRenderWidget, &CtRenderWidget::Render);
    }

    if (controls.Render) {
        ResetCameraButton = new QPushButton("Reset Camera");
        controlBarHLayout->addWidget(ResetCameraButton);
        connect(ResetCameraButton, &QPushButton::clicked, VtkRenderWidget, &CtRenderWidget::ResetCamera);
    }

    controlBarHLayout->addStretch();

    if (controls.Render) {
        ExportButton = new QPushButton("Export");
        controlBarHLayout->addWidget(ExportButton);
        connect(ExportButton, &QPushButton::clicked, VtkRenderWidget, &CtRenderWidget::Export);
    }

    vLayout->addWidget(controlBarWidget);
}

auto RenderWidget::Render() const -> void {
    VtkRenderWidget->Render();
}

auto RenderWidget::UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) -> void {
    VtkRenderWidget->UpdateImageAlgorithm(imageAlgorithm);

    VtkRenderWidget->Render();
}

auto RenderWidget::UpdateImageAlgorithm(vtkImageData& imageData) -> void {
    VtkRenderWidget->UpdateImageAlgorithm(imageData);

    VtkRenderWidget->Render();
}


CtRenderWidget::CtRenderWidget(vtkImageAlgorithm& imageAlgorithm, QWidget* parent) :
        QVTKOpenGLNativeWidget(parent),
        ImageAlgorithm(&imageAlgorithm) {

    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    UpdateImageAlgorithm(imageAlgorithm);

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

CtRenderWidget::~CtRenderWidget() = default;

auto CtRenderWidget::ResetCamera() const -> void {
    Renderer->GetActiveCamera()->DeepCopy(InitialCamera);
    RenderWindowInteractor->Render();
}

auto CtRenderWidget::Render() const -> void {
    RenderWindowInteractor->Render();
}

auto CtRenderWidget::Export() -> void {
    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const fileFilter = "Images (*.png)";
    QString const fileName = QFileDialog::getSaveFileName(this, "Save image", homePath, fileFilter);
    grabFramebuffer();
    grab().save(fileName);
}

auto CtRenderWidget::UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) -> void {
    ImageAlgorithm = &imageAlgorithm;
    VolumeMapper->SetInputConnection(ImageAlgorithm->GetOutputPort());
}

auto CtRenderWidget::UpdateImageAlgorithm(vtkImageData& imageData) -> void {
    VolumeMapper->SetInputData(&imageData);
}

auto CtRenderWidget::GetCurrentFilter() -> vtkImageAlgorithm& {
    if (!ImageAlgorithm)
        throw std::runtime_error("Filter must not be null");

    return *ImageAlgorithm;
}
