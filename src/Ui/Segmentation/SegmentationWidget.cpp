#include "SegmentationWidget.h"

#include "SegmentationFilterWidget.h"
#include "../../Artifacts/PipelineList.h"
#include "../../Modeling/CtDataSource.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

SegmentationWidget::SegmentationWidget(CtDataSource& dataSource, ThresholdFilter& thresholdFilter) :
        FilterWidget(new SegmentationFilterWidget(thresholdFilter)),
        RenderWidget(new SegmentationRenderWidget(dataSource, FilterWidget->GetFilter())) {

    setCentralWidget(RenderWidget);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);

    auto* dockWidgetContent = new QWidget();
    auto* vLayout = new QVBoxLayout(dockWidgetContent);

    vLayout->addWidget(FilterWidget);
    FilterWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

auto SegmentationWidget::UpdateDataSource(Pipeline& pipeline) -> void {
    RenderWidget->UpdateDataSource(pipeline.GetImageAlgorithm());
}


SegmentationRenderWidget::SegmentationRenderWidget(CtDataSource& ctDataSource,
                                                   vtkImageAlgorithm& segmentationFilter) :
        RenderWidget([&]() -> vtkImageAlgorithm& {
            DataSource = &ctDataSource;
            SegmentationFilter = &segmentationFilter;

            SegmentationFilter->SetInputConnection(DataSource->GetOutputPort());

            return *SegmentationFilter;
        }()) {}

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

    Render();
}
