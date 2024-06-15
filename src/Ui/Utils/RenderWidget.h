#pragma once

#include <QVTKOpenGLNativeWidget.h>

class QVTKInteractor;
class vtkCamera;
class vtkImageAlgorithm;
class vtkImageData;
class vtkOpenGLGPUVolumeRayCastMapper;
class vtkOpenGLRenderer;
class vtkOrientationMarkerWidget;

class RenderWidget : public QVTKOpenGLNativeWidget {
    Q_OBJECT

public:
    explicit RenderWidget(vtkImageAlgorithm& imageAlgorithm, QWidget* parent = nullptr);
    ~RenderWidget() override;

    [[nodiscard]] auto
    GetCurrentFilter() -> vtkImageAlgorithm&;

public Q_SLOTS:
    auto
    ResetCamera() const -> void;

    auto
    Render() const -> void;

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
