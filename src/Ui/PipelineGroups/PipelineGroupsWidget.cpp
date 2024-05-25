#include "PipelineGroupsWidget.h"

#include "PipelineGroupListWidget.h"
#include "PipelineGroupWidget.h"
#include "PipelineParameterSpanWidget.h"

#include "../../PipelineGroups/ArtifactVariantPointer.h"
#include "../../PipelineGroups/PipelineGroup.h"

#include <QHBoxLayout>

PipelineGroupsWidget::PipelineGroupsWidget(PipelineGroupList& pipelineGroups) :
        GroupListWidget(new PipelineGroupListWidget(pipelineGroups)),
        GroupWidget(new OptionalWidget<PipelineGroupWidget>("Select a pipeline group")),
        ParameterSpanWidget(new OptionalWidget<PipelineParameterSpanWidget>("Select a parameter span")) {

    auto* centralWidget = new QWidget();

    auto* hLayout = new QHBoxLayout(centralWidget);
    hLayout->addWidget(GroupListWidget);
    hLayout->addWidget(GroupWidget);
    hLayout->addWidget(ParameterSpanWidget);

    setCentralWidget(centralWidget);

    connect(GroupListWidget, &PipelineGroupListWidget::PipelineGroupChanged,
            this, [&](PipelineGroup* pipelineGroup) { OnPipelineGroupChanged(pipelineGroup); });
}

void PipelineGroupsWidget::UpdatePipelineList() noexcept {
    GroupListWidget->UpdatePipelineList();
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

                GroupWidget->Widget().AddParameterSpan(
                        createWidget->GetPipelineParameterSpan());
            });

    connect(createWidget, &PipelineParameterSpanCreateWidget::Reject,
            this, [this]() { ParameterSpanWidget->HideWidget(); });
}
