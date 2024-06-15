#include "PipelineGroupsWidget.h"

#include "PipelineGroupListWidget.h"
#include "PipelineGroupWidget.h"
#include "PipelineParameterSpanWidget.h"

#include "../../PipelineGroups/ArtifactVariantPointer.h"
#include "../../PipelineGroups/PipelineGroup.h"
#include "../../PipelineGroups/PipelineGroupList.h"
#include "../../PipelineGroups/PipelineParameterSpace.h"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QPushButton>

#include <mutex>

PipelineGroupsWidget::PipelineGroupsWidget(PipelineGroupList& pipelineGroups) :
        GroupListWidget(new PipelineGroupListWidget(pipelineGroups)),
        GroupWidget(new OptionalWidget<PipelineGroupWidget>("Select a pipeline group")),
        ParameterSpanWidget(new OptionalWidget<PipelineParameterSpanWidget>("Select a parameter span")),
        CalculateImagesButton(new QPushButton("Generate Image Data")),
        CalculateImagesProgressBar([]() {
            auto* progressBar = new QProgressBar();
            progressBar->setRange(0, 1000);
            return progressBar;
        }()) {

    auto* centralWidget = new QWidget();
    auto* vLayout = new QVBoxLayout(centralWidget);

    auto* parameterSpaceWidget = new QWidget();
    auto* parameterSpaceEditHLayout = new QHBoxLayout(parameterSpaceWidget);
    parameterSpaceEditHLayout->addWidget(GroupListWidget);
    parameterSpaceEditHLayout->addWidget(GroupWidget);
    parameterSpaceEditHLayout->addWidget(ParameterSpanWidget);
    vLayout->addWidget(parameterSpaceWidget);

    auto* renderBarWidget = new QWidget();
    auto* renderBarHLayout = new QHBoxLayout(renderBarWidget);
    renderBarHLayout->addWidget(CalculateImagesButton);
    renderBarHLayout->addWidget(CalculateImagesProgressBar);
    vLayout->addWidget(renderBarWidget);

    setCentralWidget(centralWidget);

    connect(GroupListWidget, &PipelineGroupListWidget::PipelineGroupChanged,
            this, [&](PipelineGroup* pipelineGroup) { OnPipelineGroupChanged(pipelineGroup); });

    connect(CalculateImagesButton, &QPushButton::clicked, this, [this, &pipelineGroups]() {
        CalculateImagesButton->setEnabled(false);

        std::mutex progressMutex;
        auto progressCallback = [this, &progressMutex](double progress) {
            std::scoped_lock<std::mutex> const lock (progressMutex);
            CalculateImagesProgressBar->setValue(static_cast<int>(progress * 1000.0));
        };

        CalculateImagesProgressBar->setFormat("Generating Image Data (Task 1/5)... %p%");
        pipelineGroups.GenerateImages(progressCallback);

        CalculateImagesProgressBar->setFormat("Exporting Images... (Task 2/5) %p%");
        pipelineGroups.ExportImages(progressCallback);

        CalculateImagesProgressBar->setFormat("Extracting Features... (Task 3/5) %p%");
        pipelineGroups.ExtractFeatures(progressCallback);

        CalculateImagesProgressBar->setFormat("Doing PCA... (Task 4/5) %p%");
        pipelineGroups.DoPCAs(2, progressCallback);

        CalculateImagesProgressBar->setFormat("Doing t-SNE... (Task 5/5) %p%");
        pipelineGroups.DoTsne(2, progressCallback);

        CalculateImagesProgressBar->reset();
        CalculateImagesButton->setEnabled(true);
    });
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
