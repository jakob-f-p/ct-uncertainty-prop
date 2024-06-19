#pragma once

#include <QVTKOpenGLNativeWidget.h>

#include <QFrame>

class QPushButton;
class QVTKInteractor;

class CtRenderWidget;

class vtkCamera;
class vtkImageAlgorithm;
class vtkImageData;
class vtkOpenGLGPUVolumeRayCastMapper;
class vtkOpenGLRenderer;
class vtkOrientationMarkerWidget;


class RenderWidget : public QFrame {
    Q_OBJECT

public:
    struct Controls {
        bool Render;
        bool ResetCamera;
        bool Export;

        explicit operator bool() const noexcept { return Render || ResetCamera || Export; }
    };

    explicit RenderWidget(vtkImageAlgorithm& imageAlgorithm,
                          Controls controls = { true, true, true },
                          QWidget* parent = nullptr);

public Q_SLOTS:
    auto
    Render() const -> void;

    auto
    UpdateImageAlgorithm(vtkImageAlgorithm& imageAlgorithm) -> void;

    auto
    UpdateImageAlgorithm(vtkImageData& imageData) -> void;

private:
    CtRenderWidget* VtkRenderWidget;

    QPushButton* RenderButton = nullptr;
    QPushButton* ResetCameraButton = nullptr;
    QPushButton* ExportButton = nullptr;
};



class CtRenderWidget : public QVTKOpenGLNativeWidget {
    Q_OBJECT

public:
    explicit CtRenderWidget(vtkImageAlgorithm& imageAlgorithm, QWidget* parent = nullptr);
    ~CtRenderWidget() override;

    [[nodiscard]] auto
    GetCurrentFilter() -> vtkImageAlgorithm&;

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

private:
    vtkImageAlgorithm* ImageAlgorithm;
    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> VolumeMapper;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<vtkCamera> InitialCamera;
};
