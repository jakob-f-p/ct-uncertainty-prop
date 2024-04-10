#include "PipelinesWidget.h"

#include "ArtifactsDialog.h"
#include "ImageArtifactsModel.h"
#include "ImageArtifactsView.h"
#include "ImageArtifactsWidget.h"
#include "StructureArtifactsWidget.h"
#include "../Pipeline.h"
#include "../PipelineList.h"
#include "../../App.h"

#include <vtkCallbackCommand.h>
#include <vtkNew.h>

PipelinesWidget::PipelinesWidget() :
        Pipelines(App::GetInstance()->GetPipelines()),
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
    vtkNew<vtkCallbackCommand> pipelineListObserver;
    pipelineListObserver->SetClientData(StructureArtifactModelingWidget);
    pipelineListObserver->SetCallback([](vtkObject*, unsigned long, void* clientData, void*) {
        auto* structureArtifactModelingWidget = static_cast<StructureArtifactsWidget*>(clientData);
        structureArtifactModelingWidget->ResetModel();
    });
    Pipelines.AddObserver(vtkCommand::ModifiedEvent, pipelineListObserver);

    vLayout->addWidget(ImageArtifactModelingWidget);

    InitializeViews();
    UpdatePipelineView();
}

void PipelinesWidget::AddPipeline() {
    vtkNew<Pipeline> newPipeline;
    newPipeline->SetCtDataTree(&App::GetInstance()->GetCtDataTree());

    Pipelines.AddPipeline(newPipeline);

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
    if (CurrentPipelineIndex == 0) {
        qWarning("Cannot decrease pipeline index further");
        return;
    }

    CurrentPipelineIndex--;
    UpdatePipelineView();
}

void PipelinesWidget::NextPipeline() {
    if (CurrentPipelineIndex == Pipelines.GetSize() - 1) {
        qWarning("Cannot increase pipeline index further");
        return;
    }

    CurrentPipelineIndex++;
    UpdatePipelineView();
}

void PipelinesWidget::UpdatePipelineView() {
    QString pipelineTitleString = "";
    if (GetCurrentPipeline()) {
        std::string pipelineName = GetCurrentPipeline()->GetName();
        pipelineTitleString = pipelineName.empty()
                               ? QString::fromStdString("Pipeline " + std::to_string(CurrentPipelineIndex + 1))
                               : QString::fromStdString(pipelineName);
    }
    PipelineTitle->setText(pipelineTitleString);

    PreviousPipelineButton->setEnabled(CurrentPipelineIndex > 0);
    NextPipelineButton->setEnabled(CurrentPipelineIndex < Pipelines.GetSize() - 1);
    AddPipelineButton->setEnabled(Pipelines.GetSize() < 10);
    RemovePipelineButton->setEnabled(CurrentPipelineIndex > -1);

    StructureArtifactModelingWidget->SetCurrentView(CurrentPipelineIndex);
    ImageArtifactModelingWidget->SetCurrentView(CurrentPipelineIndex);
}

QIcon PipelinesWidget::GenerateIcon(const std::string &filePrefix) {
    QIcon icon;
    QString qFilePrefix = QString::fromStdString(filePrefix);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Normal.png"), QIcon::Normal);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Disabled.png"), QIcon::Disabled);
    return icon;
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

Pipeline* PipelinesWidget::GetCurrentPipeline() {
    return Pipelines.Get(CurrentPipelineIndex);
}

QString PipelinesWidget::GetHeaderStyleSheet() {
    return "font-size: 14px; font-weight: bold";
}
