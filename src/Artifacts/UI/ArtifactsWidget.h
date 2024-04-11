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
    ~ArtifactsWidget() override = default;

private:
    void SetUpCentralRenderingWidget();
    void SetUpDockWidget();

    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
//    vtkNew<vtkCamera> InitialCamera;
};