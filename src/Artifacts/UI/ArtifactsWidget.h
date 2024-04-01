#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QVTKInteractor.h>

#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>

class ArtifactsWidget : public QMainWindow {
    Q_OBJECT

public:
    ArtifactsWidget();
    ~ArtifactsWidget() override;

private:
    void SetUpCentralRenderingWidget();
    void SetUpDockWidget();

    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    vtkOrientationMarkerWidget* OrientationMarkerWidget;
    vtkOpenGLRenderer* Renderer;
    QVTKInteractor* RenderWindowInteractor;
    std::array<double, 3> InitialCameraPosition;
    vtkCamera* InitialCamera;
};