#include "PcaSecondaryChartWidget.h"

#include "ChartTooltip.h"
#include "../Utils/ScrollAwareChart.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QApplication>
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

    setContentsMargins({});

    addWidget(ExplainedVarianceChartView);
    addWidget(PrincipalAxesChartView);

    ExplainedVarianceChartView->setContentsMargins(0, 0, 0, 10);
    PrincipalAxesChartView->setContentsMargins(0, 10, 0, 0);

    setBaseSize(200, sizeHint().height());

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

auto PcaSecondaryChartWidget::showEvent(QShowEvent* event) -> void {
    QSplitter::showEvent(event);

    moveSplitter(ExplainedVarianceChartView->sizeHint().height() * 0.35, 1);
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
            QApplication::setOverrideCursor(QCursor { Qt::CursorShape::PointingHandCursor });
        else
            QApplication::restoreOverrideCursor();
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

    PcName = pcName;

    std::vector<double> const& featureCoefficients = batchListData->GetBatchWithPcaData().PcaPrincipalAxes.at(pcIdx);
    std::vector<std::string> const& featureNames = batchListData->FeatureNames;

    Features.clear();
    for (size_t i = 0; i < featureNames.size(); i++)
        Features.emplace_back(featureCoefficients.at(i), featureNames.at(i));

    std::sort(Features.begin(), Features.end(),
              [](auto const& a, auto const& b) {
        bool const absoluteLessThan = std::abs(a.Coefficient) < std::abs(b.Coefficient);
        bool const absoluteEqual = std::abs(a.Coefficient) == std::abs(b.Coefficient);
        bool const lessThan = a.Coefficient < b.Coefficient;

        return absoluteLessThan || (absoluteEqual && lessThan);
    });

    static const int rangeSize = 10;
    auto const rangeBeginIt = Features.size() > rangeSize
                                      ? std::prev(Features.end(), rangeSize)
                                      : Features.begin();
    UpdateChart({ rangeBeginIt, Features.end() });
}

auto PcaFeaturesChartView::resizeEvent(QResizeEvent* event) -> void {
    QGraphicsView::resizeEvent(event);

    GraphicsScene->setSceneRect(QRect(QPoint(0, 0), event->size()));

    if (Chart)
        ResizeChart(event->size());
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

    static constexpr auto findFeatureName = [](QString const& featureNameToFind) {
        return [name = featureNameToFind.toStdString()](Feature const& feature) { return feature.Name.get() == name; };
    };
    auto const minIt = std::find_if(Features.begin(), Features.end(), findFeatureName(categoryAxis->min()));
    auto const maxIt = std::find_if(Features.begin(), Features.end(), findFeatureName(categoryAxis->max()));
    if (minIt == Features.end() || maxIt == Features.end())
        throw std::runtime_error("category not found");

    int const minIdx = static_cast<int>(std::distance(Features.begin(), minIt));
    int const maxIdx = static_cast<int>(std::distance(Features.begin(), maxIt));

    int const offset = std::clamp(numberOfSteps,
                                  -minIdx,
                                  static_cast<int>(Features.size()) - 1 - maxIdx);

    if (offset == 0)
        return;

    ToggleTooltip(false, -1);

    auto const newMinIt = std::next(minIt, offset);
    auto const newMaxIt = std::next(maxIt, offset);

    UpdateChart({ newMinIt, std::next(newMaxIt) });
}

auto PcaFeaturesChartView::UpdateChart(std::span<Feature> visibleFeatures) -> void {
    QString const title = QString("Most important features for %1").arg(PcName);

    bool const containsLargest
            = std::find_if(visibleFeatures.begin(), visibleFeatures.end(),
                           [this](Feature const feature) { return feature.Name.get() == Features.back().Name.get(); })
                    != visibleFeatures.end();

    auto* barSet = new QBarSet(title);
    QList<int> negativeBarsIndices;
    for (int i = 0; i < visibleFeatures.size(); ++i) {
        double const& featureValue = visibleFeatures[i].Coefficient;

        barSet->append(std::abs(featureValue));

        if (featureValue < 0.0)
            negativeBarsIndices.emplace_back(i);
    }
    if (!containsLargest)
        barSet->append(Features.back().Coefficient);

    barSet->selectBars(negativeBarsIndices);

    auto* barSeries = new QHorizontalBarSeries();
    barSeries->append(barSet);

    auto* chart = new ScrollAwareChart();
    chart->addSeries(barSeries);
    chart->legend()->hide();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setMargins({ 0, 0, 0, 0 });
    chart->setBackgroundRoundness(0.0);
    chart->setTheme(QChart::ChartTheme::ChartThemeDark);
    chart->setBackgroundBrush(Qt::BrushStyle::NoBrush);
    barSet->setSelectedColor(barSet->color().darker());

    auto* yAxis = new QBarCategoryAxis();
    for (auto& feature : visibleFeatures) {
        if (feature.Name.get().empty())
            throw std::runtime_error("invalid feature name");

        yAxis->append(QString::fromStdString(feature.Name));
    }
    if (!containsLargest)
        yAxis->append(QString::fromStdString(Features.back().Name.get()));

    auto labelFont = yAxis->labelsFont();
    labelFont.setPointSize(labelFont.pointSize() * 0.7);
    yAxis->setLabelsFont(labelFont);
    yAxis->setGridLineVisible(false);
    yAxis->setLineVisible(false);
    chart->addAxis(yAxis, Qt::AlignLeft);
    barSeries->attachAxis(yAxis);

    auto* xAxis = new QValueAxis();
    xAxis->setTitleText("Absolute Feature Coefficients");
    auto titleFont = xAxis->titleFont();
    static auto const titlePointSize = static_cast<int>(static_cast<double>(titleFont.pointSize()) * 0.9);
    titleFont.setPointSize(titlePointSize);
    titleFont.setWeight(QFont::Weight::Normal);
    xAxis->setTitleFont(titleFont);
    chart->addAxis(xAxis, Qt::AlignBottom);
    barSeries->attachAxis(xAxis);

    xAxis->applyNiceNumbers();
    if (!containsLargest) {
        barSet->remove(barSet->count() - 1);
        yAxis->remove(QString::fromStdString(Features.back().Name.get()));
    }


    delete Chart;

    Chart = chart;

    GraphicsScene->addItem(Chart);

    ResizeChart(size());

    connect(barSet, &QBarSet::hovered, this, &PcaFeaturesChartView::ToggleTooltip);

    connect(Chart, &ScrollAwareChart::wheelRotated, this, &PcaFeaturesChartView::MoveRange);
}

auto PcaFeaturesChartView::ResizeChart(QSize size) -> void {
    static constexpr qreal plotAreaFraction = 0.7;

    qreal const leftMargin = size.width() * (1 - plotAreaFraction);
    qreal const bottomMargin = 50.0;
    qreal const newPlotAreaWidth = size.width() * plotAreaFraction - 15.0;
    qreal const height = size.height() - bottomMargin;
    Chart->setPlotArea({ leftMargin + 0.5, 0.5, newPlotAreaWidth, height });
    Chart->setMargins({ static_cast<int>(leftMargin), 0, 0, static_cast<int>(bottomMargin) });

    Chart->resize(size);

    auto* categoryAxis = dynamic_cast<QBarCategoryAxis*>(Chart->axes(Qt::Orientation::Vertical).at(0));
    categoryAxis->append("VeryVeryVeryVeryVeryVeryLongLabelToInvalidateAxisGeometry");
    categoryAxis->remove("VeryVeryVeryVeryVeryVeryLongLabelToInvalidateAxisGeometry");
}
