#pragma once

#include <QWidget>

class ImageArtifactsWidget;
class Pipeline;
class PipelineList;
class StructureArtifactsWidget;

class QIcon;
class QPushButton;
class QLabel;
class QStackedLayout;

class PipelinesWidget : public QWidget {
    Q_OBJECT

public:
    explicit PipelinesWidget(PipelineList& pipelines, QWidget* parent = nullptr);

    auto GetCurrentPipeline() -> Pipeline&;

Q_SIGNALS:
    void PipelineViewUpdated(Pipeline& newPipeline);

private Q_SLOTS:
    void AddPipeline();
    void RemovePipeline();
    void PreviousPipeline();
    void NextPipeline();
    void UpdatePipelineView();

private:
    void InitializeViews();

    PipelineList& Pipelines;
    int CurrentPipelineIndex;

    QLabel* PipelineTitle;
    QPushButton* PreviousPipelineButton;
    QPushButton* NextPipelineButton;
    QPushButton* AddPipelineButton;
    QPushButton* RemovePipelineButton;

    StructureArtifactsWidget* StructureArtifactModelingWidget;
    ImageArtifactsWidget* ImageArtifactModelingWidget;
};
