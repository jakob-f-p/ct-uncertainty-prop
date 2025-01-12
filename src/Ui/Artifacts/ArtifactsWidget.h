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
    GetCurrentPipeline() const -> Pipeline&;

    auto
    UpdateDataSource() const -> void;

private:
    PipelinesWidget* PipelineWidget;
    ArtifactRenderWidget* RenderWidget;
};


class ArtifactRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit ArtifactRenderWidget(PipelineList& pipelines, QWidget* parent = nullptr);
    ~ArtifactRenderWidget() override;

    auto
    UpdateDataSource() -> void;

public Q_SLOTS:
    auto
    UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) -> void;

private:
    Pipeline const* Pipeline_;
};
