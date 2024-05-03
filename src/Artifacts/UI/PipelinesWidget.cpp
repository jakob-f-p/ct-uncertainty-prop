#include "PipelinesWidget.h"

#include "ImageArtifactsWidget.h"
#include "StructureArtifactsWidget.h"
#include "../Pipeline.h"
#include "../PipelineList.h"

#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QStackedLayout>

PipelinesWidget::PipelinesWidget(PipelineList& pipelines, QWidget* parent) :
        QWidget(parent),
        Pipelines(pipelines),
        CurrentPipelineIndex(Pipelines.IsEmpty() ? -1 : 0),
        PipelineTitle(new QLabel()),
        PreviousPipelineButton(new QPushButton("")),
        NextPipelineButton(new QPushButton("")),
        AddPipelineButton(new QPushButton("")),
        RemovePipelineButton(new QPushButton("")),
        StructureArtifactModelingWidget(new StructureArtifactsWidget()),
        ImageArtifactModelingWidget(new ImageArtifactsWidget()) {

    auto* vLayout = new QVBoxLayout(this);

    auto* pipelineTitleBarWidget = new QWidget();
    auto* pipelineTitleBarHLayout = new QHBoxLayout(pipelineTitleBarWidget);
    pipelineTitleBarHLayout->setContentsMargins(0, 11, 0, 0);
    PipelineTitle->setStyleSheet(GetHeaderStyleSheet());
    pipelineTitleBarHLayout->addWidget(PipelineTitle);
    pipelineTitleBarHLayout->addStretch();
    PreviousPipelineButton->setIcon(GenerateIcon("ArrowLeft"));
    NextPipelineButton->setIcon(GenerateIcon("ArrowRight"));
    AddPipelineButton->setIcon(GenerateIcon("Plus"));
    RemovePipelineButton->setIcon(GenerateIcon("Minus"));
    pipelineTitleBarHLayout->addWidget(PreviousPipelineButton);
    pipelineTitleBarHLayout->addWidget(NextPipelineButton);
    pipelineTitleBarHLayout->addWidget(AddPipelineButton);
    pipelineTitleBarHLayout->addWidget(RemovePipelineButton);
    vLayout->addWidget(pipelineTitleBarWidget);
    connect(AddPipelineButton, &QPushButton::clicked, this, &PipelinesWidget::AddPipeline);
    connect(RemovePipelineButton, &QPushButton::clicked, this, &PipelinesWidget::RemovePipeline);
    connect(PreviousPipelineButton, &QPushButton::clicked, this, &PipelinesWidget::PreviousPipeline);
    connect(NextPipelineButton, &QPushButton::clicked, this, &PipelinesWidget::NextPipeline);

    vLayout->addWidget(StructureArtifactModelingWidget);

    Pipelines.AddPipelineEventCallback([widget = StructureArtifactModelingWidget]() { widget->ResetModel(); });

    vLayout->addWidget(ImageArtifactModelingWidget);

    InitializeViews();
    UpdatePipelineView();
}

void PipelinesWidget::AddPipeline() {
    Pipelines.AddPipeline();

    CurrentPipelineIndex = Pipelines.GetSize() - 1;

    CreateArtifactsViewsForCurrentPipeline();

    UpdatePipelineView();
}

void PipelinesWidget::RemovePipeline() {
    Pipelines.RemovePipeline(GetCurrentPipeline());

    StructureArtifactModelingWidget->RemoveCurrentView();
    ImageArtifactModelingWidget->RemoveCurrentView();

    if (Pipelines.IsEmpty()) {
        CurrentPipelineIndex = -1;
    } else if (CurrentPipelineIndex == Pipelines.GetSize()) {
        CurrentPipelineIndex--;
    }

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
    QString const pipelineTitleString = pipelineName.empty()
                           ? QString::fromStdString("Pipeline " + std::to_string(CurrentPipelineIndex + 1))
                           : QString::fromStdString(pipelineName);
    PipelineTitle->setText(pipelineTitleString);

    PreviousPipelineButton->setEnabled(CurrentPipelineIndex > 0);
    NextPipelineButton->setEnabled(CurrentPipelineIndex < Pipelines.GetSize() - 1);
    AddPipelineButton->setEnabled(Pipelines.GetSize() < 10);
    RemovePipelineButton->setEnabled(CurrentPipelineIndex > -1);

    StructureArtifactModelingWidget->SetCurrentView(CurrentPipelineIndex);
    ImageArtifactModelingWidget->SetCurrentView(CurrentPipelineIndex);

    emit PipelineViewUpdated(GetCurrentPipeline());
}

auto PipelinesWidget::GenerateIcon(const std::string &filePrefix) noexcept -> QIcon {
    QIcon icon;
    QString qFilePrefix = QString::fromStdString(filePrefix);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Normal.png"), QIcon::Normal);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Disabled.png"), QIcon::Disabled);
    return icon;
}

auto PipelinesWidget::GetHeaderStyleSheet() noexcept -> QString {
    return "font-size: 14px; font-weight: bold";
}

void PipelinesWidget::InitializeViews() {
    for (int i = 0; i < Pipelines.GetSize(); ++i) {
        StructureArtifactModelingWidget->AddView(Pipelines.Get(i));
        ImageArtifactModelingWidget->AddView(Pipelines.Get(i));
    }
}

void PipelinesWidget::CreateArtifactsViewsForCurrentPipeline() {
    StructureArtifactModelingWidget->AddView(GetCurrentPipeline());
    ImageArtifactModelingWidget->AddView(GetCurrentPipeline());
}

auto PipelinesWidget::GetCurrentPipeline() -> Pipeline& {
    return Pipelines.Get(CurrentPipelineIndex);
}
