#pragma once

#include <QItemSelectionModel>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>
#include <QVTKInteractor.h>

#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QStackedLayout>

class ArtifactsEditDialog;
class ImageArtifactsModel;
class Pipeline;
class PipelineList;

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