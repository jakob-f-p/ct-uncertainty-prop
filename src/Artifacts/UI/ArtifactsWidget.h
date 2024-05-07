#pragma once

#include "../../Utils/RenderWidget.h"

#include <QMainWindow>

class QPushButton;

class ArtifactRenderWidget;
class CtDataSource;
class ImageArtifactConcatenation;
class Pipeline;
class PipelineList;
class PipelinesWidget;

class ArtifactsWidget : public QMainWindow {
public:
    explicit ArtifactsWidget(PipelineList& pipelines, CtDataSource& dataSource);

    [[nodiscard]] auto
    GetCurrentFilter() -> vtkImageAlgorithm&;

private:
    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    PipelinesWidget* PipelineWidget;
    ArtifactRenderWidget* RenderWidget;
};


class ArtifactRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit ArtifactRenderWidget(PipelineList& pipelines,
                                  Pipeline& pipeline,
                                  CtDataSource& dataSource,
                                  QWidget* parent = nullptr);
    ~ArtifactRenderWidget() override;

public slots:
    auto
    UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) -> void;

private:
    auto
    GetUpdatedFilter(Pipeline const& pipeline) -> vtkImageAlgorithm&;

    CtDataSource* DataSource;
};
