#pragma once

#include "../../RenderWidget.h"

#include <QMainWindow>

class QPushButton;

class ArtifactRenderWidget;
class ImageArtifactConcatenation;
class Pipeline;
class PipelineList;
class PipelinesWidget;

class ArtifactsWidget : public QMainWindow {
public:
    explicit ArtifactsWidget(PipelineList& pipelines);

private:
    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    ArtifactRenderWidget* RenderWidget;
    PipelinesWidget* PipelineWidget;
};


class ArtifactRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit ArtifactRenderWidget(ImageArtifactConcatenation& imageArtifactConcatenation, QWidget* parent = nullptr);
    ~ArtifactRenderWidget() override;

public slots:
    auto
    UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) const -> void;

private:
    PipelineList& Pipelines;
//    ImageArtifactConcatenation& ImageArtifacts;
};
