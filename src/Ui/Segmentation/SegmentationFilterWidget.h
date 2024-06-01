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
    SegmentationFilterWidget(ThresholdFilter& thresholdFilter);
    ~SegmentationFilterWidget() override;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

public Q_SLOTS:
    void UpdateFilter();

Q_SIGNALS:
    void FilterModified(vtkImageAlgorithm& segmentationFilter);

private:
    QVBoxLayout* VLayout;
    ThresholdFilterWidget* FilterWidget;

    vtkSmartPointer<vtkImageAlgorithm> SegmentationFilter;
};