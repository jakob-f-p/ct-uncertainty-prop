#include "PipelinesWidget.h"

#include "Image/ImageArtifactsWidget.h"
#include "Structure/StructureArtifactsWidget.h"
#include "../Utils/WidgetUtils.h"
#include "../../Artifacts/Pipeline.h"
#include "../../Artifacts/PipelineList.h"

#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QStackedLayout>

PipelinesWidget::PipelinesWidget(PipelineList& pipelines, QWidget* parent) :
        QWidget(parent),
        Pipelines(pipelines),
        CurrentPipelineIndex(0),
        PipelineTitle(new QLabel()),
        PreviousPipelineButton(new QPushButton(GenerateIcon("ArrowLeft"), "")),
        NextPipelineButton(new QPushButton(GenerateIcon("ArrowRight"), "")),
        AddPipelineButton(new QPushButton(GenerateIcon("Plus"), "")),
        RemovePipelineButton(new QPushButton(GenerateIcon("Minus"), "")),
        StructureArtifactModelingWidget(new StructureArtifactsWidget()),
        ImageArtifactModelingWidget(new ImageArtifactsWidget()) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(10);

    auto* pipelineTitleBarWidget = new QWidget();
    auto* pipelineTitleBarHLayout = new QHBoxLayout(pipelineTitleBarWidget);
    pipelineTitleBarHLayout->setContentsMargins(0, 11, 0, 0);
    PipelineTitle->setStyleSheet(GetHeader1StyleSheet());
    pipelineTitleBarHLayout->addWidget(PipelineTitle);
    pipelineTitleBarHLayout->addStretch();
    connect(AddPipelineButton, &QPushButton::clicked, this, &PipelinesWidget::AddPipeline);
    connect(RemovePipelineButton, &QPushButton::clicked, this, &PipelinesWidget::RemovePipeline);
    connect(PreviousPipelineButton, &QPushButton::clicked, this, &PipelinesWidget::PreviousPipeline);
    connect(NextPipelineButton, &QPushButton::clicked, this, &PipelinesWidget::NextPipeline);
    pipelineTitleBarHLayout->addWidget(PreviousPipelineButton);
    pipelineTitleBarHLayout->addWidget(NextPipelineButton);
    pipelineTitleBarHLayout->addWidget(AddPipelineButton);
    pipelineTitleBarHLayout->addWidget(RemovePipelineButton);
    vLayout->addWidget(pipelineTitleBarWidget);

    vLayout->addWidget(StructureArtifactModelingWidget);

    Pipelines.AddTreeEventCallback(
            [widget = StructureArtifactModelingWidget]() { widget->ResetModel(); });

    vLayout->addWidget(ImageArtifactModelingWidget);

    vLayout->addStretch();

    InitializeViews();
    UpdatePipelineView();

    pipelines.AddPipelineEventCallback([this](PipelineEvent const& event) {
        if (event.Type == PipelineEventType::POST_ADD) {
            StructureArtifactModelingWidget->AddView(*event.PipelinePointer);
            ImageArtifactModelingWidget->AddView(*event.PipelinePointer);

            UpdatePipelineView();
        }
    });
}

void PipelinesWidget::AddPipeline() {
    Pipelines.AddPipeline();

    CurrentPipelineIndex = Pipelines.GetSize() - 1;

    StructureArtifactModelingWidget->AddView(GetCurrentPipeline());
    ImageArtifactModelingWidget->AddView(GetCurrentPipeline());

    UpdatePipelineView();
}

void PipelinesWidget::RemovePipeline() {
    Pipelines.RemovePipeline(GetCurrentPipeline());

    StructureArtifactModelingWidget->RemoveCurrentView();
    ImageArtifactModelingWidget->RemoveCurrentView();

    if (Pipelines.IsEmpty())
        CurrentPipelineIndex = -1;
    else if (CurrentPipelineIndex == Pipelines.GetSize())
        CurrentPipelineIndex--;

    UpdatePipelineView();
}

void PipelinesWidget::PreviousPipeline() {
    if (CurrentPipelineIndex == 0)
        throw std::runtime_error("Cannot decrease pipeline index further");

    CurrentPipelineIndex--;
    UpdatePipelineView();
}

void PipelinesWidget::NextPipeline() {
    if (CurrentPipelineIndex == Pipelines.GetSize() - 1)
        throw std::runtime_error("Cannot increase pipeline index further");

    CurrentPipelineIndex++;
    UpdatePipelineView();
}

void PipelinesWidget::UpdatePipelineView() {
    std::string const pipelineName = GetCurrentPipeline().GetName();
    QString const pipelineTitleString = QString::fromStdString(pipelineName);
    PipelineTitle->setText(pipelineTitleString);

    PreviousPipelineButton->setEnabled(CurrentPipelineIndex > 0);
    NextPipelineButton->setEnabled(CurrentPipelineIndex < Pipelines.GetSize() - 1);
    AddPipelineButton->setEnabled(Pipelines.GetSize() < 10);
    RemovePipelineButton->setEnabled(CurrentPipelineIndex > -1);

    StructureArtifactModelingWidget->SetCurrentView(CurrentPipelineIndex);
    ImageArtifactModelingWidget->SetCurrentView(CurrentPipelineIndex);

    Q_EMIT PipelineViewUpdated(GetCurrentPipeline());
}

void PipelinesWidget::InitializeViews() {
    if (Pipelines.IsEmpty())
        Pipelines.AddPipeline();

    for (int i = 0; i < Pipelines.GetSize(); ++i) {
        StructureArtifactModelingWidget->AddView(Pipelines.Get(i));
        ImageArtifactModelingWidget->AddView(Pipelines.Get(i));
    }
}

auto PipelinesWidget::GetCurrentPipeline() -> Pipeline& {
    return Pipelines.Get(CurrentPipelineIndex);
}
