#include "SegmentationWidget.h"

#include "SegmentationFilterWidget.h"
#include "../../Artifacts/PipelineList.h"
#include "../../Modeling/CtDataSource.h"

#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

SegmentationWidget::SegmentationWidget(CtDataSource& dataSource, ThresholdFilter& thresholdFilter) :
        ResetCameraButton(new QPushButton("Reset Camera")),
        RenderButton(new QPushButton("Render")),
        FilterWidget(new SegmentationFilterWidget(thresholdFilter)),
        RenderWidget(new SegmentationRenderWidget(dataSource, FilterWidget->GetFilter())) {

    connect(FilterWidget, &SegmentationFilterWidget::FilterModified,
            RenderWidget, &SegmentationRenderWidget::UpdateSegmentationFilter);

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
    connect(RenderButton, &QPushButton::clicked, [&]() { RenderWidget->Render(); });
    connect(RenderButton, &QPushButton::clicked, [&]() { FilterWidget->UpdateFilter(); });

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    verticalLayout->addWidget(line);

    verticalLayout->addWidget(FilterWidget);
    FilterWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);
}

auto SegmentationWidget::UpdateDataSource(vtkImageAlgorithm& dataSource) -> void {
    RenderWidget->UpdateDataSource(dataSource);
}


SegmentationRenderWidget::SegmentationRenderWidget(CtDataSource& ctDataSource,
                                                   vtkImageAlgorithm& segmentationFilter,
                                                   QWidget* parent) :
        RenderWidget([&]() -> vtkImageAlgorithm& {
            DataSource = &ctDataSource;
            SegmentationFilter = &segmentationFilter;

            SegmentationFilter->SetInputConnection(DataSource->GetOutputPort());

            return *SegmentationFilter;
        }(), parent) {}

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
