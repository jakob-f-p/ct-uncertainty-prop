#include "PipelineGroupWidget.h"

#include "PipelineParameterSpaceView.h"
#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroup.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

PipelineGroupWidget::PipelineGroupWidget(PipelineGroup& pipelineGroup, QWidget* parent) :
        QWidget(parent),
        Group(pipelineGroup),
        AddParameterSpanButton([]() {
            auto* button = new QPushButton();
            button->setIcon(GenerateIcon("Plus"));
            return button;
        }()),
        RemoveParameterSpanButton([]() {
            auto* button = new QPushButton();
            button->setIcon(GenerateIcon("Minus"));
            return button;
        }()),
        ParameterSpaceView(new PipelineParameterSpaceView(pipelineGroup.GetParameterSpace())),
        SelectionModel(ParameterSpaceView->selectionModel()) {

    auto* vLayout = new QVBoxLayout(this);

    auto* title = new QLabel(QString::fromStdString(Group.GetName()));
    title->setStyleSheet(GetHeaderStyleSheet());
    vLayout->addWidget(title);

    auto* basePipelineName = new QLabel(QString::fromStdString(Group.GetBasePipeline().GetName()));
    vLayout->addWidget(basePipelineName);

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->addWidget(AddParameterSpanButton);
    buttonBarHLayout->addWidget(RemoveParameterSpanButton);
    vLayout->addWidget(buttonBar);

    vLayout->addWidget(ParameterSpaceView);

    UpdateButtonStatus();

    connect(AddParameterSpanButton, &QPushButton::clicked, this, &PipelineGroupWidget::RequestCreateParameterSpan);
    connect(RemoveParameterSpanButton, &QPushButton::clicked, this, &PipelineGroupWidget::OnRemoveParameterSpan);

    connect(SelectionModel, &QItemSelectionModel::selectionChanged, this, &PipelineGroupWidget::OnSelectionChanged);
}

auto PipelineGroupWidget::UpdateButtonStatus() -> void {
    QModelIndexList const selectedIndices = SelectionModel->selection().indexes();
    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection (larger than one index)");

    bool const isParameterSpan = !selectedIndices.empty()
            && selectedIndices.at(0).data(PipelineParameterSpaceModel::Roles::IS_PARAMETER_SPAN).toBool();

    AddParameterSpanButton->setEnabled(true);
    RemoveParameterSpanButton->setEnabled(isParameterSpan);
}

void PipelineGroupWidget::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                           PipelineParameterSpan&& parameterSpan) {
    QModelIndex const index = ParameterSpaceView->model()->AddParameterSpan(artifactVariantPointer,
                                                                            std::move(parameterSpan));

    SelectionModel->clearSelection();
    SelectionModel->select(index, QItemSelectionModel::SelectionFlag::Select);

    UpdateButtonStatus();
}

void PipelineGroupWidget::OnRemoveParameterSpan() {
    QModelIndex const index = SelectionModel->selection().indexes().at(0);
    ParameterSpaceView->model()->RemoveParameterSpan(index);

    SelectionModel->clearSelection();
    UpdateButtonStatus();
}

void PipelineGroupWidget::OnSelectionChanged(QItemSelection const& selected, QItemSelection const&  /*deselected*/) {
    QModelIndexList const selectedIndices = selected.indexes();

    if (selectedIndices.empty()) {
        emit ParameterSpanChanged(nullptr, {});
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);
    QModelIndex const parentIndex = selectedIndex.parent();

    if (!selectedIndex.data(PipelineParameterSpaceModel::Roles::IS_PARAMETER_SPAN).toBool())
        throw std::runtime_error("Selected index must be a a parameter span");

    if (!parentIndex.isValid())
        throw std::runtime_error("Parent index has to be valid");

    auto* parameterSpan
            = selectedIndex.data(PipelineParameterSpaceModel::POINTER).value<PipelineParameterSpan*>();

    auto* parameterSpanSet
            = parentIndex.data(PipelineParameterSpaceModel::POINTER).value<PipelineParameterSpanSet*>();

    emit ParameterSpanChanged(parameterSpan, parameterSpanSet->GetArtifactPointer());
}
