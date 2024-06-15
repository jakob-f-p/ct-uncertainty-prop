#pragma once

#include <QTabWidget>

class CtDataSource;
class PcaMainWidget;
class PipelineGroupList;
class TsneMainWidget;

class AnalysisWidget : public QTabWidget {
public:
    explicit AnalysisWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource);

    auto
    UpdateData() -> void;

private:
    TsneMainWidget* TsneWidget;
    PcaMainWidget* PcaWidget;
};
