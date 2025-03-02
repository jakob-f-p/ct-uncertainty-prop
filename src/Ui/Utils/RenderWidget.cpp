#include "RenderWidget.h"

#include "RangeSlider.h"

#include <QFileDialog>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkFloatArray.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkTrivialProducer.h>
#include <vtkVolumeProperty.h>


RenderWidget::RenderWidget(vtkImageAlgorithm* imageAlgorithm, Controls controls, QWidget* parent) :
        VtkRenderWidget(new CtRenderWidget(this, imageAlgorithm, parent)) {

    setFrameShape(Box);
    setFrameShadow(Sunken);
    setLineWidth(1);
    setMidLineWidth(1);

    if (!controls)
        return;

    auto* controlBar = new QFrame();
    auto* controlBarVLayout = new QVBoxLayout(controlBar);
    controlBarVLayout->setContentsMargins({ 10, 10, 10, 10 });
    controlBarVLayout->setSpacing(5);

    auto* bottomControlBarHLayout = new QHBoxLayout();
    bottomControlBarHLayout->setContentsMargins({});
    controlBarVLayout->addLayout(bottomControlBarHLayout);

    if (controls.Render) {
        RenderButton = new QPushButton("Render");
        bottomControlBarHLayout->addWidget(RenderButton);
    }

    if (controls.Render) {
        ResetCameraButton = new QPushButton("Reset Camera");
        bottomControlBarHLayout->addWidget(ResetCameraButton);
    }

    bottomControlBarHLayout->addStretch();

    if (controls.WindowWidthSlider != WindowWidthSliderMode::NONE) {
        Slider = new LabeledRangeSlider("Window Width [HU]", { -1500, 4500, 1 });
        Slider->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

        switch (controls.WindowWidthSlider) {
            case WindowWidthSliderMode::INLINE: {
                bottomControlBarHLayout->addWidget(Slider);
                bottomControlBarHLayout->addStretch();
                break;
            }

            case WindowWidthSliderMode::ABOVE: {
                auto* topControlBarHLayout = new QHBoxLayout();
                topControlBarHLayout->setContentsMargins({});
                topControlBarHLayout->addStretch();
                topControlBarHLayout->addWidget(Slider);
                topControlBarHLayout->addStretch();
                controlBarVLayout->insertLayout(0, topControlBarHLayout);
                break;
            }

            default:
                throw std::runtime_error("invalid window width slider mode");
        }
    }

    if (controls.Export) {
        ExportButton = new QPushButton("Export");
        bottomControlBarHLayout->addWidget(ExportButton);
    }


    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});
    vLayout->setSpacing(0);
    vLayout->addWidget(VtkRenderWidget);
    vLayout->addWidget(controlBar);


    if (RenderButton)
        connect(RenderButton, &QPushButton::clicked, VtkRenderWidget, &CtRenderWidget::Render);

    if (ResetCameraButton)
        connect(ResetCameraButton, &QPushButton::clicked, VtkRenderWidget, &CtRenderWidget::ResetCamera);

    if (Slider) {
        connect(Slider, &LabeledRangeSlider::ValueChanged, this, [this](RangeSlider::Range range) {
            VtkRenderWidget->UpdateWindowWidth({ static_cast<double>(range.Min), static_cast<double>(range.Max) });
        });
    }

    if (ExportButton)
        connect(ExportButton, &QPushButton::clicked, VtkRenderWidget, &CtRenderWidget::Export);
}

auto RenderWidget::Render() const -> void {
    VtkRenderWidget->Render();
}

auto RenderWidget::UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) const -> void {
    VtkRenderWidget->UpdateImageAlgorithm(imageAlgorithm);

    VtkRenderWidget->Render();
}

auto RenderWidget::UpdateImageAlgorithm(vtkImageData& imageData) const -> void {
    VtkRenderWidget->UpdateImageAlgorithm(imageData);

    VtkRenderWidget->Render();
}


CtRenderWidget::CtRenderWidget(RenderWidget* renderWidget, vtkImageAlgorithm* imageAlgorithm, QWidget* parent) :
        QVTKOpenGLNativeWidget(parent),
        Parent(renderWidget),
        ImageAlgorithm([imageAlgorithm]() -> vtkSmartPointer<vtkAlgorithm> {
            if (imageAlgorithm)
                return imageAlgorithm;

            vtkNew<vtkImageData> const trivialImage;
            trivialImage->SetDimensions(101, 101, 101);
            trivialImage->SetSpacing(1, 1, 1);
            trivialImage->SetOrigin(-50, -50, -50);

            vtkNew<vtkFloatArray> const trivialDataArray;
            trivialDataArray->SetNumberOfComponents(1);
            trivialDataArray->SetName("Radiodensities");
            trivialDataArray->SetNumberOfTuples(trivialImage->GetNumberOfPoints());
            trivialDataArray->FillValue(-1000.0F);
            trivialImage->GetPointData()->AddArray(trivialDataArray);
            trivialImage->GetPointData()->SetActiveScalars("Radiodensities");

            vtkSmartPointer trivialImageSource = vtkTrivialProducer::New();
            trivialImageSource->SetOutput(trivialImage);

            return trivialImageSource;
        }()) {

    setContentsMargins({});

    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    UpdateImageAlgorithm(*ImageAlgorithm);

    UpdateColorMappingFunctions();

    vtkNew<vtkVolumeProperty> const volumeProperty;
    volumeProperty->SetColor(ColorTransferFunction.GetPointer());
    volumeProperty->SetScalarOpacity(OpacityMappingFunction.GetPointer());
    volumeProperty->ShadeOff();
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->SetAmbient(0.3);

    vtkNew<vtkVolume> const volume;
    volume->SetMapper(VolumeMapper);
    volume->SetProperty(volumeProperty);

    Renderer->AddVolume(volume);
    Renderer->SetBackground(0.0, 0.0, 0.0);
    Renderer->ResetCamera();
    InitialCamera->DeepCopy(Renderer->GetActiveCamera());

    vtkNew<vtkGenericOpenGLRenderWindow> const renderWindow;
    renderWindow->SetWindowName("CT-Data");
    renderWindow->AddRenderer(Renderer);
    renderWindow->SetInteractor(RenderWindowInteractor);
    RenderWindowInteractor->Initialize();

    vtkNew<vtkInteractorStyleTrackballCamera> const trackballCameraStyle;
    RenderWindowInteractor->SetInteractorStyle(trackballCameraStyle);

    setRenderWindow(renderWindow);
    renderWindow->Render();

    vtkNew<vtkAxesActor> const axesActor;
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
    auto const homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
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

auto CtRenderWidget::UpdateImageAlgorithm(vtkAlgorithm& imageAlgorithm) -> void {
    ImageAlgorithm = &imageAlgorithm;
    VolumeMapper->SetInputConnection(ImageAlgorithm->GetOutputPort());
}

auto CtRenderWidget::UpdateImageAlgorithm(vtkImageData& imageData) const -> void {
    VolumeMapper->SetInputData(&imageData);
}

auto CtRenderWidget::SetWindowWidth(Range<double> range) -> void {
    WindowWidth = { static_cast<int>(range.Min), static_cast<int>(range.Max) };
}

auto CtRenderWidget::UpdateWindowWidth(Range<double> range) const -> void {
    Range const newWindowWidth = { static_cast<int>(range.Min), static_cast<int>(range.Max) };
    if (newWindowWidth.Min == WindowWidth.Min && newWindowWidth.Max == WindowWidth.Max)
        return;

    WindowWidth = newWindowWidth;
    UpdateColorMappingFunctions();
}

auto CtRenderWidget::showEvent(QShowEvent* event) -> void {
    UpdateColorMappingFunctions();

    if (auto const [sliderMin, sliderMax] = Parent->Slider->GetValue();
        sliderMin != WindowWidth.Min || sliderMax != WindowWidth.Max)
        Parent->Slider->SetValue({ WindowWidth.Min, WindowWidth.Max });

    QWidget::showEvent(event);
}

auto CtRenderWidget::UpdateColorMappingFunctions() const -> void {
    OpacityMappingFunction->RemoveAllPoints();
    ColorTransferFunction->RemoveAllPoints();

    double const belowLow = std::max(WindowWidth.Min - std::abs(0.25 * WindowWidth.Min), -1500.0);
    double const low = std::max(static_cast<double>(WindowWidth.Min), -1500.0);
    double const high = std::min(static_cast<double>(WindowWidth.Max), 3500.0);
    double const aboveHigh = std::min(WindowWidth.Max + std::abs(0.25 * WindowWidth.Max), 3500.0);
//    OpacityMappingFunction->AddPoint(-1500.0, 0.003, 0.5, 0.5);
//    OpacityMappingFunction->AddPoint(belowLow, 0.003, 0.0, 0.0);
//    OpacityMappingFunction->AddPoint(low, 0.005, 0.5, 0.0);
//    OpacityMappingFunction->AddPoint(high, 0.07, 0.0, 0.0);
//    OpacityMappingFunction->AddPoint(high + 1.0, 0.000);
    OpacityMappingFunction->AddPoint(belowLow, 0.005, 1.0, 1.0);
    OpacityMappingFunction->AddPoint(low, 0.03, 0.5, 0.0);
    OpacityMappingFunction->AddPoint(high, 0.03, 0.0, 0.0);
//    OpacityMappingFunction->AddPoint(low, 0.07, 0.5, 0.0);  // fine structures
//    OpacityMappingFunction->AddPoint(high, 0.07, 0.0, 0.0); // fine structures
    OpacityMappingFunction->AddPoint(aboveHigh, 0.005, 0.0, 0.0);

//    ColorTransferFunction->AddRGBPoint(-1500.0, 0.0, 0.0, 0.0, 0.5, 0.5);
//    ColorTransferFunction->AddRGBPoint(belowLow, 0.1, 0.1, 0.1, 0.0, 0.0);
    ColorTransferFunction->AddRGBPoint(low, 0.0, 0.0, 0.0, 0.5, 0.0);
    ColorTransferFunction->AddRGBPoint(high, 1.0, 1.0, 1.0, 0.0, 0.0);
}

CtRenderWidget::Range<int> CtRenderWidget::WindowWidth = { 0, 750 };
