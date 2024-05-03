#include "../../Modeling/CtDataSource.h"
#include "ArtifactsWidget.h"

#include "PipelinesWidget.h"
#include "../Image/ImageArtifactConcatenation.h"
#include "../PipelineList.h"
#include "../../App.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

ArtifactsWidget::ArtifactsWidget(PipelineList& pipelines) :
        ResetCameraButton(new QPushButton("Reset Camera")),
        RenderButton(new QPushButton("Render")),
        RenderWidget(new ArtifactRenderWidget(pipelines.Get(0).GetImageArtifactConcatenation())),
        PipelineWidget(new PipelinesWidget(pipelines)) {

    RenderWidget = new ArtifactRenderWidget(PipelineWidget->GetCurrentPipeline().GetImageArtifactConcatenation());
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



ArtifactRenderWidget::ArtifactRenderWidget(ImageArtifactConcatenation& imageArtifactConcatenation, QWidget* parent) :
        RenderWidget([&, dataTree = &App::GetInstance()->GetCtDataTree()]() -> vtkImageAlgorithm& {
            vtkNew<CtDataSource> dataSource;
            dataSource->SetDataTree(dataTree);

            imageArtifactConcatenation.UpdateArtifactFilter();
            auto& imageArtifactStartFilter = imageArtifactConcatenation.GetStartFilter();
            imageArtifactStartFilter.SetInputConnection(dataSource->GetOutputPort());

            return imageArtifactConcatenation.GetEndFilter();
        }(), parent),
        Pipelines(App::GetInstance()->GetPipelines()) {

    Pipelines.AddPipelineEventCallback([&]() { Render(); });
}

ArtifactRenderWidget::~ArtifactRenderWidget() = default;

auto ArtifactRenderWidget::UpdateImageArtifactFiltersOnPipelineChange(Pipeline const& newPipeline) const -> void {
    newPipeline.GetImageArtifactConcatenation().UpdateArtifactFilter();
    RenderWindowInteractor->Render();
}
