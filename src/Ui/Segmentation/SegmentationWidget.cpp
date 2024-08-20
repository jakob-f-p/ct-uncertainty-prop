#include "SegmentationWidget.h"

#include "SegmentationFilterWidget.h"
#include "../../Artifacts/PipelineList.h"
#include "../../Modeling/CtDataSource.h"
#include "../../App.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

SegmentationWidget::SegmentationWidget(ThresholdFilter& thresholdFilter) :
        Pipeline_(nullptr),
        FilterWidget(new SegmentationFilterWidget(thresholdFilter)),
        RenderWidget(new SegmentationRenderWidget(FilterWidget->GetFilter())) {

    setCentralWidget(RenderWidget);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setContentsMargins({});

    auto* dockWidgetContent = new QWidget();
    auto* vLayout = new QVBoxLayout(dockWidgetContent);
    vLayout->setContentsMargins({ 10, 0, 10, 10 });

    vLayout->addWidget(FilterWidget);
    FilterWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

auto SegmentationWidget::UpdateDataSourceOnPipelineChange(Pipeline& pipeline) -> void {
    Pipeline_ = &pipeline;

    auto& app = App::GetInstance();
    auto& ctDataSource = app.GetCtDataSource();
    auto artifactsPipeline = [&app, &pipeline]() {
        switch (app.GetCtDataSourceType()) {
            case App::CtDataSourceType::IMPLICIT: return pipeline.GetArtifactsAlgorithm();
            case App::CtDataSourceType::IMPORTED: return pipeline.GetImageArtifactsAlgorithm();
            default: throw std::runtime_error("invalid data source type");
        }
    }();
    artifactsPipeline.In.SetInputConnection(ctDataSource.GetOutputPort());

    RenderWidget->UpdateDataSource(artifactsPipeline.Out);
}

auto SegmentationWidget::UpdateDataSourceOnDataSourceChange() -> void {
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

    RenderWidget->UpdateDataSource(artifactsPipeline.Out);
}


SegmentationRenderWidget::SegmentationRenderWidget(vtkImageAlgorithm& segmentationFilter) :
        DataSource(nullptr),
        SegmentationFilter(&segmentationFilter) {}

SegmentationRenderWidget::~SegmentationRenderWidget() = default;

auto SegmentationRenderWidget::UpdateDataSource(vtkImageAlgorithm& dataSource) -> void {
    DataSource = &dataSource;

    UpdateFilter();
}

auto SegmentationRenderWidget::UpdateSegmentationFilter(vtkImageAlgorithm& segmentationFilter) -> void {
    SegmentationFilter = &segmentationFilter;

    UpdateFilter();
}

auto SegmentationRenderWidget::UpdateFilter() -> void {
    SegmentationFilter->SetInputConnection(DataSource->GetOutputPort());

    UpdateImageAlgorithm(*SegmentationFilter);

    SegmentationFilter->Update();
    Render();
}
