#pragma once

#include <QIcon>
#include <QMetaObject>
#include <QPushButton>
#include <QLabel>
#include <QStackedLayout>

class ImageArtifactsWidget;
class Pipeline;
class PipelineList;
class StructureArtifactsWidget;

class PipelinesWidget : public QWidget {
    Q_OBJECT

public:
    PipelinesWidget();

    static QString GetHeaderStyleSheet();
    static QIcon GenerateIcon(const std::string& filePrefix);

private slots:
    void AddPipeline();
    void RemovePipeline();
    void PreviousPipeline();
    void NextPipeline();
    void UpdatePipelineView();

private:
    void InitializeViews();

    void CreateArtifactsViewsForCurrentPipeline();

    Pipeline* GetCurrentPipeline();

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
