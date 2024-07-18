#include "DataGenerationWidget.h"

#include "../Utils/WidgetUtils.h"
#include "../Utils/VoidWorker.h"
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
        ImageTask(new GenerateImagesTaskWidget(*this)),
        FeatureTask(new ExtractFeaturesTaskWidget(*this)),
        PcaTask(new DoPcaTaskWidget(*this)),
        TsneTask(new DoTsneTaskWidget(*this)),
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

        auto* generateAllWorker = new VoidWorker([this]() {
            Statuses const statuses { *this };

            std::atomic_bool taskIsRunning = false;

            for (auto* taskWidget : TaskWidgets)
                connect(taskWidget, &DataGenerationTaskWidget::TaskDone,
                        this, [&taskIsRunning]() {
                            taskIsRunning = false;
                            taskIsRunning.notify_all();
                        });

            if (statuses.Image != DataGenerationStatus::UP_TO_DATE) {
                taskIsRunning.wait(true);
                taskIsRunning = true;
                ImageTask->Generate();
            }
            if (statuses.Feature != DataGenerationStatus::UP_TO_DATE) {
                taskIsRunning.wait(true);
                taskIsRunning = true;
                FeatureTask->Generate();
            }
            if (statuses.Pca != DataGenerationStatus::UP_TO_DATE) {
                taskIsRunning.wait(true);
                taskIsRunning = true;
                PcaTask->Generate();
            }
            if (statuses.Tsne != DataGenerationStatus::UP_TO_DATE) {
                taskIsRunning.wait(true);
                taskIsRunning = true;
                TsneTask->Generate();
            }

            taskIsRunning.wait(true);
        });

        generateAllWorker->moveToThread(&WorkerThread);
        connect(&WorkerThread, &QThread::started, generateAllWorker, &VoidWorker::DoWork);
        connect(&WorkerThread, &QThread::finished, generateAllWorker, &QObject::deleteLater);
        connect(generateAllWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
        connect(generateAllWorker, &VoidWorker::Done, this, [this]() { UpdateRowStatuses(); });

        WorkerThread.start();
    });
}

DataGenerationWidget::~DataGenerationWidget() {
    WorkerThread.quit();
    WorkerThread.wait();
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
        row->SetButtonsEnabled(false);
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

DataGenerationTaskWidget::DataGenerationTaskWidget(DataGenerationWidget& parent) :
        ParentWidget(parent),
        Name([]() {
            auto* label = new QLabel("Features");
            static int const minWidth = label->sizeHint().width() + 15;
            label->setMinimumWidth(minWidth);
            label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            return label;
        }()),
        ProgressBar([]() {
            auto* progressBar = new QProgressBar();
            progressBar->setRange(0, 1000);
            return progressBar;
        }()),
        ImportButton([]() {
            auto* button = new QPushButton("Import");
            static int const minWidth = button->sizeHint().width() + 5;
            button->setMinimumWidth(minWidth);
            auto sizePolicy = button->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            button->setSizePolicy(sizePolicy);
            return button;
        }()),
        ExportButton([]() {
            auto* button = new QPushButton("Export");
            static int const minWidth = button->sizeHint().width() + 5;
            button->setMinimumWidth(minWidth);
            auto sizePolicy = button->sizePolicy();
            sizePolicy.setRetainSizeWhenHidden(true);
            button->setSizePolicy(sizePolicy);
            return button;
        }()),
        GenerateButton(new QPushButton("Generate")),
        StatusWidget(new DataGenerationStatusWidget()) {

    auto* hLayout = new QHBoxLayout(this);
    hLayout->addWidget(Name);
    hLayout->addWidget(ProgressBar);
    hLayout->addWidget(ImportButton);
    hLayout->addWidget(ExportButton);
    hLayout->addWidget(GenerateButton);
    hLayout->addWidget(StatusWidget);

    connect(this, &DataGenerationTaskWidget::ProgressUpdated, this, &DataGenerationTaskWidget::UpdateProgress);
}

auto DataGenerationTaskWidget::ProgressCallback::operator()(double progress) -> void {
    Q_EMIT Widget.ProgressUpdated(progress);
}

auto DataGenerationTaskWidget::SetButtonsEnabled(bool enabled) const -> void {
    GenerateButton->setEnabled(enabled);
    ImportButton->setEnabled(enabled);
    ExportButton->setEnabled(enabled);
}

auto DataGenerationTaskWidget::UpdateStatus(DataGenerationTaskWidget::Status status,
                                            DataGenerationTaskWidget::Status previousStepStatus) const -> void {
    bool const previousIsGenerated = previousStepStatus != Status::NOT_GENERATED;
    GenerateButton->setEnabled(status != Status::UP_TO_DATE && previousIsGenerated);
    GenerateButton->setText(status == Status::OUT_OF_DATE ? "Update" : "Generate");

    ImportButton->setEnabled(status != Status::UP_TO_DATE && previousIsGenerated);
    ExportButton->setEnabled(status != Status::NOT_GENERATED);

    StatusWidget->UpdateStatus(status);
}

auto DataGenerationTaskWidget::DoBeforeTask(QString const& progressBarFormat) const -> void {
    ParentWidget.DisableButtons();

    QString const oldFormat = ProgressBar->format();
    ProgressBar->setFormat(progressBarFormat);
}

auto DataGenerationTaskWidget::DoAfterTask() -> void {
    Q_EMIT TaskDone();

    ProgressBar->reset();

    ParentWidget.UpdateRowStatuses();
}

auto DataGenerationTaskWidget::UpdateProgress(double progress) -> void {
    ProgressBar->setValue(static_cast<int>(progress * 1000.0));
}

DataGenerationTaskWidget::~DataGenerationTaskWidget() {
    WorkerThread.quit();
    WorkerThread.wait();
}

GenerateImagesTaskWidget::GenerateImagesTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    Name->setText("Images");

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Generating Image Data... %p%");

        GenerateImages();
    });

    connect(ImportButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Importing images... %p%");

        auto const importPath = PrepareImportImages();
        ImportImages(importPath);
    });

    connect(ExportButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Exporting images... %p%");

        auto const [ exportPath, exportType ] = PrepareExportImages();
        ExportImages(exportPath, exportType);
    });
}


auto GenerateImagesTaskWidget::GenerateImages() -> void {
    auto* imageGenerateWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.GenerateImages(ProgressCallback { *this });
    });

    imageGenerateWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, imageGenerateWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, imageGenerateWorker, &QObject::deleteLater);
    connect(imageGenerateWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(imageGenerateWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

auto GenerateImagesTaskWidget::PrepareImportImages() -> std::filesystem::path {
    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const caption = "Import Images";
    QString const fileFilter = "Images (*.h5)";
    QString const fileName = QFileDialog::getOpenFileName(this, caption, homePath, fileFilter);
    std::filesystem::path const filePath = std::filesystem::path(fileName.toStdString());

    if (!filePath.empty() && !is_regular_file(filePath))
        throw std::runtime_error("given file path is not a regular file");

    return filePath;
}

auto GenerateImagesTaskWidget::ImportImages(std::filesystem::path const& importPath) -> void {
    if (importPath.empty()) {
        DoAfterTask();
        return;
    }

    auto* imageImportWorker = new VoidWorker([this, importPath]() {
        ParentWidget.PipelineGroups.ImportImages(importPath,
                                                 ProgressCallback { *this });
    });

    imageImportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, imageImportWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, imageImportWorker, &QObject::deleteLater);
    connect(imageImportWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(imageImportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

auto GenerateImagesTaskWidget::PrepareExportImages() -> std::pair<std::filesystem::path, ExportType> {
    auto* selectImageFormatDialog = new QDialog();
    auto* dialogHLayout = new QHBoxLayout(selectImageFormatDialog);
    auto* dialogHdf5Button = new QPushButton("Hdf5 (.h5) (single file)");
    auto* dialogVtkButton = new QPushButton("VTK (.vtk) (multiple file)");
    dialogHLayout->addWidget(dialogHdf5Button);
    dialogHLayout->addWidget(dialogVtkButton);

    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    std::filesystem::path exportPath;
    ExportType exportType = ExportType::HDF5;

    connect(dialogHdf5Button, &QPushButton::clicked,
            this, [this, selectImageFormatDialog, homePath, &exportPath, &exportType]() {
        QString const caption = "Export Images (.h5)";
        QString const fileFilter = "Images (*.h5)";
        QString const filePath = QFileDialog::getSaveFileName(this, caption, homePath, fileFilter);
        std::filesystem::path const exportFilePath = filePath.toStdString();

        if (!exportFilePath.empty() && exists(exportFilePath) && !is_regular_file(exportFilePath))
            throw std::runtime_error("invalid path");

        exportPath = exportFilePath;
        exportType = ExportType::HDF5;
        selectImageFormatDialog->accept();
    });

    connect(dialogVtkButton, &QPushButton::clicked,
            this, [this, selectImageFormatDialog, homePath, &exportPath, &exportType]() {
        QString const caption = "Export Images (.vtk)";
        QString const dirName = QFileDialog::getExistingDirectory(this, caption, homePath);
        std::filesystem::path const exportDirPath = dirName.toStdString();

        if (!exportDirPath.empty() && !is_directory(exportDirPath))
            throw std::runtime_error("given path is not a directory");

        exportPath = exportDirPath;
        exportType = ExportType::VTK;
        selectImageFormatDialog->accept();
    });

    auto const accepted = selectImageFormatDialog->exec();
    if (accepted == 0)
        return { "", ExportType::HDF5 };

    return { exportPath, exportType };
}

auto GenerateImagesTaskWidget::ExportImages(std::filesystem::path const& exportPath, ExportType exportType) -> void {
    if (exportPath.empty()) {
        DoAfterTask();
        return;
    }

    auto* imageExportWorker = new VoidWorker([this, exportPath, exportType]() {
        switch (exportType) {
            case ExportType::HDF5: {
                ParentWidget.PipelineGroups.ExportImagesHdf5(exportPath, ProgressCallback{ *this });
                break;
            }
            case ExportType::VTK:
                ParentWidget.PipelineGroups.ExportImagesVtk(exportPath, ProgressCallback{ *this });
                break;
        }
    });

    imageExportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, imageExportWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, imageExportWorker, &QObject::deleteLater);
    connect(imageExportWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(imageExportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

ExtractFeaturesTaskWidget::ExtractFeaturesTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    Name->setText("Features");

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Extracting features... %p%");

        ExtractFeatures();
    });

    connect(ImportButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Importing features... %p%");

        auto const importPath = PrepareImportFeatures();
        ImportFeatures(importPath);
    });

    connect(ExportButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Exporting features... %p%");

        auto const exportPath = PrepareExportFeatures();
        ExportFeatures(exportPath);
    });
}

auto ExtractFeaturesTaskWidget::ExtractFeatures() -> void {
    auto* extractFeaturesWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.ExtractFeatures(ProgressCallback { *this });
    });

    extractFeaturesWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, extractFeaturesWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, extractFeaturesWorker, &QObject::deleteLater);
    connect(extractFeaturesWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(extractFeaturesWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

auto ExtractFeaturesTaskWidget::PrepareImportFeatures() -> std::filesystem::path {
    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const caption = "Import Features";
    QString const fileFilter = "Features (*.json)";
    QString const fileName = QFileDialog::getOpenFileName(this, caption, homePath, fileFilter);

    std::filesystem::path const filePath = std::filesystem::path(fileName.toStdString());

    if (!filePath.empty() && !is_regular_file(filePath))
        throw std::runtime_error("given file path is not a regular file");

    return filePath;
}

auto ExtractFeaturesTaskWidget::ImportFeatures(std::filesystem::path const& importPath) -> void {
    if (importPath.empty()) {
        DoAfterTask();
        return;
    }

    auto* featureImportWorker = new VoidWorker([this, importPath]() {
        ParentWidget.PipelineGroups.ImportFeatures(importPath, ProgressCallback { *this });
    });

    featureImportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, featureImportWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, featureImportWorker, &QObject::deleteLater);
    connect(featureImportWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(featureImportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

auto ExtractFeaturesTaskWidget::PrepareExportFeatures() -> std::filesystem::path {
    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const caption = "Export Features";
    QString const fileFilter = "Features (*.json)";
    QString const fileName = QFileDialog::getSaveFileName(this, caption, homePath, fileFilter);
    std::filesystem::path const filePath = fileName.toStdString();

    return filePath;
}

auto ExtractFeaturesTaskWidget::ExportFeatures(std::filesystem::path const& exportPath) -> void {
    if (exportPath.empty()) {
        DoAfterTask();
        return;
    }

    auto* featureExportWorker  = new VoidWorker([this, exportPath]() {
        ParentWidget.PipelineGroups.ExportFeatures(exportPath, ProgressCallback{ *this });
    });

    featureExportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, featureExportWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, featureExportWorker, &QObject::deleteLater);
    connect(featureExportWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(featureExportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

DoPcaTaskWidget::DoPcaTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    Name->setText("PCAs");

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Doing PCAs... %p%");

        DoPca();
    });

    ImportButton->hide();
    ExportButton->hide();
}



auto DoPcaTaskWidget::DoPca() -> void {
    auto* pcaWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.DoPCAs(2, ProgressCallback { *this });
    });

    pcaWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, pcaWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, pcaWorker, &QObject::deleteLater);
    connect(pcaWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(pcaWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

DoTsneTaskWidget::DoTsneTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    Name->setText("t-SNE");

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Doing t-SNE... %p%");

        DoTsne();
    });

    ImportButton->hide();
    ExportButton->hide();
}



auto DoTsneTaskWidget::DoTsne() -> void {
    auto* tsneWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.DoTsne(2, ProgressCallback { *this });
    });

    tsneWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::started, tsneWorker, &VoidWorker::DoWork);
    connect(&WorkerThread, &QThread::finished, tsneWorker, &QObject::deleteLater);
    connect(tsneWorker, &VoidWorker::Done, &WorkerThread, &QThread::quit);
    connect(tsneWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
}

DataGenerationStatusWidget::DataGenerationStatusWidget() :
        TextLabel([]() {
            auto* label = new QLabel("Not generated");
            static int const minWidth = label->sizeHint().width() + 10;
            label->setMinimumWidth(minWidth);
            label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
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
