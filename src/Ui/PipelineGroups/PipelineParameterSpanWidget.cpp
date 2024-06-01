#include "PipelineParameterSpanWidget.h"

#include "ObjectPropertyWidget.h"
#include "../Modeling/CtStructureTreeModel.h"
#include "../Modeling/CtStructureView.h"
#include "../Artifacts/Image/ImageArtifactsView.h"
#include "../Artifacts/Structure/StructureArtifactsView.h"
#include "../Utils/CoordinateRowWidget.h"
#include "../Utils/NameLineEdit.h"
#include "../../Artifacts/Pipeline.h"
#include "../../Artifacts/Structure/StructureArtifactListCollection.h"
#include "../../Modeling/CtStructureTree.h"
#include "../../Utils/Types.h"
#include "../../Utils/Overload.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>


PipelineParameterSpanWidget::PipelineParameterSpanWidget(Pipeline const& pipeline, QWidget* parent) :
        QWidget(parent),
        FLayout(new QFormLayout(this)),
        NameEdit(new NameLineEdit()),
        NumberOfPipelinesSpinBox([]() {
            auto* spinBox = new QSpinBox();
            spinBox->setRange(0, 10000);
            return spinBox;
        }()),
        ArtifactsView(new PipelineArtifactsView(pipeline)),
        PropertyGroup(new OptionalWidget<ObjectPropertyGroup>("Select an artifact")) {

    FLayout->addRow("Name", NameEdit);

    FLayout->addRow("Number of Pipelines", NumberOfPipelinesSpinBox);
    NumberOfPipelinesSpinBox->setEnabled(false);

    FLayout->addRow(ArtifactsView);

    FLayout->addRow(PropertyGroup);

    FLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(ArtifactsView, &PipelineArtifactsView::ArtifactChanged,
            this, [this](ArtifactVariantPointer artifactVariantPointer) {

        CurrentArtifactPointer = artifactVariantPointer;

        if (artifactVariantPointer.IsNullptr()) {
            PropertyGroup->HideWidget();
            return;
        }

        auto* propertyGroup = new ObjectPropertyGroup(artifactVariantPointer);
        PropertyGroup->UpdateWidget(propertyGroup);

        connect(propertyGroup, &ObjectPropertyGroup::ValueChanged,
                this, [this, propertyGroup]() -> void {
            auto numberOfPipelines = propertyGroup->GetParameterSpan({}, "").GetNumberOfPipelines();
            NumberOfPipelinesSpinBox->setValue(static_cast<int>(numberOfPipelines));
        });
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

auto PipelineParameterSpanCreateWidget::GetPipelineParameterSpan() const -> PipelineParameterSpan {
    return PropertyGroup->Widget().GetParameterSpan(CurrentArtifactPointer, NameEdit->GetText().toStdString());
}


PipelineParameterSpanReadOnlyWidget::PipelineParameterSpanReadOnlyWidget(Pipeline const& pipeline,
                                                                         PipelineParameterSpan& parameterSpan,
                                                                         QWidget* parent) :
        PipelineParameterSpanWidget(pipeline, parent) {

    NameEdit->SetText(QString::fromStdString(parameterSpan.GetName()));
    NameEdit->setEnabled(false);

    ArtifactsView->SelectArtifact(parameterSpan.GetArtifact());
    ArtifactsView->setEnabled(false);

    auto* objectPropertyGroup = new ObjectPropertyGroup(parameterSpan.GetArtifact());
    objectPropertyGroup->SetParameterSpan(parameterSpan);
    PropertyGroup->UpdateWidget(objectPropertyGroup);

    auto numberOfPipelines = PropertyGroup->Widget().GetParameterSpan({}, "").GetNumberOfPipelines();
    NumberOfPipelinesSpinBox->setValue(static_cast<int>(numberOfPipelines));
}


PipelineArtifactsView::PipelineArtifactsView(Pipeline const& pipeline) :
        SelectViewComboBox([]() {
            auto* comboBox = new QComboBox();
            comboBox->addItem("Image Artifacts", PipelineArtifactsView::View::IMAGE_ARTIFACTS);
            comboBox->addItem("Structure Artifacts", PipelineArtifactsView::View::STRUCTURE_ARTIFACTS);
            comboBox->setCurrentIndex(0);
            return comboBox;
        }()),
        ImageArtifactsView(new ImageArtifactsReadOnlyView(pipeline)),
        StructureArtifactsView(new PipelineStructureArtifactsView(pipeline)) {

    setTitle("Artifact");

    auto* stackedView = new QStackedWidget();
    stackedView->addWidget(ImageArtifactsView);
    stackedView->addWidget(StructureArtifactsView);

    auto* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(SelectViewComboBox);
    vLayout->addWidget(stackedView);

    connect(ImageArtifactsView, &ImageArtifactsReadOnlyView::ImageArtifactChanged,
            this, [this](ImageArtifact* imageArtifact) {
        Q_EMIT ArtifactChanged(ArtifactVariantPointer(imageArtifact));
    });

    connect(StructureArtifactsView, &PipelineStructureArtifactsView::StructureArtifactChanged,
            this, [this](StructureArtifact* structureArtifact) {
        Q_EMIT ArtifactChanged(ArtifactVariantPointer(structureArtifact));
    });

    connect(SelectViewComboBox, &QComboBox::currentIndexChanged,
            this, [this, stackedView]() {
        switch (SelectViewComboBox->currentData().value<View>()) {
            case IMAGE_ARTIFACTS:     stackedView->setCurrentWidget(ImageArtifactsView);     break;
            case STRUCTURE_ARTIFACTS: stackedView->setCurrentWidget(StructureArtifactsView); break;
        }

        Q_EMIT ArtifactChanged({});
    });
}

auto PipelineArtifactsView::SelectArtifact(ArtifactVariantPointer artifactPointer) -> void {
    if (artifactPointer.IsNullptr())
        throw std::runtime_error("Artifact pointer must not be nullptr");

    std::visit(Overload {
        [this](ImageArtifact const* artifact) -> void {
            SelectViewComboBox->setCurrentIndex(
                    SelectViewComboBox->findData(QVariant::fromValue(View::IMAGE_ARTIFACTS)));
            ImageArtifactsView->Select(*artifact); },
        [this](StructureArtifact const* artifact) -> void {
            SelectViewComboBox->setCurrentIndex(
                    SelectViewComboBox->findData(QVariant::fromValue(View::STRUCTURE_ARTIFACTS)));
            StructureArtifactsView->Select(*artifact); }
    }, artifactPointer.GetVariant());
}


PipelineStructureArtifactsView::PipelineStructureArtifactsView(Pipeline const& pipeline, QWidget* /*parent*/) :
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
        auto* artifactsView = new StructureArtifactsReadOnlyView(structureArtifactList);
        artifactsView->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
        ArtifactsView->UpdateWidget(artifactsView);

        Q_EMIT StructureArtifactChanged(nullptr);

        connect(&ArtifactsView->Widget(), &StructureArtifactsReadOnlyView::StructureArtifactChanged,
                this, &PipelineStructureArtifactsView::StructureArtifactChanged);
    });
}

auto PipelineStructureArtifactsView::Select(StructureArtifact const& structureArtifact) -> void {
    auto const& structureArtifactCollection = APipeline.GetStructureArtifactListCollection();

    auto const& structureArtifactList = structureArtifactCollection.GetStructureArtifactList(structureArtifact);

    auto ctStructureIdx = structureArtifactCollection.GetIdx(structureArtifactList);

    auto const& structureVariant = APipeline.GetCtStructureTree().GetStructureAt(ctStructureIdx);

    std::visit([this](auto const& structure) { StructuresView->Select(structure); }, structureVariant);

    ArtifactsView->Widget().Select(structureArtifact);
}
