#include "MainChartWidget.h"

#include "ChartLassoSelectionTool.h"
#include "ChartTooltip.h"
#include "../Utils/NameLineEdit.h"
#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QChart>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGraphicsLayout>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QScatterSeries>
#include <QStandardPaths>
#include <QValueAxis>

#include <ranges>


MainChartWidget::MainChartWidget(ChartView* chartView) :
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
            auto* buttonGroup = new QButtonGroup(this);
            buttonGroup->addButton(LightThemeButton);
            buttonGroup->addButton(DarkThemeButton);
            return buttonGroup;
        }()),
        ExportButton(new QPushButton("Export")) {

    VLayout->setSpacing(20);
    VLayout->setContentsMargins({});

    VLayout->addWidget(View);

    auto* actionBar = new QWidget();
    auto* actionBarHLayout = new QHBoxLayout(actionBar);
    actionBarHLayout->setContentsMargins({});
    auto* themeSelectionWidget = new QWidget();
    auto* themeSelectionHLayout = new QHBoxLayout(themeSelectionWidget);
    themeSelectionHLayout->setContentsMargins({});
    themeSelectionHLayout->addWidget(new QLabel("Theme: "));
    themeSelectionHLayout->addWidget(LightThemeButton);
    themeSelectionHLayout->addWidget(DarkThemeButton);
    actionBarHLayout->addWidget(themeSelectionWidget);
    actionBarHLayout->addStretch();
    actionBarHLayout->addSpacing(15);
    actionBarHLayout->addWidget(ExportButton);
    VLayout->addWidget(actionBar);

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

    connect(View, &ChartView::SamplePointChanged, this, &MainChartWidget::SamplePointChanged);
}

auto MainChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    View->UpdateData(batchListData);
}

PcaMainChartWidget::PcaMainChartWidget() :
        MainChartWidget(new PcaChartView()),
        SelectPointSetComboBox(new QComboBox()) {

    auto* fLayout = new QFormLayout();
    fLayout->setContentsMargins({});
    fLayout->addRow("Select set", SelectPointSetComboBox);

    VLayout->insertLayout(0, fLayout);
}

PcaMainChartWidget::~PcaMainChartWidget() = default;

auto PcaMainChartWidget::UpdateData(PipelineBatchListData const* batchListData) -> void {
    BatchListData = batchListData;

    SelectPointSetComboBox->clear();
    SelectPointSetComboBox->disconnect();
    PcaSelectionMap.clear();

    if (BatchListData) {
        for (int i = 0; i < batchListData->Data.size(); i++) {
            auto const& data = batchListData->Data[i];

            QString const groupName = QString::fromStdString(data.Group.GetName());

            SelectPointSetComboBox->addItem(groupName);
            PcaSelectionMap.emplace(groupName, std::make_unique<PipelineBatchListData>(batchListData->TrimTo(i)));
        }

        connect(SelectPointSetComboBox, &QComboBox::currentTextChanged,
                this, [this](QString const& text) {
                    if (text.isEmpty())
                        return;

                    auto const* data = PcaSelectionMap.at(text).get();
                    MainChartWidget::UpdateData(data);

                    Q_EMIT PcaDataChanged(data);
                });

        SelectPointSetComboBox->setCurrentIndex(-1);
        SelectPointSetComboBox->setCurrentIndex(0);
    }

    MainChartWidget::UpdateData(batchListData);
}

auto PcaMainChartWidget::SelectPcaPoints(QString const& name, QList<QPointF> const& points) -> void {
    auto&& data = BatchListData->TrimTo(points, PipelineBatchListData::AnalysisType::TSNE);
    PipelineGroupList const& groupList = data.GroupList;
    auto&& trimmedData = PipelineGroupList::DoPCAForSubset(data);

    PcaSelectionMap.emplace(name, std::make_unique<PipelineBatchListData>(trimmedData));

    if (SelectPointSetComboBox->findText(name) == -1)
        SelectPointSetComboBox->addItem(name);

    SelectPointSetComboBox->setCurrentText(name);
}

TsneMainChartWidget::TsneMainChartWidget() :
        MainChartWidget(new TsneChartView()) {

    connect(dynamic_cast<TsneChartView*>(View), &TsneChartView::PointsSelected,
            this, &TsneMainChartWidget::OnPointsSelected);
}

void TsneMainChartWidget::OnPointsSelected(QList<QPointF> const& points) {
    auto* dialog = new QDialog();
    dialog->setWindowTitle("Selection for PCA");
    auto* fLayout = new QFormLayout(dialog);
    fLayout->setAlignment(Qt::AlignTop);

    auto* nameEdit = new NameLineEdit();
    fLayout->addRow("Set name", nameEdit);

    fLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding));

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(dialogButtonBar, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(dialogButtonBar, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    fLayout->addRow(dialogButtonBar);

    connect(dialog, &QDialog::accepted, this, [this, dialog, &points]() {
        QString const name = dialog->findChild<NameLineEdit*>()->GetText();

        if (!name.isEmpty())
                Q_EMIT PcaPointsSelected(name, points);
    });

    dialog->exec();
}


ChartView::ChartView(QString const& chartName) :
        QGraphicsView(new QGraphicsScene()),
        BatchListData(nullptr),
        Chart(nullptr),
        GraphicsScene(scene()),
        ChartName(chartName),
        Tooltip(nullptr),
        CurrentTheme(Theme::DARK) {

    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundRole(QPalette::Window);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
    setContentsMargins({});
}

ChartView::~ChartView() {
    delete GraphicsScene;
}

auto ChartView::UpdateData(PipelineBatchListData const* batchListData) -> void {
    BatchListData = batchListData;

    if (!BatchListData)
        return;

    auto const indexScatterSeriesMap = CreateScatterSeries();

    ConnectScatterSeries(indexScatterSeriesMap);

    auto* chart = new QChart();
    chart->setTitle(ChartName);
    for (auto* scatterSeries : std::views::values(indexScatterSeriesMap))
        chart->addSeries(scatterSeries);
    chart->createDefaultAxes();
    chart->legend()->setAlignment(Qt::AlignBottom);

    if (!chart->series().isEmpty()) {
        auto* xAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
        auto* yAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
        xAxis->setTruncateLabels(false);
        yAxis->setTruncateLabels(false);
        auto [xMin, xMax] = GetAxisRange(xAxis);
        auto [yMin, yMax] = GetAxisRange(yAxis);
        xAxis->setRange(xMin, xMax);
        yAxis->setRange(yMin, yMax);
        xAxis->setTickType(QValueAxis::TickType::TicksDynamic);
        yAxis->setTickType(QValueAxis::TickType::TicksDynamic);
        xAxis->setTickInterval(GetAxisTickInterval(xAxis));
        yAxis->setTickInterval(GetAxisTickInterval(yAxis));
//        xAxis->applyNiceNumbers();
//        yAxis->applyNiceNumbers();
        EditAxes(xAxis, yAxis);
    }

    if (Chart) {
        RemoveItemsFromSceneOnUpdateData();
        delete Chart;
    }
    Chart = chart;

    GraphicsScene->addItem(Chart);
    UpdateTheme(Theme::DARK);
    AddItemsFromSceneOnUpdateData();

    Chart->resize(size());
}

auto ChartView::RemoveItemsFromSceneOnUpdateData() -> void {}

auto ChartView::AddItemsFromSceneOnUpdateData() -> void {}

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
                scatterSeries->setSelectedColor(scatterSeries->color().lighter());
                scatterSeries->setBorderColor(QColor { "black" });
            }
            break;
        }

        case Theme::DARK: {
            Chart->setTheme(QChart::ChartTheme::ChartThemeDark);
            Chart->setBackgroundBrush(Qt::BrushStyle::NoBrush);

            auto seriesList = Chart->series();
            for (auto* series : seriesList) {
                auto* scatterSeries = qobject_cast<QScatterSeries*>(series);
                scatterSeries->setSelectedColor(scatterSeries->color().darker());
                scatterSeries->setBorderColor(QColor { "white" });
            }
            break;
        }

        default: throw std::runtime_error("invalid theme");
    }

    auto seriesList = Chart->series();
    for (auto* series : seriesList) {
        auto* scatterSeries = qobject_cast<QScatterSeries*>(series);

        static const qreal markerSize = scatterSeries->markerSize() * 0.35;
        scatterSeries->setMarkerSize(markerSize);

        static const qreal penSize = scatterSeries->pen().widthF() * 0.25;
        auto thinnerPen = scatterSeries->pen();
        thinnerPen.setWidthF(penSize);
        scatterSeries->setPen(thinnerPen);
    }

    static auto const titlePointSize = static_cast<int>(static_cast<double>(Chart->titleFont().pointSize()) * 1.3);
    auto titleFont = Chart->titleFont();
    titleFont.setPointSize(titlePointSize);
    Chart->setTitleFont(titleFont);

    Chart->layout()->setContentsMargins(0, 0, 0, 0);
    Chart->setBackgroundRoundness(0.0);
    Chart->setMargins({});
    Chart->legend()->layout()->setContentsMargins(0, 0, 0, 0);
}

auto ChartView::resizeEvent(QResizeEvent* event) -> void {
    GraphicsScene->setSceneRect(QRect(QPoint(0, 0), event->size()));
    if (Chart)
        Chart->resize(event->size());
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

void ChartView::ToggleTooltip(QPointF const& point, bool entered) {
    if (!entered) {
        assert(Tooltip != nullptr);

        GraphicsScene->removeItem(Tooltip);
        delete Tooltip;
        Tooltip = nullptr;
    } else if (!Tooltip) {  // after clicked during hover, hovered signal is emitted again

        uint16_t const numberOfNonDecimals = std::floor(
                std::max({ std::log10(std::abs(point.x())), std::log10(std::abs(point.y())) }) + 1);
        uint16_t const numberOfDecimals = 2;
        uint16_t const formatWidth = numberOfNonDecimals + numberOfDecimals + 2;
        std::string const tooltipString = std::format("x: {:{}.{}f}\ny: {:{}.{}f}",
                                                      point.x(), formatWidth, numberOfDecimals, point.y(),
                                                      formatWidth, numberOfDecimals);

        Tooltip = new ChartTooltip(*Chart, point,
                                   QString::fromStdString(tooltipString),
                                   GetCurrentForegroundBackground());
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

auto ChartView::ConnectScatterSeries(std::map<uint16_t, QScatterSeries*> const& indexScatterSeriesMap) noexcept
-> void {

    for (auto const& pair : indexScatterSeriesMap) {
        auto* scatterSeries = pair.second;

        connect(scatterSeries, &QScatterSeries::hovered, this, &ChartView::ToggleTooltip);

        connect(scatterSeries, &QScatterSeries::clicked, this,
                [this, scatterSeries, groupIdx = pair.first, indexScatterSeriesMap](QPointF const& point) {

                    auto points = scatterSeries->points();
                    auto it = std::find(points.cbegin(), points.cend(), point);
                    if (it == points.cend())
                        throw std::runtime_error("point not present in chart");

                    auto idx = static_cast<uint16_t>(std::distance(points.cbegin(), it));

                    bool const pointIsAlreadySelected = scatterSeries->isPointSelected(idx);

                    for (auto* series : std::views::values(indexScatterSeriesMap))
                        series->deselectAllPoints();

                    if (pointIsAlreadySelected)
                        Q_EMIT SamplePointChanged({});
                    else {
                        scatterSeries->selectPoint(idx);
                        Q_EMIT SamplePointChanged(SampleId { static_cast<uint16_t>(groupIdx), idx });
                    }
                });
    }
}


PcaChartView::PcaChartView() :
        ChartView("PCA") {}

auto PcaChartView::CreateScatterSeries() noexcept -> std::map<uint16_t, QScatterSeries*> {
    std::map<uint16_t, QScatterSeries*> indexScatterSeriesMap;

    for (int i = 0; i < BatchListData->Data.size(); i++) {
        auto const& batchData = BatchListData->Data.at(i);
        if (batchData.StateDataList.empty())
            continue;

        auto* scatterSeries = new QScatterSeries();
        scatterSeries->setName(QString::fromStdString(batchData.Group.GetName()));

        QList<QPointF> points;
        std::transform(batchData.StateDataList.cbegin(), batchData.StateDataList.cend(),
                       std::back_inserter(points),
                       [](auto const& psData) {
                           return QPointF(psData.PcaCoordinates.at(0), psData.PcaCoordinates.at(1));
                       });
        scatterSeries->append(points);

        indexScatterSeriesMap.emplace(i, scatterSeries);
    }

    return indexScatterSeriesMap;
}

auto PcaChartView::EditAxes(QValueAxis* xAxis, QValueAxis* yAxis) -> void {
    xAxis->setTitleText("PC1");
    yAxis->setTitleText("PC2");
}

TsneChartView::TsneChartView() :
        ChartView("t-SNE"),
        Tooltip(nullptr),
        LassoTool(nullptr) {}

auto TsneChartView::CreateScatterSeries() noexcept -> std::map<uint16_t, QScatterSeries*> {
    std::map<uint16_t, QScatterSeries*> indexScatterSeriesMap;

    for (int i = 0; i < BatchListData->Data.size(); i++) {
        auto const& batchData = BatchListData->Data.at(i);
        if (batchData.StateDataList.empty())
            continue;

        auto* scatterSeries = new QScatterSeries();
        scatterSeries->setName(QString::fromStdString(batchData.Group.GetName()));

        QList<QPointF> points;
        std::transform(batchData.StateDataList.cbegin(), batchData.StateDataList.cend(),
                       std::back_inserter(points),
                       [](auto const& psData) {
                           return QPointF(psData.TsneCoordinates.at(0), psData.TsneCoordinates.at(1));
                       });
        scatterSeries->append(points);

        indexScatterSeriesMap.emplace(i, scatterSeries);
    }

    return indexScatterSeriesMap;
}

void TsneChartView::AddItemsFromSceneOnUpdateData() {
    LassoTool = new ChartLassoSelectionTool(*Chart, GetCurrentForegroundBackground());

    connect(LassoTool, &ChartLassoSelectionTool::PointsSelected, this, [this](QList<QPointF> const& points) {
        if (points.size() > 2)
                Q_EMIT PointsSelected(points);
    });
}
