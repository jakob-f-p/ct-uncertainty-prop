#pragma once

#include "../Utils/RenderWidget.h"

#include <QMainWindow>

class QPushButton;

class ArtifactRenderWidget;
class Pipeline;
class PipelineList;
class PipelinesWidget;

class ArtifactsWidget : public QMainWindow {
public:
    explicit ArtifactsWidget(PipelineList& pipelines);

    [[nodiscard]] auto
    GetCurrentPipeline() -> Pipeline&;

private:
    PipelinesWidget* PipelineWidget;
    ArtifactRenderWidget* RenderWidget;
};


class ArtifactRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit ArtifactRenderWidget(PipelineList& pipelines,
                                  Pipeline& pipeline,
                                  QWidget* parent = nullptr);
    ~ArtifactRenderWidget() override;

public Q_SLOTS:
    auto
    UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) -> void;
};
