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
    explicit PipelinesWidget(QWidget* parent = nullptr);

    [[nodiscard]] static auto
    GetHeaderStyleSheet() noexcept -> QString;

    [[nodiscard]] static auto
    GenerateIcon(const std::string& filePrefix) noexcept -> QIcon;

private slots:
    void AddPipeline();
    void RemovePipeline();
    void PreviousPipeline();
    void NextPipeline();
    void UpdatePipelineView();

private:
    void InitializeViews();

    void CreateArtifactsViewsForCurrentPipeline();

    Pipeline& GetCurrentPipeline();

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
