#include "TsneMainWidget.h"

#include "PipelineParameterSpaceStateView.h"
#include "TsneChartWidget.h"
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


TsneMainWidget::TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource) :
        GroupList(pipelineGroups),
        BatchData(std::make_unique<std::optional<PipelineBatchListData>>(pipelineGroups.GetBatchData())),
        ChartWidget(new OptionalWidget<TsneChartWidget>("Please generate the data first", new TsneChartWidget())),
        RenderWidget(new ParameterSpaceStateRenderWidget(dataSource)),
        DataWidget(new OptionalWidget<TsneDataWidget>("Please select a sample point", new TsneDataWidget())) {

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setWidget(DataWidget);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);

    auto* splitter = new QSplitter();
    splitter->addWidget(ChartWidget);
    splitter->addWidget(RenderWidget);
    splitter->setSizes({ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() });
    setCentralWidget(splitter);

    connect(&ChartWidget->Widget(), &TsneChartWidget::SamplePointChanged,
            RenderWidget, &ParameterSpaceStateRenderWidget::UpdateSample);

    connect(&ChartWidget->Widget(), &TsneChartWidget::SamplePointChanged,
            &DataWidget->Widget(), [this](std::optional<SampleId> sampleId) {
                DataWidget->Widget().UpdateSample(sampleId);

                if (sampleId)
                    DataWidget->ShowWidget();
                else
                    DataWidget->HideWidget();
            });

    connect(&DataWidget->Widget(), &TsneDataWidget::RequestResetCamera,
            RenderWidget, &ParameterSpaceStateRenderWidget::ResetCamera);

    UpdateData();
}

auto TsneMainWidget::UpdateData() -> void {
    BatchData = std::make_unique<std::optional<PipelineBatchListData>>(GroupList.GetBatchData());

    PipelineBatchListData const* data = *BatchData
            ? &**BatchData
            : nullptr;
    RenderWidget->UpdateData(data);
    ChartWidget->Widget().UpdateData(data);
    DataWidget->Widget().UpdateData(data);

    if (data)
        ChartWidget->ShowWidget();
    else
        ChartWidget->HideWidget();

    Q_EMIT ChartWidget->Widget().SamplePointChanged(std::nullopt);
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

TsneDataWidget::TsneDataWidget() :
        BatchListData(nullptr),
        ResetCameraButton(new QPushButton("Reset Camera")),
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

    auto* renderingButtonBarWidget = new QWidget();
    auto* renderingHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingHorizontalLayout->addWidget(ResetCameraButton);
    renderingHorizontalLayout->addStretch();
    fLayout->addRow(renderingButtonBarWidget);

    connect(ResetCameraButton, &QPushButton::clicked, this, &TsneDataWidget::RequestResetCamera);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    fLayout->addRow(line);

    auto* title = new QLabel("Selected Pipeline");
    title->setStyleSheet(GetHeaderStyleSheet());
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
    auto* selectedPointLabel = new QLabel("t-SNE values");
    selectedPointLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    fLayout->addRow(selectedPointLabel, pointWidget);

    fLayout->addRow(ParameterSpaceStateView);

    fLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding));
}

auto TsneDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto TsneDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        return;

    if (!BatchListData)
        throw std::runtime_error("Data must not be null");

    auto const& batchData = BatchListData->GetBatchData(sampleId->GroupIdx);
    auto const& group = batchData.Group;
    auto const& spaceStateData = BatchListData->GetSpaceStateData(*sampleId);

    auto const xValue = spaceStateData.TsneCoordinates.at(0);
    auto const yValue = spaceStateData.TsneCoordinates.at(1);
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
