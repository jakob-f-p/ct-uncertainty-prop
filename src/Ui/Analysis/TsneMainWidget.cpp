#include "TsneMainWidget.h"

#include "../../Modeling/CtDataSource.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QChart>
#include <QChartView>
#include <QFrame>
#include <QPushButton>
#include <QScatterSeries>
#include <QSplitter>
#include <QValueAxis>
#include <QVBoxLayout>

#include <vtkImageData.h>

#include <algorithm>
#include <limits>


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
}


TsneChartWidget::TsneChartWidget() :
        BatchListData(nullptr),
        ChartView(new QChartView()) {

    ChartView->setRenderHint(QPainter::RenderHint::Antialiasing);
    auto* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(ChartView);

    UpdateData(BatchListData);
}

auto TsneChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    BatchListData = batchListData;

    if (!BatchListData)
        return;

    std::vector<QScatterSeries*> scatterSeriesVector;
    scatterSeriesVector.reserve(BatchListData->Data.size());
    for (auto const& batchData : BatchListData->Data) {
        auto* scatterSeries = new QScatterSeries();
        scatterSeries->setName(QString::fromStdString(batchData.Group.GetName()));
        scatterSeries->setMarkerSize(scatterSeries->markerSize() * 0.75);

        QList<QPointF> points;
        std::transform(batchData.StateDataList.cbegin(), batchData.StateDataList.cend(),
                       std::back_inserter(points),
                       [](auto const psData) {
            return QPointF(psData.TsneCoordinates.at(0), psData.TsneCoordinates.at(1));
        });
        scatterSeries->append(points);

        scatterSeriesVector.emplace_back(scatterSeries);
    }

    auto* chart = new QChart();
    chart->setTheme(QChart::ChartTheme::ChartThemeQt);
    chart->setTitle("t-SNE analysis");
    auto titleFont = chart->titleFont();
    titleFont.setPointSize(static_cast<int>(static_cast<double>(titleFont.pointSize()) * 2.0));
    chart->setTitleFont(titleFont);
    chart->legend()->setAlignment(Qt::AlignBottom);
    for (auto* scatterSeries : scatterSeriesVector)
        chart->addSeries(scatterSeries);
    chart->createDefaultAxes();

    auto* xAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    auto* yAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    auto [ xMin, xMax ] = GetAxisRange(xAxis);
    auto [ yMin, yMax ] = GetAxisRange(yAxis);
    xAxis->setRange(xMin, xMax);
    yAxis->setRange(yMin, yMax);
    xAxis->setTickType(QValueAxis::TickType::TicksDynamic);
    yAxis->setTickType(QValueAxis::TickType::TicksDynamic);
    xAxis->setTickInterval(GetAxisTickInterval(xAxis));
    yAxis->setTickInterval(GetAxisTickInterval(yAxis));

    QChart* oldChart = ChartView->chart();
    ChartView->setChart(chart);

    if (oldChart) {
        delete oldChart;
        oldChart = nullptr;
    }
}

auto TsneChartWidget::GetAxisRange(QValueAxis* axis) -> std::pair<int, int> {
    if (!axis)
        throw std::runtime_error("axis must not be nullptr");

    double const min = axis->min();
    double const max = axis->max();

    double const range = max - min;
    double const padding = range * 0.05;

    double const axisMin = min - padding;
    double const axisMax = max + padding;

    return { axisMin, axisMax };
}

auto TsneChartWidget::GetAxisTickInterval(QValueAxis* axis) -> double {
    if (!axis)
        throw std::runtime_error("axis must not be nullptr");

    double const range = axis->max() - axis->min();
    double const interval = range / 10.0;

    double const roundingDigitPlace = std::floor(std::log10(interval));
    double const roundingFactor = std::pow(10, roundingDigitPlace);

    double roundedInterval = std::ceil(interval / roundingFactor) * roundingFactor;
    if (interval / roundedInterval < 0.75)
        roundedInterval -= roundingFactor / 2;

    return roundedInterval;
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
        ResetCameraButton(new QPushButton("Reset Camera")),
        BatchListData(nullptr) {

    auto* vLayout = new QVBoxLayout(this);

    auto* renderingButtonBarWidget = new QWidget();
    auto* renderingHorizontalLayout = new QHBoxLayout(renderingButtonBarWidget);
    renderingHorizontalLayout->setContentsMargins(0, 11, 0, 11);
    renderingHorizontalLayout->addWidget(ResetCameraButton);
    renderingHorizontalLayout->addStretch();
    vLayout->addWidget(renderingButtonBarWidget);

    connect(ResetCameraButton, &QPushButton::clicked, this, &TsneDataWidget::RequestResetCamera);

    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vLayout->addWidget(line);
}

auto TsneDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto TsneDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {

}
