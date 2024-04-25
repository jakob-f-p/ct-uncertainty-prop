#pragma once

#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkNew.h>

class QPushButton;

class vtkCamera;
class vtkOrientationMarkerWidget;
class vtkOpenGLRenderer;
class vtkOpenGLGPUVolumeRayCastMapper;

class ArtifactRenderWidget;
class CtDataSource;
class CtStructureTree;
class ImageArtifactConcatenation;
class PipelineList;
class PipelinesWidget;

class ArtifactsWidget : public QMainWindow {
public:
    ArtifactsWidget();

private:
    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    ArtifactRenderWidget* RenderWidget;
    PipelinesWidget* PipelineWidget;
};


class ArtifactRenderWidget : public QVTKOpenGLNativeWidget {
public:
    explicit ArtifactRenderWidget(ImageArtifactConcatenation& imageArtifactConcatenation, QWidget* parent = nullptr);

    void ResetCamera() const;

    auto
    UpdateImageArtifactFiltersOnPipelineChange(ImageArtifactConcatenation& imageArtifactConcatenation) -> void;

    vtkNew<CtDataSource> DataSource;
    PipelineList& Pipelines;
//    ImageArtifactConcatenation& ImageArtifacts;

    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> VolumeMapper;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
    vtkNew<vtkCamera> InitialCamera;
};
