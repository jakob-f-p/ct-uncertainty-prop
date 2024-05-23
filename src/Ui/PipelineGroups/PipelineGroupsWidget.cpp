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

    connect(GroupListWidget, &PipelineGroupListWidget::PipelineGroupChanged, this, [&](PipelineGroup* pipelineGroup) {
        if (!pipelineGroup) {
            GroupWidget->HideWidget();
            return;
        }

        GroupWidget->UpdateWidget(new PipelineGroupWidget(*pipelineGroup));

        connect(&GroupWidget->Widget(), &PipelineGroupWidget::ParameterSpanChanged,
                this, [&, pipelineGroup](PipelineParameterSpan* parameterSpan,
                        ArtifactVariantPointer artifactVariantPointer) {
            if (!parameterSpan) {
                ParameterSpanWidget->HideWidget();
                return;
            }

            ParameterSpanWidget->UpdateWidget(new PipelineParameterSpanReadOnlyWidget(pipelineGroup->GetBasePipeline(),
                                                                                      *parameterSpan,
                                                                                      artifactVariantPointer));
        });

        connect(&GroupWidget->Widget(), &PipelineGroupWidget::RequestCreateParameterSpan,
                this, [&, pipelineGroup]() {
            ParameterSpanWidget->UpdateWidget(new PipelineParameterSpanCreateWidget(pipelineGroup->GetBasePipeline()));
        });
    });
}

void PipelineGroupsWidget::UpdatePipelineList() noexcept {
    GroupListWidget->UpdatePipelineList();
}
