#pragma once

#include <QVTKOpenGLNativeWidget.h>

class QVTKInteractor;
class vtkCamera;
class vtkImageAlgorithm;
class vtkOrientationMarkerWidget;
class vtkOpenGLRenderer;

class RenderWidget : public QVTKOpenGLNativeWidget {
    Q_OBJECT

public:
    explicit RenderWidget(vtkImageAlgorithm& dataSource, QWidget* parent = nullptr);
    ~RenderWidget() override;

public slots:
    auto
    ResetCamera() const -> void;

    auto
    Render() const -> void;

protected:
    vtkNew<QVTKInteractor> RenderWindowInteractor;

private:
    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<vtkCamera> InitialCamera;
};
