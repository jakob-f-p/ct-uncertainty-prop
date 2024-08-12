#include "PipelineGroupsWidget.h"

#include "PipelineGroupListWidget.h"
#include "PipelineGroupWidget.h"
#include "PipelineParameterSpanWidget.h"

#include "../../PipelineGroups/PipelineGroup.h"
#include "../../PipelineGroups/PipelineParameterSpace.h"

#include <QHBoxLayout>

PipelineGroupsWidget::PipelineGroupsWidget(PipelineGroupList& pipelineGroups) :
        GroupListWidget(new PipelineGroupListWidget(pipelineGroups)),
        GroupWidget(new OptionalWidget<PipelineGroupWidget>("Select a pipeline group")),
        ParameterSpanWidget(new OptionalWidget<PipelineParameterSpanWidget>("Select a parameter span")) {

    auto* splitter = new QSplitter();
    splitter->addWidget(GroupListWidget);
    splitter->addWidget(GroupWidget);
    splitter->addWidget(ParameterSpanWidget);

    QList<int> const widths = { GroupListWidget->sizeHint().width(),
                                GroupWidget->sizeHint().width(),
                                ParameterSpanWidget->sizeHint().width() };
    int const maxWidth = *std::max_element(widths.begin(), widths.end());
//    QList<int> const equalWidths { 3, maxWidth };
//    splitter->setSizes(equalWidths);

    GroupWidget->setBaseSize(maxWidth, GroupWidget->baseSize().height());
    ParameterSpanWidget->setBaseSize(maxWidth, ParameterSpanWidget->baseSize().height());

    setCentralWidget(splitter);

    connect(GroupListWidget, &PipelineGroupListWidget::PipelineGroupChanged,
            this, [&](PipelineGroup* pipelineGroup) { OnPipelineGroupChanged(pipelineGroup); });
}

void PipelineGroupsWidget::UpdatePipelineList() noexcept {
    GroupListWidget->UpdatePipelineList();
}

void PipelineGroupsWidget::hideEvent(QHideEvent* /*event*/) {
    GroupWidget->HideWidget();
    ParameterSpanWidget->HideWidget();
}

void PipelineGroupsWidget::OnPipelineGroupChanged(PipelineGroup* pipelineGroup) {
    if (!pipelineGroup) {
        GroupWidget->HideWidget();
        return;
    }

    GroupWidget->UpdateWidget(new PipelineGroupWidget(*pipelineGroup));
    ParameterSpanWidget->HideWidget();

    connect(&GroupWidget->Widget(), &PipelineGroupWidget::ParameterSpanChanged,
            this, [this, pipelineGroup](PipelineParameterSpan* span) { OnParameterSpanChanged(pipelineGroup, span); });

    connect(&GroupWidget->Widget(), &PipelineGroupWidget::RequestCreateParameterSpan,
            this, [this, pipelineGroup]() { OnRequestCreateParameterSpan(pipelineGroup); });

    connect(&GroupWidget->Widget(), &PipelineGroupWidget::NumberOfPipelinesUpdated,
            GroupListWidget, &PipelineGroupListWidget::UpdateNumberOfPipelines);
}

void PipelineGroupsWidget::OnParameterSpanChanged(PipelineGroup* pipelineGroup, PipelineParameterSpan* parameterSpan) {
    if (!parameterSpan) {
        ParameterSpanWidget->HideWidget();
        return;
    }

    ParameterSpanWidget->UpdateWidget(
            new PipelineParameterSpanReadOnlyWidget(pipelineGroup->GetBasePipeline(), *parameterSpan));
}

void PipelineGroupsWidget::OnRequestCreateParameterSpan(PipelineGroup* pipelineGroup) {
    auto* createWidget = new PipelineParameterSpanCreateWidget(pipelineGroup->GetBasePipeline());
    ParameterSpanWidget->UpdateWidget(createWidget);

    connect(createWidget, &PipelineParameterSpanCreateWidget::Accept,
            this, [this, createWidget]() {
                GroupWidget->Widget().AddParameterSpan(createWidget->GetPipelineParameterSpan());
            });

    connect(createWidget, &PipelineParameterSpanCreateWidget::Reject,
            this, [this]() { ParameterSpanWidget->HideWidget(); });
}
