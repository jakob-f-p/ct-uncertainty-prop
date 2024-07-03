#include "AnalysisWidget.h"

#include "AnalysisMainWidget.h"


AnalysisWidget::AnalysisWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        TsneWidget(new TsneMainWidget(pipelineGroups, dataSource)),
        PcaWidget(new PcaMainWidget(pipelineGroups, dataSource)) {

    addTab(TsneWidget, "t-SNE");
    addTab(PcaWidget, "PCA");

    connect(TsneWidget, &TsneMainWidget::PcaPointsSelected,
            this, [this](QString const& pointSetName, QList<QPointF> const& points) {
        PcaWidget->SelectPcaPoints(pointSetName, points);
        setCurrentWidget(PcaWidget);
    });
}

auto AnalysisWidget::UpdateData() -> void {
    TsneWidget->UpdateData();
    PcaWidget->UpdateData();
}
