#include "SegmentationFilterWidget.h"

#include "../Utils/WidgetUtils.h"
#include "../../Segmentation/ThresholdFilter.h"

#include <QLabel>
#include <QVBoxLayout>


SegmentationFilterWidget::SegmentationFilterWidget(ThresholdFilter& thresholdFilter) :
        VLayout(new QVBoxLayout(this)),
        FilterWidget(new ThresholdFilterWidget()),
        SegmentationFilter(&thresholdFilter) {

    VLayout->setContentsMargins({});

    auto* titleLabel = new QLabel("Segmentation Filter");
    titleLabel->setStyleSheet(GetHeader1StyleSheet());
    titleLabel->setContentsMargins(0, 0, 0, 11);
    VLayout->addWidget(titleLabel);
    VLayout->addWidget(FilterWidget);
    VLayout->addStretch();

    FilterWidget->Populate(dynamic_cast<ThresholdFilter&>(*SegmentationFilter));
    FilterWidget->SetFilterData(dynamic_cast<ThresholdFilter&>(*SegmentationFilter));

    connect(FilterWidget, &ThresholdFilterWidget::DataChanged, this, [this]() {
        FilterWidget->SetFilterData(dynamic_cast<ThresholdFilter&>(*SegmentationFilter));
    });
}

SegmentationFilterWidget::~SegmentationFilterWidget() = default;

auto SegmentationFilterWidget::GetFilter() const -> vtkImageAlgorithm& {
    if (!SegmentationFilter)
        throw std::runtime_error("Segmentation filter must not be null");

    return *SegmentationFilter;
}

void SegmentationFilterWidget::showEvent(QShowEvent* event) {
    FilterWidget->Populate(dynamic_cast<ThresholdFilter&>(*SegmentationFilter));

    QWidget::showEvent(event);
}
