#include "PipelineGroupsWidget.h"

#include "PipelineGroupListWidget.h"
#include "PipelineGroupWidget.h"
#include "PipelineParameterSpanWidget.h"

#include "../../PipelineGroups/PipelineGroup.h"
#include "../../PipelineGroups/PipelineParameterSpace.h"

#include <QApplication>
#include <QHBoxLayout>


PipelineGroupsWidget::PipelineGroupsWidget(PipelineGroupList& pipelineGroups) :
        GroupListWidget(new PipelineGroupListWidget(pipelineGroups)),
        GroupWidget(new OptionalWidget<PipelineGroupWidget>("Select a pipeline group")),
        ParameterSpanWidget(new OptionalWidget<PipelineParameterSpanWidget>("Select a parameter span")) {

    setContentsMargins(10, 10, 10, 10);

    setHandleWidth(10);

    addWidget(GroupListWidget);
    addWidget(GroupWidget);
    addWidget(ParameterSpanWidget);

    auto modifiedPalette = QApplication::palette();
    auto const darkerBackgroundColor = modifiedPalette.color(QPalette::ColorRole::Window).darker();
    modifiedPalette.setColor(QPalette::ColorRole::Window, darkerBackgroundColor);
    modifiedPalette.setColor(QPalette::ColorRole::Base, darkerBackgroundColor);
    modifiedPalette.setColor(QPalette::ColorRole::ToolTipBase, darkerBackgroundColor);
    setPalette(modifiedPalette);
    setBackgroundRole(QPalette::ColorRole::Shadow);
    setAutoFillBackground(true);

    for (int i = 0; i < count(); ++i) {
        auto* frame = dynamic_cast<QFrame*>(widget(i));
        frame->setFrameShape(StyledPanel);
        frame->setFrameShadow(Sunken);
        frame->setBackgroundRole(QPalette::ColorRole::Window);
        frame->setAutoFillBackground(true);
    }

    QList const widths = { GroupListWidget->sizeHint().width(),
                                GroupWidget->sizeHint().width(),
                                ParameterSpanWidget->sizeHint().width() };
    int const maxWidth = *std::ranges::max_element(widths);

    GroupWidget->setBaseSize(maxWidth, GroupWidget->baseSize().height());
    ParameterSpanWidget->setBaseSize(maxWidth, ParameterSpanWidget->baseSize().height());

    connect(GroupListWidget, &PipelineGroupListWidget::PipelineGroupChanged,
            this, [&](PipelineGroup* pipelineGroup) { OnPipelineGroupChanged(pipelineGroup); });
}

auto PipelineGroupsWidget::UpdatePipelineList() const noexcept -> void {
    GroupListWidget->UpdatePipelineList();
}

auto PipelineGroupsWidget::hideEvent(QHideEvent* /*event*/) -> void {
    GroupWidget->HideWidget();
    ParameterSpanWidget->HideWidget();
}

auto PipelineGroupsWidget::OnPipelineGroupChanged(PipelineGroup* pipelineGroup) -> void {
    if (!pipelineGroup) {
        GroupWidget->HideWidget();
        return;
    }

    GroupWidget->UpdateWidget(new PipelineGroupWidget(*pipelineGroup));
    ParameterSpanWidget->HideWidget();

    connect(&GroupWidget->Widget(), &PipelineGroupWidget::ParameterSpanChanged,
            this, [this, pipelineGroup](PipelineParameterSpan* span) { OnParameterSpanChanged(pipelineGroup, span); });

    connect(&GroupWidget->Widget(), &PipelineGroupWidget::RequestCreateParameterSpan,
            this, [this, pipelineGroup] { OnRequestCreateParameterSpan(pipelineGroup); });

    connect(&GroupWidget->Widget(), &PipelineGroupWidget::NumberOfPipelinesUpdated,
            GroupListWidget, &PipelineGroupListWidget::UpdateNumberOfPipelines);
}

auto PipelineGroupsWidget::OnParameterSpanChanged(PipelineGroup const* pipelineGroup,
                                                  PipelineParameterSpan const* parameterSpan) const -> void {
    if (!parameterSpan) {
        ParameterSpanWidget->HideWidget();
        return;
    }

    ParameterSpanWidget->UpdateWidget(
            new PipelineParameterSpanReadOnlyWidget(pipelineGroup->GetBasePipeline(), *parameterSpan));
}

void PipelineGroupsWidget::OnRequestCreateParameterSpan(PipelineGroup const* pipelineGroup) {
    auto* createWidget = new PipelineParameterSpanCreateWidget(pipelineGroup->GetBasePipeline());
    ParameterSpanWidget->UpdateWidget(createWidget);

    connect(createWidget, &PipelineParameterSpanCreateWidget::Accept,
            this, [this, createWidget] {
                GroupWidget->Widget().AddParameterSpan(createWidget->GetPipelineParameterSpan());
            });

    connect(createWidget, &PipelineParameterSpanCreateWidget::Reject,
            this, [this] { ParameterSpanWidget->HideWidget(); });
}
