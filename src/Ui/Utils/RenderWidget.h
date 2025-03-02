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
    enum struct WindowWidthSliderMode : uint8_t {
        NONE = 0,
        INLINE,
        ABOVE
    };

    struct Controls {
        bool Render;
        bool ResetCamera;
        WindowWidthSliderMode WindowWidthSlider;
        bool Export;

        explicit operator bool() const noexcept {
            return Render || ResetCamera || WindowWidthSlider != WindowWidthSliderMode::NONE || Export;
        }
    };

    explicit RenderWidget(vtkImageAlgorithm* imageAlgorithm = nullptr,
                          Controls controls = { true, true, WindowWidthSliderMode::INLINE, true },
                          QWidget* parent = nullptr);

public Q_SLOTS:
    auto
    Render() const -> void;

    auto
    UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) const -> void;

    auto
    UpdateImageAlgorithm(vtkImageData& imageData) const -> void;

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

    auto static
    SetWindowWidth(Range<double> range) -> void;

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
    UpdateImageAlgorithm(vtkImageData& imageData) const -> void;

    auto
    UpdateWindowWidth(Range<double> range) const -> void;

protected:
    auto
    showEvent(QShowEvent* event) -> void override;

private:
    auto
    UpdateImageAlgorithm(vtkAlgorithm& imageAlgorithm) -> void;

    auto
    UpdateColorMappingFunctions() const -> void;

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
