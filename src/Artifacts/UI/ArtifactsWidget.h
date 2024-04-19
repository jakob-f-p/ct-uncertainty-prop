#pragma once

#include <QMainWindow>

#include <vtkNew.h>

class QPushButton;

class QVTKInteractor;
class vtkCamera;
class vtkOrientationMarkerWidget;
class vtkOpenGLRenderer;

class CtDataSource;
class CtStructureTree;
class ImageArtifactConcatenation;

class ArtifactsWidget : public QMainWindow {
public:
    ArtifactsWidget();

private:
    void SetUpCentralRenderingWidget();
    void SetUpDockWidget();

    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    vtkNew<CtDataSource> DataSource;
//    ImageArtifactConcatenation& ImageArtifacts;

    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
    vtkNew<vtkCamera> InitialCamera;
};