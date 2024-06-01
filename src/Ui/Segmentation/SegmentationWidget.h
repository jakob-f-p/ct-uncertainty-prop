#pragma once

#include "../Utils/RenderWidget.h"

#include <QMainWindow>

class QPushButton;

class CtDataSource;
class SegmentationFilterWidget;
class SegmentationRenderWidget;
class ThresholdFilter;

class SegmentationWidget : public QMainWindow {
public:
    explicit SegmentationWidget(CtDataSource& dataSource, ThresholdFilter& thresholdFilter);

    auto
    UpdateDataSource(vtkImageAlgorithm& dataSource) -> void;

private:
    QPushButton* ResetCameraButton;
    QPushButton* RenderButton;

    SegmentationFilterWidget* FilterWidget;
    SegmentationRenderWidget* RenderWidget;
};


class SegmentationRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit SegmentationRenderWidget(CtDataSource& ctDataSource,
                                      vtkImageAlgorithm& segmentationFilter,
                                      QWidget* parent = nullptr);
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
