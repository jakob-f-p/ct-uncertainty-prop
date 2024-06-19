#include "AnalysisMainWidget.h"

#include "PipelineParameterSpaceStateView.h"
#include "ChartWidget.h"
#include "../Utils/NameLineEdit.h"
#include "../Utils/WidgetUtils.h"
#include "../../Modeling/CtDataSource.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QSplitter>

#include <vtkImageData.h>


AnalysisMainWidget::AnalysisMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource,
                                       ChartWidget* chartWidget, AnalysisDataWidget* dataWidget) :
        GroupList(pipelineGroups),
        BatchData(std::make_unique<std::optional<PipelineBatchListData>>(pipelineGroups.GetBatchData())),
        ChartWidget_(new OptionalWidget<ChartWidget>("Please generate the data first", chartWidget)),
        RenderWidget(new ParameterSpaceStateRenderWidget(dataSource)),
        DataWidget(new OptionalWidget<AnalysisDataWidget>("Please select a sample point", dataWidget)) {

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setWidget(DataWidget);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);

    auto* splitter = new QSplitter();
    splitter->addWidget(ChartWidget_);
    splitter->addWidget(RenderWidget);
    splitter->setSizes({ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() });
    setCentralWidget(splitter);

    connect(&ChartWidget_->Widget(), &ChartWidget::SamplePointChanged,
            RenderWidget, &ParameterSpaceStateRenderWidget::UpdateSample);

    connect(&ChartWidget_->Widget(), &ChartWidget::SamplePointChanged,
            &DataWidget->Widget(), [this](std::optional<SampleId> sampleId) {
                DataWidget->Widget().UpdateSample(sampleId);

                if (sampleId)
                    DataWidget->ShowWidget();
                else
                    DataWidget->HideWidget();
            });

    UpdateData();
}

PcaMainWidget::PcaMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        AnalysisMainWidget(pipelineGroups, dataSource, new PcaChartWidget(), new PcaDataWidget()) {}

TsneMainWidget::TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        AnalysisMainWidget(pipelineGroups, dataSource, new TsneChartWidget(), new TsneDataWidget()) {}

auto AnalysisMainWidget::UpdateData() -> void {
    BatchData = std::make_unique<std::optional<PipelineBatchListData>>(GroupList.GetBatchData());

    PipelineBatchListData const* data = *BatchData
            ? &**BatchData
            : nullptr;
    RenderWidget->UpdateData(data);
    ChartWidget_->Widget().UpdateData(data);
    DataWidget->Widget().UpdateData(data);

    if (data)
        ChartWidget_->ShowWidget();
    else
        ChartWidget_->HideWidget();

    Q_EMIT ChartWidget_->Widget().SamplePointChanged(std::nullopt);
}

ParameterSpaceStateRenderWidget::ParameterSpaceStateRenderWidget(CtDataSource& dataSource) :
        RenderWidget(dataSource),
        DataSource(dataSource),
        BatchListData(nullptr) {}

auto ParameterSpaceStateRenderWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto ParameterSpaceStateRenderWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        UpdateImageAlgorithm(DataSource);
    else {
        auto const& parameterSpaceStateData = BatchListData->GetSpaceStateData(*sampleId);

        UpdateImageAlgorithm(*parameterSpaceStateData.ImageData);
    }

    Render();
}

AnalysisDataWidget::AnalysisDataWidget(QString const& analysisName) :
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

    fLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding));
}

auto AnalysisDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto AnalysisDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        return;

    if (!BatchListData)
        throw std::runtime_error("Data must not be null");

    auto const& batchData = BatchListData->GetBatchData(sampleId->GroupIdx);
    auto const& group = batchData.Group;
    auto const& spaceStateData = BatchListData->GetSpaceStateData(*sampleId);

    auto const [ xValue, yValue ] = GetXYData(spaceStateData);
    if (PointXSpinBox->minimum() > xValue)
        PointXSpinBox->setMinimum(xValue);
    if (PointXSpinBox->maximum() < xValue)
        PointXSpinBox->setMaximum(xValue);
    if (PointYSpinBox->minimum() > yValue)
        PointYSpinBox->setMinimum(yValue);
    if (PointYSpinBox->maximum() < yValue)
        PointYSpinBox->setMaximum(yValue);
    PointXSpinBox->setValue(xValue);
    PointYSpinBox->setValue(yValue);

    PipelineGroupNameEdit->SetText(QString::fromStdString(group.GetName()));
    auto const numberOfGroupPipelines = static_cast<int>(batchData.StateDataList.size());
    if (NumberOfGroupPipelinesSpinBox->maximum() < numberOfGroupPipelines)
        NumberOfGroupPipelinesSpinBox->setMaximum(numberOfGroupPipelines);
    NumberOfGroupPipelinesSpinBox->setValue(numberOfGroupPipelines);
    BasePipelineNameEdit->SetText(QString::fromStdString(group.GetBasePipeline().GetName()));

    ParameterSpaceStateView->UpdateWidget(new PipelineParameterSpaceStateView(spaceStateData.State));
}

PcaDataWidget::PcaDataWidget() :
        AnalysisDataWidget("PCA") {}

auto
PcaDataWidget::GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords {
    return { spaceStateData.PcaCoordinates.at(0), spaceStateData.PcaCoordinates.at(1) };
}

TsneDataWidget::TsneDataWidget() :
        AnalysisDataWidget("t-SNE") {}
auto
TsneDataWidget::GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords {
    return { spaceStateData.TsneCoordinates.at(0), spaceStateData.TsneCoordinates.at(1) };
}
