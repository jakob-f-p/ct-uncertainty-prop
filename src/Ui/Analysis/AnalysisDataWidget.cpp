#include "AnalysisDataWidget.h"

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


AnalysisDataWidget::AnalysisDataWidget(AnalysisSampleDataWidget* sampleDataWidget) :
        VLayout(new QVBoxLayout(this)),
        SampleDataWidget(new OptionalWidget<AnalysisSampleDataWidget>("Please select a sample point",
                                                                      sampleDataWidget)) {

    VLayout->addWidget(SampleDataWidget);
    VLayout->setContentsMargins({});
}

auto AnalysisDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    SampleDataWidget->Widget().UpdateData(batchData);

    UpdateDataDerived(batchData);
}

auto AnalysisDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    SampleDataWidget->Widget().UpdateSample(sampleId);
    if (sampleId)
        SampleDataWidget->ShowWidget();
    else
        SampleDataWidget->HideWidget();
}

auto AnalysisDataWidget::UpdateDataDerived(PipelineBatchListData const* batchData) -> void {}

PcaDataWidget::PcaDataWidget() :
        AnalysisDataWidget(new PcaSampleDataWidget()),
        PcaAnalysisChartWidget(new OptionalWidget<PcaAnalysisDataWidget>("Please generate the data first",
                                                                         new PcaAnalysisDataWidget())) {
    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);

    VLayout->addSpacing(5);
    VLayout->addWidget(separator);
    VLayout->addSpacing(5);

    VLayout->addWidget(PcaAnalysisChartWidget);
}

auto PcaDataWidget::UpdateDataDerived(PipelineBatchListData const* batchData) -> void {
    PcaAnalysisChartWidget->Widget().UpdateData(batchData);
    if (batchData)
        PcaAnalysisChartWidget->ShowWidget();
    else
        PcaAnalysisChartWidget->HideWidget();
}

TsneDataWidget::TsneDataWidget() :
        AnalysisDataWidget(new TsneSampleDataWidget()) {}



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
    auto margins = fLayout->contentsMargins();
    margins.setTop(0);
    fLayout->setContentsMargins(margins);

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

auto AnalysisSampleDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    UpdateSample(std::nullopt);
}

auto AnalysisSampleDataWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        return;

    if (!BatchListData)
        throw std::runtime_error("Data must not be null");

    auto const& batchData = BatchListData->GetBatchData(sampleId->GroupIdx);
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

PcaAnalysisDataWidget::PcaAnalysisDataWidget() :
        BatchData(nullptr),
        ExplainedVarianceChartView(new QChartView()),
        PrincipalAxesChartView(new OptionalWidget<QChartView>("Please select a principal component",
                                                              new QChartView())) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});
    vLayout->addWidget(ExplainedVarianceChartView);
    vLayout->addWidget(PrincipalAxesChartView);
}

auto PcaAnalysisDataWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchData = batchData;

    PrincipalAxesChartView->HideWidget();

    if (BatchData)
        UpdateExplainedVarianceChart();
}

auto PcaAnalysisDataWidget::UpdateExplainedVarianceChart() -> void {
    std::vector<double> const& varianceRatios = BatchData->PcaExplainedVarianceRatios;

    auto* barSet = new QBarSet("Explained Variance (%)");
    std::for_each(varianceRatios.rbegin(), varianceRatios.rend(),
                  [=](double const ratio) { barSet->append(ratio * 100.0); });

    auto* barSeries = new QHorizontalBarSeries();
    barSeries->append(barSet);

    auto* chart = new QChart();
    chart->addSeries(barSeries);
    chart->legend()->hide();
    chart->setTheme(QChart::ChartTheme::ChartThemeDark);
    chart->setMaximumWidth(450);
    chart->setMaximumHeight(150);
    chart->setMinimumHeight(150);
    chart->setContentsMargins(QMargins { 0, 0, 0, -20 });

    QStringList pcNames {};
    for (size_t i = varianceRatios.size(); i > 0; --i)
        pcNames.emplace_back(QString::fromStdString(std::format("PC{}", i)));
    auto* yAxis = new QBarCategoryAxis();
    yAxis->append(pcNames);
    chart->addAxis(yAxis, Qt::AlignLeft);
    barSeries->attachAxis(yAxis);

    auto* xAxis = new QValueAxis();
    xAxis->setRange(0.0, 100.0);
    xAxis->setTickType(QValueAxis::TickType::TicksFixed);
    xAxis->setTickCount(6);
    xAxis->setTitleText("Explained Variance (%)");
    chart->addAxis(xAxis, Qt::AlignBottom);
    barSeries->attachAxis(xAxis);

    auto* previousChart = ExplainedVarianceChartView->chart();
    ExplainedVarianceChartView->setChart(chart);
    delete previousChart;

    barSet->setSelectedColor(barSet->color().lighter());
    connect(barSet, &QBarSet::clicked, this, [this, barSet](int barIdx) {
        barSet->deselectAllBars();
        barSet->selectBar(barIdx);

        UpdatePrincipalAxesChart(barIdx);
    });
    connect(barSet, &QBarSet::hovered, this, [this](bool entered, int barIdx) {
        if (entered)
            setCursor(QCursor(Qt::PointingHandCursor));
        else
            setCursor(QCursor(Qt::ArrowCursor));
    });
}

auto PcaAnalysisDataWidget::UpdatePrincipalAxesChart(int barIdx) -> void {
    auto& explainedVarianceChart = *ExplainedVarianceChartView->chart();
    auto* varianceChartYAxis = dynamic_cast<QBarCategoryAxis*>(explainedVarianceChart.axes(Qt::Vertical).at(0));
    QString const pcName = varianceChartYAxis->at(barIdx);
    QString const title = QString("Most important features for %1").arg(pcName);

    size_t const forwardBarIdx = BatchData->PcaPrincipalAxes.size() - 1 - barIdx;
    std::vector<double> const& featureCoefficients = BatchData->PcaPrincipalAxes.at(forwardBarIdx);
    std::vector<std::string> const& featureNames = BatchData->FeatureNames;

    struct FeatureData {
        double Coefficient;
        std::reference_wrapper<std::string const> Name;
    };
    std::vector<FeatureData> featureData {};
    for (size_t i = 0; i < featureNames.size(); i++)
        featureData.emplace_back(featureCoefficients.at(i), featureNames.at(i));
    std::sort(featureData.begin(), featureData.end(),
              [](auto const& a, auto const& b) { return std::abs(a.Coefficient) > std::abs(b.Coefficient); });
    featureData.erase(std::next(featureData.begin(), 10), featureData.end());

    auto* barSet = new QBarSet(title);
    std::for_each(featureData.begin(), featureData.end(),
                  [=](auto const& feature) { barSet->append(feature.Coefficient); });

    auto* barSeries = new QBarSeries();
    barSeries->append(barSet);

    auto* chart = new QChart();
    chart->addSeries(barSeries);
    chart->legend()->hide();
    chart->setTheme(QChart::ChartTheme::ChartThemeDark);
    chart->setMaximumWidth(450);
    chart->setMaximumHeight(300);
    chart->setContentsMargins(QMargins { 0, 0, 0, -30 });
    chart->setMargins({ 10, 5, 5, 30 });

    auto* xAxis = new QBarCategoryAxis();
    std::for_each(featureData.begin(), featureData.end(),
                  [=](auto const& feature) { xAxis->append(QString::fromStdString(feature.Name)); });
    xAxis->setTitleText(title);
    xAxis->setLabelsAngle(-90);
    chart->addAxis(xAxis, Qt::AlignBottom);
    barSeries->attachAxis(xAxis);

    auto* yAxis = new QValueAxis();
    yAxis->setTitleText("Coefficients");
    chart->addAxis(yAxis, Qt::AlignLeft);
    barSeries->attachAxis(yAxis);
    yAxis->applyNiceNumbers();

    auto* previousChart = PrincipalAxesChartView->Widget().chart();
    PrincipalAxesChartView->Widget().setChart(chart);
    delete previousChart;

    PrincipalAxesChartView->ShowWidget();
}
