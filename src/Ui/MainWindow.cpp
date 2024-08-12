#include "MainWindow.h"

#include "Analysis/AnalysisWidget.h"
#include "Artifacts/ArtifactsWidget.h"
#include "Data/DataGenerationWidget.h"
#include "Modeling/ModelingWidget.h"
#include "PipelineGroups/PipelineGroupsWidget.h"
#include "Segmentation/SegmentationWidget.h"

#include <QApplication>
#include <QFileDialog>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QStyle>


MainWindow::MainWindow(CtStructureTree& ctStructureTree,
                       ThresholdFilter& thresholdFilter,
                       PipelineList& pipelineList,
                       PipelineGroupList& pipelineGroups,
                       Mode mode) {
    if (mode == Mode::NORMAL)
        resize(1400, 700);
    else {
        auto font = QApplication::font();
        font.setPointSize(font.pointSize() * 1.2);
        QApplication::setFont(font);
    }

    setWindowTitle("CT Uncertainty Propagation");

    auto* tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::TabPosition::West);
    QColor const windowColor = QApplication::palette().color(QPalette::ColorGroup::Active,
                                                           QPalette::ColorRole::Window).lighter();
    auto const styleSheetString = tabWidget->styleSheet();
    tabWidget->setStyleSheet(QString( "QTabBar::tab { padding: 15px 10px; }\n\n"
                                      "QTabBar::tab:selected {\n"
                                      "    background-color: rgb(%1, %2, %3);\n"
                                      "    border: 1px solid white;\n"
                                      "    border-radius: 2px;\n"
                                      "}")
                                      .arg(windowColor.red())
                                      .arg(windowColor.green())
                                      .arg(windowColor.blue()));

    auto* modelingWidget = new ModelingWidget(ctStructureTree);
    auto* artifactsWidget = new ArtifactsWidget(pipelineList);
    auto* segmentationWidget = new SegmentationWidget(thresholdFilter);
    auto* pipelineGroupsWidget = new PipelineGroupsWidget(pipelineGroups);
    auto* dataGenerationWidget = new DataGenerationWidget(pipelineGroups, thresholdFilter);
    auto* analysisWidget = new AnalysisWidget(pipelineGroups);

    tabWidget->addTab(modelingWidget, "Acquisition");
    tabWidget->addTab(artifactsWidget, "Artifacts");
    tabWidget->addTab(segmentationWidget, "Segmentation");
    tabWidget->addTab(pipelineGroupsWidget, "Pipeline Groups");
    tabWidget->addTab(dataGenerationWidget, "Data");
    tabWidget->addTab(analysisWidget, "Analysis");

    setCentralWidget(tabWidget);

    auto updateWidgets = [=](int idx) {
        if (idx == tabWidget->indexOf(segmentationWidget))
            segmentationWidget->UpdateDataSourceOnPipelineChange(artifactsWidget->GetCurrentPipeline());

        if (idx == tabWidget->indexOf(pipelineGroupsWidget))
            pipelineGroupsWidget->UpdatePipelineList();

        if (idx == tabWidget->indexOf(dataGenerationWidget))
            dataGenerationWidget->UpdateRowStatuses();

        if (idx == tabWidget->indexOf(analysisWidget))
            analysisWidget->UpdateData();
    };
    connect(tabWidget, &QTabWidget::currentChanged, this, updateWidgets);

    auto updateDataSources = [=]() {
        artifactsWidget->UpdateDataSource();
        segmentationWidget->UpdateDataSourceOnDataSourceChange();
        dataGenerationWidget->UpdateRowStatuses();
//        analysisWidget->UpdateDataSource();
    };
    connect(modelingWidget, &ModelingWidget::DataSourceUpdated, this, updateDataSources);

    Q_EMIT modelingWidget->DataSourceUpdated();
}


auto MainWindow::keyPressEvent(QKeyEvent* event) -> void {
    if (event->key() != Qt::Key::Key_F10) {
        QWidget::keyPressEvent(event);
        return;
    }

    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const fileFilter = "Images (*.png)";
    QString const caption = QString("Save screenshot");
    QString const fileName = QFileDialog::getSaveFileName(this, caption, homePath, fileFilter);
    grab().save(fileName);
}
