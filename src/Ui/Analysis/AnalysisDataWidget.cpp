#include "AnalysisDataWidget.h"

#include "ChartTooltip.h"
#include "PipelineParameterSpaceStateView.h"
#include "../Utils/NameLineEdit.h"
#include "../Utils/WidgetUtils.h"
#include "../../Artifacts/Pipeline.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QBarSet>
#include <QChartView>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHorizontalBarSeries>
#include <QSpinBox>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QBarSeries>


AnalysisSampleDataWidget::AnalysisSampleDataWidget(QString const& analysisName) :
        AnalysisName(analysisName),
        BatchListData(nullptr),
        PipelineGroupNameEdit([]() {
            auto* lineEdit = new NameLineEdit();
            lineEdit->setEnabled(false);
            return lineEdit;
        }()),
        NumberOfGroupPipelinesSpinBox([]() {
            auto* spinBox = new QSpinBox();
            spinBox->setEnabled(false);
            spinBox->setRange(0, 100);
            return spinBox;
        }()),
        BasePipelineNameEdit([]() {
            auto* lineEdit = new NameLineEdit();
            lineEdit->setEnabled(false);
            return lineEdit;
        }()),
        PointXSpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setEnabled(false);
            return spinBox;
        }()),
        PointYSpinBox([]() {
            auto* spinBox = new QDoubleSpinBox();
            spinBox->setEnabled(false);
            return spinBox;
        }()),
        ParameterSpaceStateView(new OptionalWidget<PipelineParameterSpaceStateView>("")) {

    auto* fLayout = new QFormLayout(this);
    fLayout->setContentsMargins({});

    auto* title = new QLabel("Selected Pipeline");
    title->setStyleSheet(GetHeader1StyleSheet());
    fLayout->addRow(title);

    fLayout->addRow("Group", PipelineGroupNameEdit);
    fLayout->addRow("Group pipelines", NumberOfGroupPipelinesSpinBox);
    fLayout->addRow("Base pipeline", BasePipelineNameEdit);

    auto* pointWidget = new QWidget();
    auto* pointWidgetGridLayout = new QGridLayout(pointWidget);
    pointWidgetGridLayout->setContentsMargins({});
    auto* pointWidgetXLabel = new QLabel("x");
    auto* pointWidgetYLabel = new QLabel("y");
    pointWidgetXLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    pointWidgetYLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    pointWidgetGridLayout->addItem(
            new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred), 0, 0);
    pointWidgetGridLayout->addWidget(pointWidgetXLabel, 0, 1);
    pointWidgetGridLayout->addWidget(PointXSpinBox, 0, 2);
    pointWidgetGridLayout->addItem(
            new QSpacerItem(10, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred), 0, 3);
    pointWidgetGridLayout->addWidget(pointWidgetYLabel, 0, 4);
    pointWidgetGridLayout->addWidget(PointYSpinBox, 0, 5);
    auto* selectedPointLabel = new QLabel(QString("%1 values").arg(AnalysisName));
    selectedPointLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    fLayout->addRow(selectedPointLabel, pointWidget);

    fLayout->addRow(ParameterSpaceStateView);
    ParameterSpaceStateView->setMinimumWidth(100);
    ParameterSpaceStateView->setMinimumHeight(100);
    ParameterSpaceStateView->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

    fLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding));
}

auto AnalysisSampleDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto AnalysisSampleDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        return;

    if (!BatchListData)
        throw std::runtime_error("Data must not be null");

    auto const& batchData = BatchListData->Data.at(sampleId->GroupIdx);
    auto const& group = batchData.Group;
    auto const& spaceStateData = BatchListData->GetSpaceStateData(*sampleId);

    auto const [ xValue, yValue ] = GetXYData(spaceStateData);
    PointXSpinBox->setMinimum(std::min(PointXSpinBox->minimum(), xValue));
    PointXSpinBox->setMaximum(std::max(PointXSpinBox->maximum(), xValue));
    PointXSpinBox->setValue(xValue);
    PointYSpinBox->setMinimum(std::min(PointYSpinBox->minimum(), yValue));
    PointYSpinBox->setMaximum(std::max(PointYSpinBox->maximum(), yValue));
    PointYSpinBox->setValue(yValue);

    PipelineGroupNameEdit->SetText(QString::fromStdString(group.GetName()));
    auto const numberOfGroupPipelines = static_cast<int>(batchData.StateDataList.size());
    if (NumberOfGroupPipelinesSpinBox->maximum() < numberOfGroupPipelines)
        NumberOfGroupPipelinesSpinBox->setMaximum(numberOfGroupPipelines);
    NumberOfGroupPipelinesSpinBox->setValue(numberOfGroupPipelines);
    BasePipelineNameEdit->SetText(QString::fromStdString(group.GetBasePipeline().GetName()));

    ParameterSpaceStateView->UpdateWidget(new PipelineParameterSpaceStateView(spaceStateData.State));
}
PcaSampleDataWidget::PcaSampleDataWidget() :
        AnalysisSampleDataWidget("PCA") {}

auto
PcaSampleDataWidget::GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords {
    return { spaceStateData.PcaCoordinates.at(0), spaceStateData.PcaCoordinates.at(1) };
}

TsneSampleDataWidget::TsneSampleDataWidget() :
        AnalysisSampleDataWidget("t-SNE") {}

auto
TsneSampleDataWidget::GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords {
    return { spaceStateData.TsneCoordinates.at(0), spaceStateData.TsneCoordinates.at(1) };
}
