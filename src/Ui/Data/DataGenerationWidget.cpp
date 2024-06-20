#include "DataGenerationWidget.h"

#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroupList.h"
#include "../../Segmentation/ThresholdFilter.h"

#include <QFileDialog>
#include <QLabel>
#include <QStandardPaths>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

DataGenerationWidget::DataGenerationWidget(PipelineGroupList& pipelineGroups,
                                           ThresholdFilter& thresholdFilter,
                                           QWidget* parent) :
        QWidget(parent),
        PipelineGroups(pipelineGroups),
        ThresholdFilterAlgorithm(thresholdFilter),
        GenerateAllButton(new QPushButton("Generate all")),
        TotalStatusWidget(new DataGenerationStatusWidget()),
        ImageTask(new DataGenerationTaskWidget(
                *this,
                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
                    PipelineGroups.GenerateImages(callback); },
                "Images",
                "Generating Image Data... %p%",
                [this](DataGenerationTaskWidget::ProgressCallback const& callback){
                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
                        throw std::runtime_error("home path must not be empty");
                    QString const& homePath = homeLocations.at(0);

                    QString const caption = "Import Images (select all)";
                    QString const fileFilter = "Images (*.vtk)";
                    QStringList const fileNames = QFileDialog::getOpenFileNames(this, caption, homePath, fileFilter);
                    std::vector<std::filesystem::path> filePaths { static_cast<size_t>(fileNames.size()) };
                    std::transform(fileNames.cbegin(), fileNames.cend(), filePaths.begin(),
                                   [](QString const& str) { return std::filesystem::path(str.toStdString()); });

                    if (filePaths.empty())
                        return;

                    PipelineGroups.ImportImages(filePaths, callback);
                },
                [this](DataGenerationTaskWidget::ProgressCallback const& callback){
                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
                        throw std::runtime_error("home path must not be empty");
                    QString const& homePath = homeLocations.at(0);

                    QString const caption = "Export Images";
                    QString const dirName = QFileDialog::getExistingDirectory(this, caption, homePath);
                    std::filesystem::path const exportDirPath = dirName.toStdString();

                    if (exportDirPath.empty())
                        return;

                    PipelineGroups.ExportGeneratedImages(exportDirPath, callback);})
                ),
        FeatureTask(new DataGenerationTaskWidget(
                *this,
                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
                    PipelineGroups.ExtractFeatures(callback); },
                "Features",
                "Extracting Features... %p%",
                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
                        throw std::runtime_error("home path must not be empty");
                    QString const& homePath = homeLocations.at(0);

                    QString const caption = "Import Features (select all)";
                    QString const fileFilter = "Features (*.json)";
                    QStringList const fileNames = QFileDialog::getOpenFileNames(this, caption, homePath, fileFilter);
                    std::vector<std::filesystem::path> filePaths{ static_cast<size_t>(fileNames.size()) };
                    std::transform(fileNames.cbegin(), fileNames.cend(), filePaths.begin(),
                                   [](QString const& str) { return std::filesystem::path(str.toStdString()); });

                    if (filePaths.empty())
                        return;

                    PipelineGroups.ImportFeatures(filePaths, callback);
                },
                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
                        throw std::runtime_error("home path must not be empty");
                    QString const& homePath = homeLocations.at(0);

                    QString const caption = "Export Features";
                    QString const dirName = QFileDialog::getExistingDirectory(this, caption, homePath);
                    std::filesystem::path const exportDirPath = dirName.toStdString();

                    if (exportDirPath.empty())
                        return;

                    PipelineGroups.ExportFeatures(exportDirPath, callback); })
                ),
        PcaTask(new DataGenerationTaskWidget(
                *this,
                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
                    PipelineGroups.DoPCAs(2, callback); },
                    "PCA",
                    "Doing PCA... %p%")),
        TsneTask(new DataGenerationTaskWidget(
                *this,
                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
                    PipelineGroups.DoTsne(2, callback); },
                "t-SNE",
                "Doing t-SNE... %p%")),
        TaskWidgets({ ImageTask, FeatureTask, PcaTask, TsneTask }) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(10);

    auto* titleBarWidget = new QWidget();
    auto* titleBarHLayout = new QHBoxLayout(titleBarWidget);
    auto* titleLabel = new QLabel("Generate Data");
    titleLabel->setStyleSheet(GetHeader1StyleSheet());
    titleBarHLayout->addWidget(titleLabel);
    titleBarHLayout->addStretch();
    titleBarHLayout->addWidget(GenerateAllButton);
    titleBarHLayout->addWidget(TotalStatusWidget);
    vLayout->addWidget(titleBarWidget);

    for (auto* taskWidget : TaskWidgets)
        vLayout->addWidget(taskWidget);

    vLayout->addStretch();

    connect(GenerateAllButton, &QPushButton::clicked, this, [this]() {
        GenerateAllButton->setEnabled(false);
        for (auto* row : TaskWidgets)
            row->SetButtonEnabled(false);

        GeneratingAll = true;

        Statuses const statuses { *this };
        if (statuses.Image != DataGenerationStatus::UP_TO_DATE) {
            ImageTask->Generate();
            DisableButtons();
        }
        if (statuses.Feature != DataGenerationStatus::UP_TO_DATE) {
            FeatureTask->Generate();
            DisableButtons();
        }
        if (statuses.Pca != DataGenerationStatus::UP_TO_DATE) {
            PcaTask->Generate();
            DisableButtons();
        }
        if (statuses.Tsne != DataGenerationStatus::UP_TO_DATE) {
            TsneTask->Generate();
            DisableButtons();
        }

        GeneratingAll = false;

        UpdateRowStatuses();
    });
}

auto DataGenerationWidget::UpdateRowStatuses() -> void {
    Statuses const statuses { *this };

    GenerateAllButton->setEnabled(!statuses.AllUpToDate());
    TotalStatusWidget->UpdateStatus(statuses.AllUpToDate()
            ? DataGenerationStatus::UP_TO_DATE
            : (statuses.AllGenerated()
                    ? DataGenerationStatus::OUT_OF_DATE
                    : DataGenerationStatus::NOT_GENERATED));

    ImageTask->UpdateStatus(statuses.Image, DataGenerationStatus::UP_TO_DATE);
    FeatureTask->UpdateStatus(statuses.Feature, statuses.Image);
    PcaTask->UpdateStatus(statuses.Pca, statuses.Feature);
    TsneTask->UpdateStatus(statuses.Tsne, statuses.Feature);
}

auto DataGenerationWidget::DisableButtons() const -> void {
    GenerateAllButton->setEnabled(false);
    for (auto* row : TaskWidgets)
        row->SetButtonEnabled(false);
}

DataGenerationTaskWidget::DataGenerationTaskWidget(DataGenerationWidget& parent,
                                                   DataGenerator&& generator, QString const& name,
                                                   QString const& progressBarFormat,
                                                   DataGenerator&& importer, DataGenerator&& exporter) :
        ParentWidget(parent),
        Generator(std::move(generator)),
        Name([&name]() {
            auto* label = new QLabel(name);
            label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            static int const minWidth = (new QLabel("Features"))->sizeHint().width() + 15;
            label->setMinimumWidth(minWidth);
            return label;
        }()),
        ProgressBar([&progressBarFormat]() {
            auto* progressBar = new QProgressBar();
            progressBar->setRange(0, 1000);
            progressBar->setFormat(progressBarFormat);
            return progressBar;
        }()),
        ImportButton([]() {
            auto* button = new QPushButton("Import");
            static int const minWidth = (new QLabel("Import"))->sizeHint().width() + 5;
            button->setMinimumWidth(minWidth);
            auto sizePolicy = button->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            button->setSizePolicy(sizePolicy);
            return button;
        }()),
        Importer(importer ? std::move(importer) : nullptr),
        ExportButton([]() {
            auto* button = new QPushButton("Export");
            static int const minWidth = (new QLabel("Import"))->sizeHint().width() + 5;
            button->setMinimumWidth(minWidth);
            auto sizePolicy = button->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            button->setSizePolicy(sizePolicy);
            return button;
        }()),
        Exporter(exporter ? std::move(exporter) : nullptr),
        GenerateButton(new QPushButton("Generate")),
        StatusWidget(new DataGenerationStatusWidget()) {

    auto* hLayout = new QHBoxLayout(this);
    hLayout->addWidget(Name);
    hLayout->addWidget(ProgressBar);
    hLayout->addWidget(ImportButton);
    hLayout->addWidget(ExportButton);
    hLayout->addWidget(GenerateButton);
    hLayout->addWidget(StatusWidget);

    connect(GenerateButton, &QPushButton::clicked, this, [this]() { Generate(); });

    if (Importer)
        connect(ImportButton, &QPushButton::clicked, this, [this]() {
            ParentWidget.DisableButtons();

            QString const oldFormat = ProgressBar->format();
            ProgressBar->setFormat("Importing... %p%");
            Importer(ProgressCallback { *ProgressBar, Mutex });

            ProgressBar->reset();
            ProgressBar->setFormat(oldFormat);

            ParentWidget.UpdateRowStatuses();
        });
    else
        ImportButton->hide();

    if (Exporter)
        connect(ExportButton, &QPushButton::clicked, this, [this]() {
            ParentWidget.DisableButtons();

            QString const oldFormat = ProgressBar->format();
            ProgressBar->setFormat("Exporting... %p%");
            Exporter(ProgressCallback { *ProgressBar, Mutex });

            ProgressBar->reset();
            ProgressBar->setFormat(oldFormat);

            ParentWidget.UpdateRowStatuses();
        });
    else
        ExportButton->hide();
}

auto DataGenerationTaskWidget::ProgressCallback::operator()(double progress) -> void {
    std::scoped_lock<std::mutex> const lock { Mutex };

    ProgressBar.setValue(static_cast<int>(progress * 1000.0));
}

auto DataGenerationTaskWidget::Generate() -> void {
    ParentWidget.DisableButtons();

    Generator(ProgressCallback { *ProgressBar, Mutex });

    ProgressBar->reset();

    ParentWidget.UpdateRowStatuses();
}

auto DataGenerationTaskWidget::SetButtonEnabled(bool enabled) const -> void {
    GenerateButton->setEnabled(enabled);
}

auto DataGenerationTaskWidget::UpdateStatus(Status status, Status previousStepStatus) const -> void {
    bool const previousIsGenerated = previousStepStatus != Status::NOT_GENERATED;
    GenerateButton->setEnabled(status != Status::UP_TO_DATE && previousIsGenerated);
    GenerateButton->setText(status == Status::OUT_OF_DATE ? "Update" : "Generate");

    ImportButton->setEnabled(status != Status::UP_TO_DATE && previousIsGenerated);
    ExportButton->setEnabled(status != Status::NOT_GENERATED);

    StatusWidget->UpdateStatus(status);
}


DataGenerationStatusWidget::DataGenerationStatusWidget() :
        TextLabel([]() {
            auto* label = new QLabel("");
            label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
            static int const minWidth = (new QLabel("Not generated"))->sizeHint().width() + 10;
            label->setMinimumWidth(minWidth);
            return label;
        }()) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});
    vLayout->addWidget(TextLabel);

    UpdateStatus(Status::NOT_GENERATED);
}

auto DataGenerationStatusWidget::UpdateStatus(Status status) -> void {
    switch (status) {
        case Status::NOT_GENERATED: {
            TextLabel->setText("Not generated");
            setStyleSheet("border: 1px solid red");
            break;
        }

        case Status::OUT_OF_DATE: {
            TextLabel->setText("Out of date");
            setStyleSheet("border: 1px solid orange");
            break;
        }

        case Status::UP_TO_DATE: {
            TextLabel->setText("Up to date");
            setStyleSheet("border: 1px solid white");
            break;
        }

        default: throw std::runtime_error("invalid status");
    }
}

DataGenerationWidget::Statuses::Statuses(DataGenerationWidget& dataGenerationWidget) {
    auto dataStatus = dataGenerationWidget.PipelineGroups.GetDataStatus();
    vtkMTimeType const pipelineGroupsTime = dataGenerationWidget.PipelineGroups.GetMTime();
    vtkMTimeType const thresholdFilterMTime = dataGenerationWidget.ThresholdFilterAlgorithm.GetMTime();

    auto compareAndUpdate = [this, dataStatus](vtkMTimeType time) noexcept -> void {
        if (dataStatus.Image   < time) {
            Image   = dataStatus.Image   == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
            Feature = dataStatus.Feature == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
            Pca     = dataStatus.Pca     == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
            Tsne    = dataStatus.Tsne    == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
        } else if (dataStatus.Feature < time) {
            Feature = dataStatus.Feature == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
            Pca     = dataStatus.Pca     == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
            Tsne    = dataStatus.Tsne    == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
        } else {
            if (dataStatus.Pca     < time)
                Pca     = dataStatus.Pca     == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;

            if (dataStatus.Tsne    < time)
                Tsne    = dataStatus.Tsne    == 0 ? Status::NOT_GENERATED : Status::OUT_OF_DATE;
        }
    };
    compareAndUpdate(pipelineGroupsTime);
    compareAndUpdate(thresholdFilterMTime);
}
