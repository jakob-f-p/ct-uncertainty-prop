#include "ArtifactsWidget.h"

#include "PipelinesWidget.h"
#include "../../Artifacts/Pipeline.h"
#include "../../Artifacts/PipelineList.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

ArtifactsWidget::ArtifactsWidget(PipelineList& pipelines) :
        PipelineWidget(new PipelinesWidget(pipelines)),
        RenderWidget(new ArtifactRenderWidget(pipelines,
                                              PipelineWidget->GetCurrentPipeline())) {

    setCentralWidget(RenderWidget);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);

    auto* dockWidgetContent = new QWidget();

    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);

    connect(PipelineWidget, &PipelinesWidget::PipelineViewUpdated,
            RenderWidget, &ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange);

    verticalLayout->addWidget(PipelineWidget);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

auto ArtifactsWidget::GetCurrentPipeline() -> Pipeline& {
    return PipelineWidget->GetCurrentPipeline();
}


ArtifactRenderWidget::ArtifactRenderWidget(PipelineList& pipelines,
                                           Pipeline& pipeline,
                                           QWidget* parent) :
        RenderWidget(pipeline.GetImageAlgorithm()) {

    pipelines.AddTreeEventCallback([&]() { Render(); });
}

ArtifactRenderWidget::~ArtifactRenderWidget() = default;

auto ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) -> void {
    UpdateImageAlgorithm(newPipeline.GetImageAlgorithm());
}
