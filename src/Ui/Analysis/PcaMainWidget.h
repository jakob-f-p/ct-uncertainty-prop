#pragma once

#include <QMainWindow>

class PipelineGroupList;


class PcaMainWidget : public QMainWindow {
public:
    PcaMainWidget(PipelineGroupList const& pipelineGroups) {};

    auto
    UpdateData() -> void {};
};
