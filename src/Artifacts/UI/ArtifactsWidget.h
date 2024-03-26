#pragma once

#include "ImageArtifactsView.h"
#include "../PipelineList.h"
#include "../../Modeling/UI/CtDataCsgTreeModel.h"

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

class ArtifactsWidget : public QMainWindow {
    Q_OBJECT

public:
    ArtifactsWidget();
    ~ArtifactsWidget() override;

public slots:
    void AddPipeline();
    void RemovePipeline();
    void PreviousPipeline();
    void NextPipeline();
    void UpdatePipelineView();

private:
    void SetUpCentralWidgetForRendering();

    void SetUpDockWidgetForAddingArtifacts();

    void ConnectButtons();

    static QIcon GenerateIcon(const std::string& filePrefix);

    void InitializeViews();

    void CreateArtifactsViewsAndModels(int pipelineIdx);

    PipelineList* Pipelines;
    int CurrentPipelineIndex;
    Pipeline* CurrentPipeline;
    QStackedLayout* StructureArtifactsViews;
    QStackedLayout* ImageArtifactsViews;

    QLabel* PipelineTitle;
    QWidget* StructureArtifactModelingWidget;
    QWidget* ImageArtifactModelingWidget;

    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;
    QPushButton* PreviousPipelineButton;
    QPushButton* NextPipelineButton;
    QPushButton* AddPipelineButton;
    QPushButton* RemovePipelineButton;

    vtkOrientationMarkerWidget* OrientationMarkerWidget;
    vtkOpenGLRenderer* Renderer;
    QVTKInteractor* RenderWindowInteractor;
    std::array<double, 3> InitialCameraPosition;
    vtkCamera* InitialCamera;
};