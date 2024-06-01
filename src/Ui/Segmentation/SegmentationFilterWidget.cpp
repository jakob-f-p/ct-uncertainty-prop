#include "SegmentationFilterWidget.h"

#include "../../Segmentation/ThresholdFilter.h"

#include <QVBoxLayout>

SegmentationFilterWidget::SegmentationFilterWidget(ThresholdFilter& thresholdFilter) :
        VLayout(new QVBoxLayout(this)),
        FilterWidget(new ThresholdFilterWidget()),
        SegmentationFilter(&thresholdFilter) {

    FilterWidget->Populate(dynamic_cast<ThresholdFilter&>(*SegmentationFilter));
    VLayout->addWidget(FilterWidget);
}

SegmentationFilterWidget::~SegmentationFilterWidget() = default;

auto SegmentationFilterWidget::GetFilter() const -> vtkImageAlgorithm& {
    if (!SegmentationFilter)
        throw std::runtime_error("Segmentation filter must not be null");

    return *SegmentationFilter;
}

void SegmentationFilterWidget::UpdateFilter() {
    FilterWidget->SetFilterData(dynamic_cast<ThresholdFilter&>(*SegmentationFilter));
}
