#include "AnalysisWidget.h"

#include "PcaMainWidget.h"
#include "TsneMainWidget.h"

AnalysisWidget::AnalysisWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        TsneWidget(new TsneMainWidget(pipelineGroups, dataSource)),
        PcaWidget(new PcaMainWidget(pipelineGroups)) {

    addTab(TsneWidget, "t-SNE");
    addTab(PcaWidget, "PCA");
}

auto AnalysisWidget::UpdateData() -> void {
    TsneWidget->UpdateData();
    PcaWidget->UpdateData();
}
