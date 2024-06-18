#include "TsneChartWidget.h"

#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QChart>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QScatterSeries>
#include <QStandardPaths>
#include <QValueAxis>

#include <utility>


TsneChartWidget::TsneChartWidget() :
        ChartView(new TsneChartView()),
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

    auto* vLayout = new QVBoxLayout(this);

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
    vLayout->addWidget(actionBar);

    vLayout->addWidget(ChartView);

    connect(ThemeButtonGroup, &QButtonGroup::buttonPressed, this, [this](QAbstractButton* button) {
        auto* pushButton = qobject_cast<QPushButton*>(button);
        TsneChartView::Theme const theme = [this, pushButton]() {
            if (pushButton == LightThemeButton)
                return TsneChartView::Theme::LIGHT;

            if (pushButton == DarkThemeButton)
                return TsneChartView::Theme::DARK;

            throw std::runtime_error("invalid theme");
        }();

        ChartView->UpdateTheme(theme);
    });

    connect(ExportButton, &QPushButton::clicked, ChartView, &TsneChartView::ExportChart);

    connect(ChartView, &TsneChartView::SamplePointChanged, this, &TsneChartWidget::SamplePointChanged);
}

auto TsneChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    ChartView->UpdateData(batchListData);
}

TsneChartView::TsneChartView() :
        QGraphicsView(new QGraphicsScene()),
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

TsneChartView::~TsneChartView() {
    delete GraphicsScene;
}

auto TsneChartView::UpdateData(PipelineBatchListData const* batchListData) -> void {
    BatchListData = batchListData;

    if (!BatchListData)
        return;

    auto const scatterSeriesVector = CreateScatterSeries();

    auto* chart = new QChart();
    chart->setTitle("t-SNE analysis");
    for (auto* scatterSeries : scatterSeriesVector)
        chart->addSeries(scatterSeries);
    chart->createDefaultAxes();
    chart->legend()->setAlignment(Qt::AlignBottom);

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

    if (Chart) {
        GraphicsScene->removeItem(Chart);
        delete Chart;
    }
    Chart = chart;

    GraphicsScene->addItem(Chart);
    UpdateTheme(Theme::DARK);

    Chart->resize(size());
}

auto TsneChartView::ExportChart() -> void {
    if (!Chart)
        throw std::runtime_error("chart must not be nullptr");

    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const fileFilter = "Images (*.png)";
    QString const fileName = QFileDialog::getSaveFileName(this, "Save t-SNE chart", homePath, fileFilter);
    grab().save(fileName);
}

auto TsneChartView::UpdateTheme(Theme theme) -> void {
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

auto TsneChartView::resizeEvent(QResizeEvent* event) -> void {
    GraphicsScene->setSceneRect(QRect(QPoint(0, 0), event->size()));
    Chart->resize(event->size());
}

void TsneChartView::ToggleTooltip(QPointF const& point, bool entered) {
    if (!entered) {
        assert(Tooltip != nullptr);

        GraphicsScene->removeItem(Tooltip);
        delete Tooltip;
        Tooltip = nullptr;
    } else if (!Tooltip) {  // after clicked during hover, hovered signal is emitted again
        Tooltip = new TsneChartTooltip(*Chart, point, GetCurrentForegroundBackground());
        Tooltip->show();
    }
}

auto TsneChartView::GetAxisRange(QValueAxis* axis) -> std::pair<int, int> {
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

auto TsneChartView::GetAxisTickInterval(QValueAxis* axis) -> double {
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

    for (int i = 0; i < scatterSeriesVector.size(); i++) {
        auto* scatterSeries = scatterSeriesVector[i];

        connect(scatterSeries, &QScatterSeries::hovered, this, &TsneChartView::ToggleTooltip);

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

    return scatterSeriesVector;
}

auto TsneChartView::GetCurrentForegroundBackground() const -> TsneChartView::PenBrushPair {
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



TsneChartTooltip::TsneChartTooltip(QChart& parentChart,
                                   QPointF anchorPoint,
                                   TsneChartView::PenBrushPair const& foregroundBackgroundColors) :
        QGraphicsItem(&parentChart),
        Chart(&parentChart),
        Anchor(anchorPoint),
        Text([this]() {
            uint16_t const numberOfNonDecimals = std::floor(
                    std::max({ std::log10(std::abs(Anchor.x())) + 1, std::log10(std::abs(Anchor.y())) }) + 1);
            uint16_t const numberOfDecimals = 2;
            uint16_t const formatWidth = numberOfNonDecimals + numberOfDecimals + 2;
            std::string const tooltipString = std::format("x: {:{}.{}f}\ny: {:{}.{}f}",
                                                          Anchor.x(), formatWidth, numberOfDecimals, Anchor.y(),
                                                          formatWidth, numberOfDecimals);

            return QString::fromStdString(tooltipString);
        }()),
        Font(QFontDatabase::systemFont(QFontDatabase::FixedFont)),
        TextPen(foregroundBackgroundColors.Pen),
        BackgroundBrush(foregroundBackgroundColors.Brush) {

    QFontMetrics const metrics { Font };
    TextRectangle = metrics.boundingRect(QRect(0, 0, 150, 150), Qt::AlignLeft, Text);
    TextRectangle.translate(5, 5);
    Rectangle = TextRectangle.adjusted(-5, -5, 5, 5);

    setZValue(11);

    AdjustPosition();
}

auto TsneChartTooltip::boundingRect() const -> QRectF {
    QPointF const anchor = mapFromParent(Chart->mapToPosition(Anchor));

    return QRectF { QPointF { qMin(Rectangle.left(), anchor.x()), qMin(Rectangle.top(), anchor.y()) },
                    QPointF { qMax(Rectangle.right(), anchor.x()), qMax(Rectangle.bottom(), anchor.y()) } };
}

void TsneChartTooltip::paint(QPainter* painter, QStyleOptionGraphicsItem const* /*option*/, QWidget* /*widget*/) {
    QPainterPath const tooltipPath = GetTooltipPath();

    painter->setPen(TextPen);
    painter->setFont(Font);

    painter->setBrush(BackgroundBrush);
    painter->drawPath(tooltipPath);

    painter->drawText(TextRectangle, Text);
}

void TsneChartTooltip::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    event->ignore();
}

auto TsneChartTooltip::GetTooltipPath() const -> QPainterPath {
    QPainterPath path;
    path.addRoundedRect(Rectangle, 5, 5);

    QPointF const anchor = mapFromParent(Chart->mapToPosition(Anchor));
    if (!Rectangle.contains(anchor) && !Anchor.isNull()) {
        // establish the position of the anchor point in relation to Rectangle
        bool const above = anchor.y() <= Rectangle.top();
        bool const aboveCenter = anchor.y() > Rectangle.top() && anchor.y() <= Rectangle.center().y();
        bool const belowCenter = anchor.y() > Rectangle.center().y() && anchor.y() <= Rectangle.bottom();
        bool const below = anchor.y() > Rectangle.bottom();

        bool const onLeft = anchor.x() <= Rectangle.left();
        bool const leftOfCenter = anchor.x() > Rectangle.left() && anchor.x() <= Rectangle.center().x();
        bool const rightOfCenter = anchor.x() > Rectangle.center().x() && anchor.x() <= Rectangle.right();
        bool const onRight = anchor.x() > Rectangle.right();

        // get the nearest Rectangle corner.
        qreal const x = onRight || rightOfCenter
                        ? Rectangle.width()
                        : 0;
        qreal const y = below || belowCenter
                        ? Rectangle.height()
                        : 0;
        bool const cornerCase = (above && onLeft) || (above && onRight) || (below && onLeft) || (below && onRight);
        bool const vertical = qAbs(anchor.x() - x) > qAbs(anchor.y() - y);

        qreal const x1 = x + leftOfCenter * 10 - rightOfCenter * 20 + cornerCase * !vertical * (onLeft * 10 - onRight * 20);
        qreal const y1 = y + aboveCenter * 10 - belowCenter * 20 + cornerCase * vertical * (above * 10 - below * 20);
        QPointF const point1 { x1, y1 };

        qreal const x2 = x + leftOfCenter * 20 - rightOfCenter * 10 + cornerCase * !vertical * (onLeft * 20 - onRight * 10);
        qreal const y2 = y + aboveCenter * 20 - belowCenter * 10 + cornerCase * vertical * (above * 20 - below * 10);
        QPointF const point2 { x2, y2 };

        path.moveTo(point1);
        path.lineTo(anchor);
        path.lineTo(point2);
        path = path.simplified();
    }

    return path;
}

auto TsneChartTooltip::AdjustPosition() noexcept -> void {
    prepareGeometryChange();
    setPos(Chart->mapToPosition(Anchor) + QPoint(10, -50));

    auto rect = mapRectToParent(boundingRect());
    auto parentRect = parentItem()->boundingRect();

    if (!parentRect.contains(rect)) {
        if (parentRect.top() > rect.top())
            moveBy(0, rect.top() - parentRect.top());

        if (parentRect.bottom() < rect.bottom())
            moveBy(0, parentRect.bottom() - rect.bottom());

        if (parentRect.left() > rect.left())
            moveBy(rect.left() - parentRect.left(), 0);

        if (parentRect.right() < rect.right())
            moveBy(parentRect.right() - rect.right(), 0);
    }

    auto newRect = mapRectToParent(boundingRect());
    if (!parentRect.contains(rect))
        qWarning("Tooltip to large for chart");
}
