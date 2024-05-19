#include "ArtifactsWidget.h"

#include "PipelinesWidget.h"
#include "../../Artifacts/Image/ImageArtifactConcatenation.h"
#include "../../Artifacts/Structure/StructureArtifactListCollection.h"
#include "../../Artifacts/PipelineList.h"
#include "../../Modeling/CtDataSource.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

ArtifactsWidget::ArtifactsWidget(PipelineList& pipelines, CtDataSource& dataSource) :
        ResetCameraButton(new QPushButton("Reset Camera")),
        RenderButton(new QPushButton("Render")),
        PipelineWidget(new PipelinesWidget(pipelines)),
        RenderWidget(new ArtifactRenderWidget(pipelines,
                                              PipelineWidget->GetCurrentPipeline(),
                                              dataSource)) {

    setCentralWidget(RenderWidget);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);

    auto* dockWidgetContent = new QWidget();
    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);

    auto* renderingButtonBarWidget = new QWidget();
    auto* renderingHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingHorizontalLayout->addWidget(ResetCameraButton);
    renderingHorizontalLayout->addWidget(RenderButton);
    renderingHorizontalLayout->addStretch();
    verticalLayout->addWidget(renderingButtonBarWidget);

    connect(ResetCameraButton, &QPushButton::clicked, [&]() { RenderWidget->ResetCamera(); });
    connect(RenderButton, &QPushButton::clicked, [&]() {
        RenderWidget->UpdateImageArtifactFiltersOnPipelineChange(PipelineWidget->GetCurrentPipeline()); });
    connect(PipelineWidget, &PipelinesWidget::PipelineViewUpdated,
            RenderWidget, &ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    verticalLayout->addWidget(PipelineWidget);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

auto ArtifactsWidget::GetCurrentFilter() -> vtkImageAlgorithm& {
    return RenderWidget->GetCurrentFilter();
}


ArtifactRenderWidget::ArtifactRenderWidget(PipelineList& pipelines,
                                           Pipeline& pipeline,
                                           CtDataSource& dataSource,
                                           QWidget* parent) :
        RenderWidget([&]() -> vtkImageAlgorithm& {
            DataSource = &dataSource;

            return GetUpdatedFilter(pipeline);
        }(), parent) {

    pipelines.AddPipelineEventCallback([&]() { Render(); });
}

ArtifactRenderWidget::~ArtifactRenderWidget() = default;

auto ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) -> void {
    UpdateImageAlgorithm(GetUpdatedFilter(newPipeline));

    Render();
}

auto ArtifactRenderWidget::GetUpdatedFilter(Pipeline const& pipeline) -> vtkImageAlgorithm& {
    auto& treeArtifacts = pipeline.GetTreeArtifacts();
    auto& treeArtifactsFilter = treeArtifacts.GetFilter();
    treeArtifactsFilter.SetInputConnection(DataSource->GetOutputPort());

    auto& imageArtifactConcatenation = pipeline.GetImageArtifactConcatenation();
    imageArtifactConcatenation.UpdateArtifactFilter();
    auto& imageArtifactStartFilter = imageArtifactConcatenation.GetStartFilter();
    imageArtifactStartFilter.SetInputConnection(treeArtifactsFilter.GetOutputPort());

    return imageArtifactConcatenation.GetEndFilter();
}
