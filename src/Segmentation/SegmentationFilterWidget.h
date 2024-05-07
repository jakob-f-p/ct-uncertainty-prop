#pragma once

#include <QWidget>

#include <vtkSmartPointer.h>

class ThresholdFilterWidget;

class QVBoxLayout;

class vtkImageAlgorithm;

class SegmentationFilterWidget : public QWidget {
    Q_OBJECT

public:
    SegmentationFilterWidget();
    ~SegmentationFilterWidget() override;

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

public slots:
    void UpdateFilter();

signals:
    void FilterModified(vtkImageAlgorithm& segmentationFilter);

private:
    QVBoxLayout* VLayout;
    ThresholdFilterWidget* FilterWidget;

    vtkSmartPointer<vtkImageAlgorithm> SegmentationFilter;
};