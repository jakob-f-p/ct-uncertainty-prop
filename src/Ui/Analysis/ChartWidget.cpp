#include "ChartWidget.h"

#include "MainChartWidget.h"
#include "PcaSecondaryChartWidget.h"


ChartWidget::ChartWidget(QWidget* parent) noexcept :
        QSplitter(parent) {

    setOrientation(Qt::Orientation::Horizontal);
    setContentsMargins({});
}



PcaChartWidget::PcaChartWidget(QWidget* parent) noexcept :
        ChartWidget(parent),
        PcaMainWidget(new PcaMainChartWidget()),
        PcaSecondaryWidget(new PcaSecondaryChartWidget()) {

    connect(PcaMainWidget, &PcaMainChartWidget::SamplePointChanged,
            this, &PcaChartWidget::SamplePointChanged);
    connect(PcaMainWidget, &PcaMainChartWidget::PcaDataChanged,
            PcaSecondaryWidget, &PcaSecondaryChartWidget::UpdateData);

    addWidget(PcaMainWidget);
    addWidget(PcaSecondaryWidget);

    PcaMainWidget->setContentsMargins(0, 0, 5, 0);
    PcaSecondaryWidget->setContentsMargins(5, 0, 0, 0);
}

auto PcaChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    PcaMainWidget->UpdateData(batchListData);
    PcaSecondaryWidget->UpdateData(batchListData);
}

auto PcaChartWidget::SelectPcaPoints(QString const& name, QList<QPointF> const& points) const -> void {
    PcaMainWidget->SelectPcaPoints(name, points);
}


TsneChartWidget::TsneChartWidget(QWidget* parent) noexcept :
        ChartWidget(parent),
        TsneWidget(new TsneMainChartWidget()) {

    connect(TsneWidget, &TsneMainChartWidget::SamplePointChanged, this, &TsneChartWidget::SamplePointChanged);
    connect(TsneWidget, &TsneMainChartWidget::PcaPointsSelected, this, &TsneChartWidget::PcaPointsSelected);

    addWidget(TsneWidget);
}

auto TsneChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    TsneWidget->UpdateData(batchListData);
}
