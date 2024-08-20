#include "PcaSecondaryChartWidget.h"

#include "ChartTooltip.h"
#include "../Utils/ScrollAwareChart.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChartView>
#include <QGraphicsLayout>
#include <QHorizontalBarSeries>
#include <QPieSeries>
#include <QValueAxis>

#include <numbers>


PcaSecondaryChartWidget::PcaSecondaryChartWidget() :
        QSplitter(Qt::Orientation::Vertical),
        BatchData(nullptr),
        ExplainedVarianceChartView(new PcaExplainedVariancePieChartView()),
        PrincipalAxesChartView(new OptionalWidget<PcaFeaturesChartView>("Please select a\nprincipal component",
                                                                        new PcaFeaturesChartView())) {

    setContentsMargins(0, 10, 0, 10);

    addWidget(ExplainedVarianceChartView);
    addWidget(PrincipalAxesChartView);

    ExplainedVarianceChartView->setContentsMargins(0, 0, 0, 5);
    PrincipalAxesChartView->setContentsMargins(0, 5, 0, 0);

    moveSplitter(ExplainedVarianceChartView->sizeHint().height() * 0.7, 1);

    connect(ExplainedVarianceChartView, &PcaExplainedVariancePieChartView::PrincipalComponentSelected,
            this, [this](QString const& pcName, size_t pcIdx) {

        assert(BatchData);

        bool const validPcSelected = pcIdx < BatchData->GetBatchWithPcaData().PcaExplainedVarianceRatios.size();
        if (validPcSelected) {
            PrincipalAxesChartView->Widget().UpdateData(BatchData, pcName, pcIdx);
            PrincipalAxesChartView->ShowWidget();
        } else
            PrincipalAxesChartView->HideWidget();
    });
}

auto PcaSecondaryChartWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchData = batchData;

    PrincipalAxesChartView->HideWidget();

    if (BatchData)
        ExplainedVarianceChartView->UpdateData(BatchData);
}



PcaExplainedVariancePieChartView::PcaExplainedVariancePieChartView() :
        QGraphicsView(new QGraphicsScene()),
        GraphicsScene(scene()),
        Chart(nullptr),
        Tooltip(nullptr) {

    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundRole(QPalette::Window);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
}

PcaExplainedVariancePieChartView::~PcaExplainedVariancePieChartView() { delete GraphicsScene; }

auto PcaExplainedVariancePieChartView::UpdateData(PipelineBatchListData const* batchListData) -> void {
    if (!batchListData)
        throw std::runtime_error("batch data must not be null");

    std::vector<double> const& varianceRatios = batchListData->GetBatchWithPcaData().PcaExplainedVarianceRatios;

    QList<QPieSlice*> slices;
    for (size_t i = 0; i < varianceRatios.size(); ++i) {
        QString const& sliceName = QString::fromStdString(std::format("PC{}", i + 1));
        double const sliceValue = varianceRatios.at(i) * 100.0;

        slices.emplace_back(new QPieSlice(sliceName, sliceValue));
    }
    double const totalExplainedVariance = std::reduce(varianceRatios.begin(), varianceRatios.end());
    double const unexplainedVariance = 1.0 - totalExplainedVariance;
    slices.emplace_back(new QPieSlice("other", unexplainedVariance * 100.0));

    auto* pieSeries = new QPieSeries();
    pieSeries->append(slices);
    pieSeries->setLabelsVisible(false);
    pieSeries->setPieSize(0.85);

    auto* chart = new QChart();
    chart->addSeries(pieSeries);
    chart->setTitle("Explained Variance (%)");
    chart->legend()->setAlignment(Qt::AlignmentFlag::AlignBottom);
    chart->legend()->layout()->setContentsMargins(0, 0, 0, 0);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setMargins({ 0, 0, 0, 0 });
    chart->setBackgroundRoundness(0.0);
    chart->setTheme(QChart::ChartTheme::ChartThemeDark);
    chart->setBackgroundBrush(Qt::BrushStyle::NoBrush);

    connect(pieSeries, &QPieSeries::clicked, this, [this, slices, batchListData](QPieSlice* clickedSlice) {
        for (auto* slice: slices)
            slice->setExploded(false);

        size_t const pcIdx = std::distance(slices.cbegin(), std::find(slices.cbegin(), slices.cend(), clickedSlice));

        bool const isValidSlice = pcIdx < batchListData->GetBatchWithPcaData().PcaExplainedVarianceRatios.size();
        clickedSlice->setExploded(isValidSlice);

        Q_EMIT PrincipalComponentSelected(clickedSlice->label(), pcIdx);
    });

    connect(pieSeries, &QPieSeries::hovered, this, [this](QPieSlice* /*slice*/, bool entered) {
        if (entered)
            setCursor(QCursor(Qt::PointingHandCursor));
        else
            setCursor(QCursor(Qt::ArrowCursor));
    });

    connect(pieSeries, &QPieSeries::hovered, this, &PcaExplainedVariancePieChartView::ToggleTooltip);

    delete Chart;

    Chart = chart;

    GraphicsScene->addItem(Chart);

    Chart->resize(size());
}

auto PcaExplainedVariancePieChartView::resizeEvent(QResizeEvent* event) -> void {
    GraphicsScene->setSceneRect(QRect(QPoint(0, 0), event->size()));
    if (Chart)
        Chart->resize(event->size());
}

void PcaExplainedVariancePieChartView::ToggleTooltip(QPieSlice* slice, bool entered) {
    if (!entered) {
        assert(Tooltip != nullptr);
        delete Tooltip;
        Tooltip = nullptr;
    } else if (!Tooltip) {  // after clicked during hover, hovered signal is emitted again
        QString const sliceName = slice->label();
        double const value = slice->value();

        double const minPlotAreaDim = std::min(Chart->plotArea().width(), Chart->plotArea().height());
        double const radius = slice->series()->pieSize() * minPlotAreaDim / 2.0;
        double const halfRadius = radius / 2.0;
        double const midSliceAngleDeg12 = slice->startAngle() + (slice->angleSpan() / 2);
        double const midSliceAngleDeg = midSliceAngleDeg12 + -90.0;
        double const midSliceAngle = midSliceAngleDeg * std::numbers::pi / 180.0;
        QPointF const xyNormalizedCoordinates { cos(midSliceAngle), sin(midSliceAngle) };
        QPointF const anchorPoint = Chart->rect().center() + (xyNormalizedCoordinates * halfRadius);

        QString const valueString = QString::fromStdString(std::format("{:.{}f}", value, 2));
        QString const text = QString("%1: %2 %").arg(sliceName).arg(valueString);

        auto penColor = QPen(slice->labelColor());
        auto brushColor = QBrush(palette().color(QPalette::ColorRole::Window));

        Tooltip = new ChartTooltip(*Chart, anchorPoint, text, { penColor, brushColor }, true);
        Tooltip->show();
    }
}



PcaFeaturesChartView::PcaFeaturesChartView() :
        QGraphicsView(new QGraphicsScene()),
        GraphicsScene(scene()),
        Chart(nullptr),
        Tooltip(nullptr) {

    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundRole(QPalette::Window);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
}

PcaFeaturesChartView::~PcaFeaturesChartView() { delete GraphicsScene; }

auto PcaFeaturesChartView::UpdateData(PipelineBatchListData const* batchListData,
                                      QString const& pcName,
                                      size_t pcIdx) -> void {
    if (!batchListData)
        return;

    QString const title = QString("Most important features for %1").arg(pcName);

    std::vector<double> const& featureCoefficients = batchListData->GetBatchWithPcaData().PcaPrincipalAxes.at(pcIdx);
    std::vector<std::string> const& featureNames = batchListData->FeatureNames;

    struct FeatureData {
        double Coefficient;
        std::reference_wrapper<std::string const> Name;
    };

    std::vector<FeatureData> featureData {};
    for (size_t i = 0; i < featureNames.size(); i++)
        featureData.emplace_back(featureCoefficients.at(i), featureNames.at(i));
    std::sort(featureData.begin(), featureData.end(),
              [](auto const& a, auto const& b) {
        bool const absoluteLessThan = std::abs(a.Coefficient) < std::abs(b.Coefficient);
        bool const absoluteEqual = std::abs(a.Coefficient) == std::abs(b.Coefficient);
        bool const lessThan = a.Coefficient < b.Coefficient;

        return absoluteLessThan || (absoluteEqual && lessThan);
    });

    auto* barSet = new QBarSet(title);
    for (int i = 0; i < featureData.size(); ++i) {
        double const feature = featureData.at(i).Coefficient;

        barSet->append(std::abs(feature));

        if (feature < 0.0)
            barSet->setBarSelected(i, true);
    }

    auto* barSeries = new QHorizontalBarSeries();
    barSeries->append(barSet);

    auto* chart = new ScrollAwareChart();
    chart->addSeries(barSeries);
    chart->setTitle(title);
    chart->legend()->hide();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setMargins({ 0, 0, 0, 0 });
    chart->setBackgroundRoundness(0.0);
    chart->setTheme(QChart::ChartTheme::ChartThemeDark);
    chart->setBackgroundBrush(Qt::BrushStyle::NoBrush);
    barSet->setSelectedColor(barSet->color().darker());

    auto* yAxis = new QBarCategoryAxis();
    for (auto& feature : featureData) {
        if (feature.Name.get().empty())
            throw std::runtime_error("invalid feature name");

        yAxis->append(QString::fromStdString(feature.Name));
    }
    static const int rangeSize = 10;
    QStringList yCategories = yAxis->categories();
    auto const rangeBeginIt = yCategories.size() > rangeSize
                                      ? std::prev(yCategories.end(), rangeSize)
                                      : yCategories.begin();
    yAxis->setRange(*rangeBeginIt, yCategories.back());

    auto labelFont = yAxis->labelsFont();
    labelFont.setPointSize(labelFont.pointSize() * 0.7);
    yAxis->setLabelsFont(labelFont);
    yAxis->setGridLineVisible(false);
    yAxis->setLineVisible(false);
    chart->addAxis(yAxis, Qt::AlignLeft);
    barSeries->attachAxis(yAxis);

    auto* xAxis = new QValueAxis();
    xAxis->setTitleText("Absolute Coefficients");
    chart->addAxis(xAxis, Qt::AlignBottom);
    barSeries->attachAxis(xAxis);
    xAxis->applyNiceNumbers();
    xAxis->setRange(0.0, xAxis->max());


    delete Chart;

    Chart = chart;

    GraphicsScene->addItem(Chart);

    Chart->resize(size());

    connect(barSet, &QBarSet::hovered, this, &PcaFeaturesChartView::ToggleTooltip);

    connect(Chart, &ScrollAwareChart::wheelRotated, this, &PcaFeaturesChartView::MoveRange);
}

auto PcaFeaturesChartView::resizeEvent(QResizeEvent* event) -> void {
    GraphicsScene->setSceneRect(QRect(QPoint(0, 0), event->size()));
    if (Chart)
        Chart->resize(event->size());
}

auto PcaFeaturesChartView::ToggleTooltip(bool entered, int barIdx) -> void {
    if (!entered) {
        delete Tooltip;
        Tooltip = nullptr;
    } else if (!Tooltip) {  // after clicked during hover, hovered signal is emitted again
        auto* yAxis = dynamic_cast<QBarCategoryAxis*>(Chart->axes(Qt::Vertical).at(0));
        QString const featureName = yAxis->at(barIdx);

        auto* barSeries = dynamic_cast<QHorizontalBarSeries*>(Chart->series().at(0));
        auto* barSet = barSeries->barSets().at(0);
        double const absoluteValue = barSet->at(barIdx);
        double const value = barSet->isBarSelected(barIdx) ? -absoluteValue : absoluteValue;

        QPointF const anchorPoint = { absoluteValue / 2.0, static_cast<qreal>(barIdx) };

        QString const valueString = QString::fromStdString(std::format("{:.{}f}", value, 2));
        QString const text = QString("%1: %2").arg(featureName).arg(valueString);

        auto penColor = QPen(Chart->axes(Qt::Orientation::Vertical).at(0)->labelsColor());
        auto brushColor = QBrush(palette().color(QPalette::ColorRole::Window));

        Tooltip = new ChartTooltip(*Chart, anchorPoint, text, { penColor, brushColor });
        Tooltip->show();
    }
}

auto PcaFeaturesChartView::MoveRange(int numberOfSteps) -> void {
    auto* categoryAxis = dynamic_cast<QBarCategoryAxis*>(Chart->axes(Qt::Orientation::Vertical).front());
    QStringList const categories = categoryAxis->categories();

    auto const minIt = std::find(categories.begin(), categories.end(), categoryAxis->min());
    auto const maxIt = std::find(categories.begin(), categories.end(), categoryAxis->max());

    int const minIdx = static_cast<int>(categories.indexOf(categoryAxis->min()));
    int const maxIdx = static_cast<int>(categories.indexOf(categoryAxis->max()));
    if (minIdx == -1 || maxIdx == -1)
        throw std::runtime_error("category not found");

    int const offset = std::clamp(numberOfSteps,
                                  -minIdx,
                                  static_cast<int>(categories.size()) - 1 - maxIdx);

    if (offset == 0)
        return;

    ToggleTooltip(false, -1);

    auto const newMinIt = std::next(minIt, offset);
    auto const newMaxIt = std::next(maxIt, offset);

    categoryAxis->setRange(*newMinIt, *newMaxIt);
}
