#pragma once

#include <QWidget>

#include <vtkSmartPointer.h>

class ThresholdFilter;
class ThresholdFilterWidget;

class QVBoxLayout;

class vtkImageAlgorithm;


class SegmentationFilterWidget : public QWidget {
    Q_OBJECT

public:
    explicit SegmentationFilterWidget(ThresholdFilter& thresholdFilter);
    ~SegmentationFilterWidget() override;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

private:
    QVBoxLayout* VLayout;
    ThresholdFilterWidget* FilterWidget;

    vtkSmartPointer<vtkImageAlgorithm> SegmentationFilter;
};