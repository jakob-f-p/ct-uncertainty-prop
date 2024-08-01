#pragma once

#include "../Utils/RenderWidget.h"

#include <QMainWindow>

class QPushButton;

class CtDataSource;
class Pipeline;
class SegmentationFilterWidget;
class SegmentationRenderWidget;
class ThresholdFilter;


class SegmentationWidget : public QMainWindow {
public:
    explicit SegmentationWidget(ThresholdFilter& thresholdFilter);

    auto
    UpdateDataSourceOnPipelineChange(Pipeline& pipeline) -> void;

    auto
    UpdateDataSourceOnDataSourceChange() -> void;

private:
    Pipeline const* Pipeline_;
    SegmentationFilterWidget* FilterWidget;
    SegmentationRenderWidget* RenderWidget;
};


class SegmentationRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit SegmentationRenderWidget(vtkImageAlgorithm& segmentationFilter);
    ~SegmentationRenderWidget() override;

public Q_SLOTS:
    auto
    UpdateDataSource(vtkImageAlgorithm& dataSource) -> void;

    auto
    UpdateSegmentationFilter(vtkImageAlgorithm& segmentationFilter) -> void;

private:
    auto
    UpdateFilter() -> void;

    vtkImageAlgorithm* DataSource;
    vtkImageAlgorithm* SegmentationFilter;
};
