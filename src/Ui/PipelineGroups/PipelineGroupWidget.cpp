#include "PipelineGroupWidget.h"

#include "PipelineParameterSpaceView.h"
#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroup.h"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>


PipelineGroupWidget::PipelineGroupWidget(PipelineGroup& pipelineGroup, QWidget* parent) :
        QWidget(parent),
        Group(pipelineGroup),
        NumberOfPipelinesSpinBox([&pipelineGroup]() {
            auto* spinBox = new QSpinBox();
            spinBox->setRange(0, 10000);
            spinBox->setValue(pipelineGroup.GetParameterSpace().GetNumberOfPipelines());
            spinBox->setEnabled(false);
            return spinBox;
        }()),
        AddParameterSpanButton(new QPushButton(GenerateIcon("Plus"), " Add")),
        RemoveParameterSpanButton(new QPushButton(GenerateIcon("Minus"), " Remove")),
        ParameterSpaceView(new PipelineParameterSpaceView(pipelineGroup.GetParameterSpace())),
        SelectionModel(ParameterSpaceView->selectionModel()) {

    auto* fLayout = new QFormLayout(this);

    auto* title = new QLabel(QString::fromStdString(Group.GetName()));
    title->setStyleSheet(GetHeaderStyleSheet());
    fLayout->addRow(title);

    fLayout->addRow("Number of pipelines", NumberOfPipelinesSpinBox);

    auto* basePipelineName = new QLabel(QString::fromStdString(Group.GetBasePipeline().GetName()));
    fLayout->addRow("Base pipeline", basePipelineName);

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->addStretch();
    buttonBarHLayout->addWidget(AddParameterSpanButton);
    buttonBarHLayout->addWidget(RemoveParameterSpanButton);
    fLayout->addRow(buttonBar);

    fLayout->addRow(ParameterSpaceView);

    UpdateButtonStatus();

    connect(AddParameterSpanButton, &QPushButton::clicked, this, &PipelineGroupWidget::RequestCreateParameterSpan);
    connect(RemoveParameterSpanButton, &QPushButton::clicked, this, &PipelineGroupWidget::OnRemoveParameterSpan);

    connect(SelectionModel, &QItemSelectionModel::selectionChanged, this, &PipelineGroupWidget::OnSelectionChanged);
}

void PipelineGroupWidget::AddParameterSpan(PipelineParameterSpan&& parameterSpan) {
    QModelIndex const index = ParameterSpaceView->model()->AddParameterSpan(std::move(parameterSpan));

    SelectionModel->clearSelection();
    SelectionModel->select(index, QItemSelectionModel::SelectionFlag::Select);

    UpdateButtonStatus();
    UpdateNumberOfPipelines();
}

void PipelineGroupWidget::UpdateNumberOfPipelines() {
    NumberOfPipelinesSpinBox->setValue(Group.GetParameterSpace().GetNumberOfPipelines());

    emit NumberOfPipelinesUpdated();
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

void PipelineGroupWidget::OnRemoveParameterSpan() {
    QModelIndex const index = SelectionModel->selection().indexes().at(0);
    ParameterSpaceView->model()->RemoveParameterSpan(index);

    SelectionModel->clearSelection();
    UpdateButtonStatus();
    UpdateNumberOfPipelines();
}

void PipelineGroupWidget::OnSelectionChanged(QItemSelection const& selected, QItemSelection const& /*deselected*/) {
    QModelIndexList const selectedIndices = selected.indexes();

    if (selectedIndices.empty()) {
        emit ParameterSpanChanged(nullptr);
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);
    QModelIndex const parentIndex = selectedIndex.parent();

    if (!selectedIndex.data(PipelineParameterSpaceModel::Roles::IS_PARAMETER_SPAN).toBool()) {
        emit ParameterSpanChanged(nullptr);
        return;
    }

    if (!parentIndex.isValid())
        throw std::runtime_error("Parent index has to be valid");

    auto* parameterSpan
            = selectedIndex.data(PipelineParameterSpaceModel::POINTER).value<PipelineParameterSpan*>();

    emit ParameterSpanChanged(parameterSpan);
}
