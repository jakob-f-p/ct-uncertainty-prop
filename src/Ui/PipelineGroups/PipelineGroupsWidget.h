#pragma once

#include "../Utils/OptionalWidget.h"

#include <QSplitter>

class PipelineGroup;
class PipelineGroupList;
class PipelineGroupListWidget;
class PipelineGroupWidget;
class PipelineParameterSpan;
class PipelineParameterSpanWidget;

class QPushButton;
class QProgressBar;


class PipelineGroupsWidget : public QSplitter {
public:
    explicit PipelineGroupsWidget(PipelineGroupList& pipelineGroups);

public Q_SLOTS:
    void UpdatePipelineList() const noexcept;

protected:
    void hideEvent(QHideEvent* event) override;

private Q_SLOTS:
    void OnPipelineGroupChanged(PipelineGroup* pipelineGroup);
    void OnParameterSpanChanged(PipelineGroup const* pipelineGroup, PipelineParameterSpan const* parameterSpan) const;
    void OnRequestCreateParameterSpan(PipelineGroup const* pipelineGroup);

private:
    PipelineGroupListWidget* GroupListWidget;
    OptionalWidget<PipelineGroupWidget>* GroupWidget;
    OptionalWidget<PipelineParameterSpanWidget>* ParameterSpanWidget;
};
