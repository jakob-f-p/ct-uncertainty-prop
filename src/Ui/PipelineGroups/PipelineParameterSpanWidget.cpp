#include "PipelineParameterSpanWidget.h"

#include "ObjectPropertyWidget.h"
#include "../Modeling/CtStructureView.h"
#include "../Artifacts/Image/ImageArtifactsView.h"
#include "../Artifacts/Structure/StructureArtifactsView.h"
#include "../Utils/CoordinateRowWidget.h"
#include "../Utils/NameLineEdit.h"
#include "../../Artifacts/Pipeline.h"
#include "../../Modeling/CtStructureTree.h"
#include "../../Utils/Types.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>

PipelineParameterSpanWidget::PipelineParameterSpanWidget(Pipeline const& pipeline, QWidget* parent) :
        QWidget(parent),
        FLayout(new QFormLayout(this)),
        NameEdit(new NameLineEdit()),
        NumberOfPipelinesSpinBox(new QSpinBox()),
        ArtifactsView(new PipelineArtifactsView(pipeline)),
        PropertyGroup(new OptionalWidget<ObjectPropertyGroup>("Select an artifact")) {

    FLayout->addRow("Name", NameEdit);

    FLayout->addRow("Number of Pipelines", NumberOfPipelinesSpinBox);
    NumberOfPipelinesSpinBox->setEnabled(false);

    FLayout->addWidget(ArtifactsView);

    FLayout->addWidget(PropertyGroup);

    connect(ArtifactsView, &PipelineArtifactsView::ArtifactChanged,
            this, [this](ArtifactVariantPointer artifactVariantPointer) {

        if (artifactVariantPointer.IsNullptr()) {
            PropertyGroup->HideWidget();
            return;
        }

        PropertyGroup->UpdateWidget(new ObjectPropertyGroup(artifactVariantPointer));
    });
}

PipelineParameterSpanCreateWidget::PipelineParameterSpanCreateWidget(Pipeline const& pipeline, QWidget* parent) :
        PipelineParameterSpanWidget(pipeline, parent),
        DialogButtonBox([]() {
            auto* dialogButtonBar = new QDialogButtonBox();
            dialogButtonBar->setOrientation(Qt::Horizontal);
            dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
            return dialogButtonBar;
        }()) {

    FLayout->addWidget(DialogButtonBox);

    connect(DialogButtonBox, &QDialogButtonBox::accepted, this, &PipelineParameterSpanCreateWidget::Accept);
    connect(DialogButtonBox, &QDialogButtonBox::rejected, this, &PipelineParameterSpanCreateWidget::Reject);
}

PipelineParameterSpanReadOnlyWidget::PipelineParameterSpanReadOnlyWidget(Pipeline const& pipeline,
                                                                         PipelineParameterSpan& parameterSpan,
                                                                         ArtifactVariantPointer artifactVariantPointer,
                                                                         QWidget* parent) :
        PipelineParameterSpanWidget(pipeline, parent) {

    NameEdit->SetData(QString::fromStdString(parameterSpan.GetName()));
    NameEdit->setEnabled(false);

    ArtifactsView->setEnabled(false);

    PropertyGroup->UpdateWidget(new ObjectPropertyGroup(artifactVariantPointer));
}



PipelineArtifactsView::PipelineArtifactsView(Pipeline const& pipeline) :
        ImageArtifactsView(new ImageArtifactsReadOnlyView(pipeline)),
        StructureArtifactsView(new PipelineStructureArtifactsView(pipeline)) {

    addWidget(ImageArtifactsView);

    connect(ImageArtifactsView, &ImageArtifactsReadOnlyView::ImageArtifactChanged,
            this, [this](ImageArtifact* imageArtifact) {
        emit ArtifactChanged(ArtifactVariantPointer(imageArtifact));
    });

    connect(StructureArtifactsView, &PipelineStructureArtifactsView::StructureArtifactChanged,
            this, [this](StructureArtifact* structureArtifact) {
        emit ArtifactChanged(ArtifactVariantPointer(structureArtifact));
    });
}

void PipelineArtifactsView::Show(PipelineArtifactsView::View view) {
    if (view == IMAGE_ARTIFACTS)
        setCurrentWidget(ImageArtifactsView);
    else
        throw std::runtime_error("todo");
}

PipelineStructureArtifactsView::PipelineStructureArtifactsView(Pipeline const& pipeline, QWidget* parent) :
        APipeline(pipeline),
        StructuresView(new CtStructureReadOnlyView(pipeline.GetCtStructureTree())),
        ArtifactsView(new OptionalWidget<StructureArtifactsReadOnlyView>("Select a structure")) {

    addWidget(StructuresView);
    addWidget(ArtifactsView);

    connect(StructuresView, &CtStructureReadOnlyView::CtStructureChanged, this, [&](idx_t structureIdx) {
        if (!structureIdx) {
            ArtifactsView->HideWidget();
            return;
        }

        auto& structureArtifactList = APipeline.GetStructureArtifactList(*structureIdx);
        ArtifactsView->UpdateWidget(new StructureArtifactsReadOnlyView(structureArtifactList));

        connect(&ArtifactsView->Widget(), &StructureArtifactsReadOnlyView::StructureArtifactChanged,
                this, &PipelineStructureArtifactsView::StructureArtifactChanged);
    });
}
