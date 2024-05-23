#pragma once

#include "../Utils/OptionalWidget.h"

#include <QMainWindow>

class PipelineGroupList;
class PipelineGroupListWidget;
class PipelineGroupWidget;
class PipelineParameterSpanWidget;

class PipelineGroupsWidget : public QMainWindow {
public:
    explicit PipelineGroupsWidget(PipelineGroupList& pipelineGroups);

public slots:
    void UpdatePipelineList() noexcept;

private:
    PipelineGroupListWidget* GroupListWidget;
    OptionalWidget<PipelineGroupWidget>* GroupWidget;
    OptionalWidget<PipelineParameterSpanWidget>* ParameterSpanWidget;
};
