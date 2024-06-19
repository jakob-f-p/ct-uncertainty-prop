#include "ChartWidget.h"

#include "ChartTooltip.h"
#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QChart>
#include <QComboBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QScatterSeries>
#include <QStandardPaths>
#include <QValueAxis>


ChartWidget::ChartWidget(ChartView* chartView) :
        VLayout(new QVBoxLayout(this)),
        View(chartView),
        LightThemeButton([]() {
            auto* button = new QPushButton(GenerateSimpleIcon("LightMode"), "");
            button->setCheckable(true);
            button->setChecked(false);
            button->setMinimumWidth(static_cast<int>(static_cast<double>(button->sizeHint().width()) * 1.2));
            return button;
        }()),
        DarkThemeButton([]() {
            auto* button = new QPushButton(GenerateSimpleIcon("DarkMode"), "");
            button->setCheckable(true);
            button->setChecked(true);
            button->setMinimumWidth(static_cast<int>(static_cast<double>(button->sizeHint().width()) * 1.2));
            return button;
        }()),
        ThemeButtonGroup([this]() {
            auto* buttonGroup = new QButtonGroup();
            buttonGroup->addButton(LightThemeButton);
            buttonGroup->addButton(DarkThemeButton);
            return buttonGroup;
        }()),
        ExportButton(new QPushButton("Export")) {

    auto* actionBar = new QWidget();
    auto* actionBarHLayout = new QHBoxLayout(actionBar);
    auto* themeSelectionWidget = new QWidget();
    auto* themeSelectionHLayout = new QHBoxLayout(themeSelectionWidget);
    themeSelectionHLayout->addWidget(new QLabel("Theme: "));
    themeSelectionHLayout->addWidget(LightThemeButton);
    themeSelectionHLayout->addWidget(DarkThemeButton);
    actionBarHLayout->addWidget(themeSelectionWidget);
    actionBarHLayout->addStretch();
    actionBarHLayout->addWidget(ExportButton);
    VLayout->addWidget(actionBar);

    VLayout->addWidget(View);

    connect(ThemeButtonGroup, &QButtonGroup::buttonPressed, this, [this](QAbstractButton* button) {
        auto* pushButton = qobject_cast<QPushButton*>(button);
        ChartView::Theme const theme = [this, pushButton]() {
            if (pushButton == LightThemeButton)
                return ChartView::Theme::LIGHT;

            if (pushButton == DarkThemeButton)
                return ChartView::Theme::DARK;

            throw std::runtime_error("invalid theme");
        }();

        View->UpdateTheme(theme);
    });

    connect(ExportButton, &QPushButton::clicked, View, &ChartView::ExportChart);

    connect(View, &ChartView::SamplePointChanged, this, &ChartWidget::SamplePointChanged);
}

auto ChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    View->UpdateData(batchListData);
}

PcaChartWidget::PcaChartWidget() :
        ChartWidget(new PcaChartView()),
        SelectPipelineGroupBox(new QComboBox()) {

    auto* selectGroupWidget = new QWidget();
    auto* selectGroupFLayout = new QFormLayout(selectGroupWidget);
    selectGroupFLayout->setContentsMargins(5, 5, 5, 5);
    selectGroupFLayout->addRow("Select group", SelectPipelineGroupBox);

    VLayout->insertWidget(VLayout->count() - 1, selectGroupWidget);
}

auto PcaChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    BatchListData = batchListData;

    SelectPipelineGroupBox->clear();

    if (BatchListData) {
        for (auto const& data : batchListData->Data)
            SelectPipelineGroupBox->addItem(QString::fromStdString(data.Group.GetName()));

        connect(SelectPipelineGroupBox, &QComboBox::currentIndexChanged, this, [this, batchListData](int idx) {
            if (idx == -1)
                return;

            dynamic_cast<PcaChartView*>(View)->SetBatchData(&BatchListData->Data.at(idx));
            ChartWidget::UpdateData(batchListData);
        });

        SelectPipelineGroupBox->setCurrentIndex(-1);
        SelectPipelineGroupBox->setCurrentIndex(0);
    }

    ChartWidget::UpdateData(batchListData);
}

TsneChartWidget::TsneChartWidget() :
        ChartWidget(new TsneChartView()){}




ChartView::ChartView(QString const& chartName) :
        QGraphicsView(new QGraphicsScene()),
        ChartName(chartName),
        BatchListData(nullptr),
        GraphicsScene(scene()),
        Chart(nullptr),
        Tooltip(nullptr),
        CurrentTheme(Theme::DARK) {

    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundRole(QPalette::Window);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
}

ChartView::~ChartView() {
    delete GraphicsScene;
}

auto ChartView::UpdateData(PipelineBatchListData const* batchListData) -> void {
    BatchListData = batchListData;

    if (!BatchListData)
        return;

    auto const scatterSeriesVector = CreateScatterSeries();
    ConnectScatterSeries(scatterSeriesVector);

    auto* chart = new QChart();
    chart->setTitle(ChartName);
    for (auto* scatterSeries : scatterSeriesVector)
        chart->addSeries(scatterSeries);
    chart->createDefaultAxes();
    chart->legend()->setAlignment(Qt::AlignBottom);

    auto* xAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    auto* yAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    xAxis->setTruncateLabels(false);
    yAxis->setTruncateLabels(false);
    auto [ xMin, xMax ] = GetAxisRange(xAxis);
    auto [ yMin, yMax ] = GetAxisRange(yAxis);
    xAxis->setRange(xMin, xMax);
    yAxis->setRange(yMin, yMax);
    xAxis->setTickType(QValueAxis::TickType::TicksDynamic);
    yAxis->setTickType(QValueAxis::TickType::TicksDynamic);
    xAxis->setTickInterval(GetAxisTickInterval(xAxis));
    yAxis->setTickInterval(GetAxisTickInterval(yAxis));

    if (Chart) {
        GraphicsScene->removeItem(Chart);
        delete Chart;
    }
    Chart = chart;

    GraphicsScene->addItem(Chart);
    UpdateTheme(Theme::DARK);

    Chart->resize(size());
}

auto ChartView::ExportChart() -> void {
    if (!Chart)
        throw std::runtime_error("chart must not be nullptr");

    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const fileFilter = "Images (*.png)";
    QString const caption = QString("Save %1").arg(ChartName);
    QString const fileName = QFileDialog::getSaveFileName(this, caption, homePath, fileFilter);
    grab().save(fileName);
}

auto ChartView::UpdateTheme(Theme theme) -> void {
    if (!Chart)
        throw std::runtime_error("chart must not be nullptr");

    CurrentTheme = theme;
    switch (CurrentTheme) {
        case Theme::LIGHT: {
            Chart->setTheme(QChart::ChartTheme::ChartThemeLight);

            auto seriesList = Chart->series();
            for (auto* series : seriesList) {
                auto* scatterSeries = qobject_cast<QScatterSeries*>(series);
                scatterSeries->setSelectedColor(scatterSeries->color().darker());
            }
            break;
        }

        case Theme::DARK: {
            Chart->setTheme(QChart::ChartTheme::ChartThemeDark);
            Chart->setBackgroundBrush(Qt::BrushStyle::NoBrush);

            auto seriesList = Chart->series();
            for (auto* series : seriesList) {
                auto* scatterSeries = qobject_cast<QScatterSeries*>(series);
                scatterSeries->setSelectedColor(scatterSeries->color().lighter());
            }
            break;
        }

        default: throw std::runtime_error("invalid theme");
    }

    static auto const titlePointSize = static_cast<int>(static_cast<double>(Chart->titleFont().pointSize()) * 1.5);
    auto titleFont = Chart->titleFont();
    titleFont.setPointSize(titlePointSize);
    Chart->setTitleFont(titleFont);
}

auto ChartView::resizeEvent(QResizeEvent* event) -> void {
    GraphicsScene->setSceneRect(QRect(QPoint(0, 0), event->size()));
    if (Chart)
        Chart->resize(event->size());
}

void ChartView::ToggleTooltip(QPointF const& point, bool entered) {
    if (!entered) {
        assert(Tooltip != nullptr);

        GraphicsScene->removeItem(Tooltip);
        delete Tooltip;
        Tooltip = nullptr;
    } else if (!Tooltip) {  // after clicked during hover, hovered signal is emitted again
        Tooltip = new ChartTooltip(*Chart, point, GetCurrentForegroundBackground());
        Tooltip->show();
    }
}

auto ChartView::GetAxisRange(QValueAxis* axis) -> std::pair<double, double> {
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

auto ChartView::GetAxisTickInterval(QValueAxis* axis) -> double {
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

auto ChartView::GetCurrentForegroundBackground() const -> PenBrushPair {
    if (!Chart)
        throw std::runtime_error("Chart must not be null");

    auto penColor = QPen(Chart->axes(Qt::Orientation::Horizontal).at(0)->labelsColor());
    auto brushColor = [this]() {
        switch (CurrentTheme) {
            case Theme::LIGHT: return Chart->backgroundBrush();
            case Theme::DARK:  return QBrush(palette().color(QPalette::ColorRole::Window));
            default: throw std::runtime_error("Invalid theme");
        }
    }();

    return { penColor, brushColor };
}

auto ChartView::ConnectScatterSeries(std::vector<QScatterSeries*> const& scatterSeriesVector) noexcept -> void {
    for (int i = 0; i < scatterSeriesVector.size(); i++) {
        auto* scatterSeries = scatterSeriesVector[i];

        connect(scatterSeries, &QScatterSeries::hovered, this, &ChartView::ToggleTooltip);

        connect(scatterSeries, &QScatterSeries::clicked, this,
                [this, scatterSeries, i, scatterSeriesVector](QPointF const& point) {

            auto points = scatterSeries->points();
            auto it = std::find(points.cbegin(), points.cend(), point);
            if (it == points.cend())
                throw std::runtime_error("point not present in chart");

            auto idx = static_cast<uint16_t>(std::distance(points.cbegin(), it));

            for (auto* series : scatterSeriesVector)
                series->deselectAllPoints();
            scatterSeries->selectPoint(idx);

            Q_EMIT SamplePointChanged(SampleId { static_cast<uint16_t>(i), idx });
        });
    }
}


PcaChartView::PcaChartView()
        : ChartView("PCA") {}

auto PcaChartView::SetBatchData(PipelineBatchData const* batchData) noexcept -> void {
    BatchData = batchData;
}

auto PcaChartView::CreateScatterSeries() noexcept -> std::vector<QScatterSeries*> {
    auto* scatterSeries = new QScatterSeries();
    scatterSeries->setName(QString::fromStdString(BatchData->Group.GetName()));
    scatterSeries->setMarkerSize(scatterSeries->markerSize() * 0.5);

    QList<QPointF> points;
    std::transform(BatchData->StateDataList.cbegin(), BatchData->StateDataList.cend(),
                   std::back_inserter(points),
                   [](auto const& psData) {
                       return QPointF(psData.PcaCoordinates.at(0), psData.PcaCoordinates.at(1));
                   });
    scatterSeries->append(points);

    return { scatterSeries };
}

TsneChartView::TsneChartView()
        : ChartView("t-SNE") {}

auto TsneChartView::CreateScatterSeries() noexcept -> std::vector<QScatterSeries*> {
    std::vector<QScatterSeries*> scatterSeriesVector;
    scatterSeriesVector.reserve(BatchListData->Data.size());

    for (auto const& batchData : BatchListData->Data) {
        auto* scatterSeries = new QScatterSeries();
        scatterSeries->setName(QString::fromStdString(batchData.Group.GetName()));
        scatterSeries->setMarkerSize(scatterSeries->markerSize() * 0.75);

        QList<QPointF> points;
        std::transform(batchData.StateDataList.cbegin(), batchData.StateDataList.cend(),
                       std::back_inserter(points),
                       [](auto const& psData) {
                           return QPointF(psData.TsneCoordinates.at(0), psData.TsneCoordinates.at(1));
                       });
        scatterSeries->append(points);

        scatterSeriesVector.push_back(scatterSeries);
    }

    return scatterSeriesVector;
}
