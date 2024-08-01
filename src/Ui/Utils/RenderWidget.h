#pragma once

#include <QVTKOpenGLNativeWidget.h>

#include <QFrame>

class QPushButton;
class QVTKInteractor;

class CtRenderWidget;
class LabeledRangeSlider;

class vtkAlgorithm;
class vtkCamera;
class vtkImageAlgorithm;
class vtkImageData;
class vtkOpenGLGPUVolumeRayCastMapper;
class vtkOpenGLRenderer;
class vtkOrientationMarkerWidget;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;


class RenderWidget : public QFrame {
    Q_OBJECT

public:
    struct Controls {
        bool Render;
        bool ResetCamera;
        bool RangeSlider;
        bool Export;

        explicit operator bool() const noexcept { return Render || ResetCamera || RangeSlider || Export; }
    };

    explicit RenderWidget(vtkImageAlgorithm* imageAlgorithm = nullptr,
                          Controls controls = { true, true, true, true },
                          QWidget* parent = nullptr);

public Q_SLOTS:
    auto
    Render() const -> void;

    auto
    UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) -> void;

    auto
    UpdateImageAlgorithm(vtkImageData& imageData) -> void;

private:
    friend class CtRenderWidget;

    CtRenderWidget* VtkRenderWidget;

    QPushButton* RenderButton = nullptr;
    QPushButton* ResetCameraButton = nullptr;
    QPushButton* ExportButton = nullptr;
    LabeledRangeSlider* Slider = nullptr;
};



class CtRenderWidget : public QVTKOpenGLNativeWidget {
    Q_OBJECT

public:
    explicit CtRenderWidget(RenderWidget* renderWidget,
                            vtkImageAlgorithm* imageAlgorithm = nullptr,
                            QWidget* parent = nullptr);
    ~CtRenderWidget() override;

    template<typename T>
    requires std::is_arithmetic_v<T>
    struct Range { T Min, Max; };

public Q_SLOTS:
    auto
    ResetCamera() const -> void;

    auto
    Render() const -> void;

    auto
    Export() -> void;

    auto
    UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) -> void;

    auto
    UpdateImageAlgorithm(vtkImageData& imageData) -> void;

    auto
    UpdateWindowWidth(Range<double> range) -> void;

protected:
    auto
    showEvent(QShowEvent* event) -> void override;

private:
    auto
    UpdateImageAlgorithm(vtkAlgorithm& imageAlgorithm) -> void;

    auto
    UpdateColorMappingFunctions() -> void;

    RenderWidget* Parent;

    vtkSmartPointer<vtkAlgorithm> ImageAlgorithm;
    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> VolumeMapper;
    vtkNew<vtkPiecewiseFunction> OpacityMappingFunction;
    vtkNew<vtkColorTransferFunction> ColorTransferFunction;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<vtkCamera> InitialCamera;

    static Range<int> WindowWidth;
};
