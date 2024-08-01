#include "ArtifactsWidget.h"

#include "PipelinesWidget.h"
#include "../../Modeling/CtDataSource.h"
#include "../../Artifacts/Pipeline.h"
#include "../../Artifacts/PipelineList.h"
#include "../../App.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

#include <vtkImageAlgorithm.h>

ArtifactsWidget::ArtifactsWidget(PipelineList& pipelines) :
        PipelineWidget(new PipelinesWidget(pipelines)),
        RenderWidget(new ArtifactRenderWidget(pipelines)) {

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

auto ArtifactsWidget::UpdateDataSource() -> void {
    RenderWidget->UpdateImageArtifactFiltersOnPipelineChange(GetCurrentPipeline());
}


ArtifactRenderWidget::ArtifactRenderWidget(PipelineList& pipelines, QWidget* parent) :
        Pipeline_(nullptr) {
    pipelines.AddTreeEventCallback([&]() { Render(); });
}

ArtifactRenderWidget::~ArtifactRenderWidget() = default;

auto ArtifactRenderWidget::UpdateDataSource() -> void {
    if (!Pipeline_)
        return;

    auto& app = App::GetInstance();
    auto& ctDataSource = app.GetCtDataSource();
    auto artifactsPipeline = [this, &app]() {
        switch (app.GetCtDataSourceType()) {
            case App::CtDataSourceType::IMPLICIT: return Pipeline_->GetArtifactsAlgorithm();
            case App::CtDataSourceType::IMPORTED: return Pipeline_->GetImageArtifactsAlgorithm();
            default: throw std::runtime_error("invalid data source type");
        }
    }();
    artifactsPipeline.In.SetInputConnection(ctDataSource.GetOutputPort());

    UpdateImageAlgorithm(artifactsPipeline.Out);
}

auto ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) -> void {
    Pipeline_ = &newPipeline;

    auto& app = App::GetInstance();
    auto& ctDataSource = app.GetCtDataSource();
    auto artifactsPipeline = [&newPipeline, &app]() {
        switch (app.GetCtDataSourceType()) {
            case App::CtDataSourceType::IMPLICIT: return newPipeline.GetArtifactsAlgorithm();
            case App::CtDataSourceType::IMPORTED: return newPipeline.GetImageArtifactsAlgorithm();
            default: throw std::runtime_error("invalid data source type");
        }
    }();
    artifactsPipeline.In.SetInputConnection(ctDataSource.GetOutputPort());

    UpdateImageAlgorithm(artifactsPipeline.Out);
}
