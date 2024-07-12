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
//        ImageTask(new DataGenerationTaskWidget(
//                *this,
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
//                    PipelineGroups.GenerateImages(callback); },
//                "Images",
//                "Generating Image Data... %p%",
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback){
//                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
//                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
//                        throw std::runtime_error("home path must not be empty");
//                    QString const& homePath = homeLocations.at(0);
//
//                    QString const caption = "Import Images (select all)";
//                    QString const fileFilter = "Images (*.vtk)";
//                    QStringList const fileNames = QFileDialog::getOpenFileNames(this, caption, homePath, fileFilter);
//                    std::vector<std::filesystem::path> filePaths { static_cast<size_t>(fileNames.size()) };
//                    std::transform(fileNames.cbegin(), fileNames.cend(), filePaths.begin(),
//                                   [](QString const& str) { return std::filesystem::path(str.toStdString()); });
//
//                    if (filePaths.empty())
//                        return;
//
//                    PipelineGroups.ImportImages(filePaths, callback);
//                },
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback){
//                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
//                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
//                        throw std::runtime_error("home path must not be empty");
//                    QString const& homePath = homeLocations.at(0);
//
//                    QString const caption = "Export Images";
//                    QString const dirName = QFileDialog::getExistingDirectory(this, caption, homePath);
//                    std::filesystem::path const exportDirPath = dirName.toStdString();
//
//                    if (exportDirPath.empty())
//                        return;
//
//                    PipelineGroups.ExportImagesHdf5(exportDirPath, callback);})
//                ),
//        FeatureTask(new DataGenerationTaskWidget(
//                *this,
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
//                    PipelineGroups.ExtractFeatures(callback); },
//                "Features",
//                "Extracting Features... %p%",
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
//                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
//                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
//                        throw std::runtime_error("home path must not be empty");
//                    QString const& homePath = homeLocations.at(0);
//
//                    QString const caption = "Import Features (select all)";
//                    QString const fileFilter = "Features (*.json)";
//                    QStringList const fileNames = QFileDialog::getOpenFileNames(this, caption, homePath, fileFilter);
//                    std::vector<std::filesystem::path> filePaths{ static_cast<size_t>(fileNames.size()) };
//                    std::transform(fileNames.cbegin(), fileNames.cend(), filePaths.begin(),
//                                   [](QString const& str) { return std::filesystem::path(str.toStdString()); });
//
//                    if (filePaths.empty())
//                        return;
//
//                    PipelineGroups.ImportFeatures(filePaths, callback);
//                },
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
//                    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
//                    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
//                        throw std::runtime_error("home path must not be empty");
//                    QString const& homePath = homeLocations.at(0);
//
//                    QString const caption = "Export Features";
//                    QString const dirName = QFileDialog::getExistingDirectory(this, caption, homePath);
//                    std::filesystem::path const exportDirPath = dirName.toStdString();
//
//                    if (exportDirPath.empty())
//                        return;
//
//                    PipelineGroups.ExportFeatures(exportDirPath, callback); })
//                ),
//        PcaTask(new DataGenerationTaskWidget(
//                *this,
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
//                    PipelineGroups.DoPCAs(2, callback); },
//                    "PCA",
//                    "Doing PCA... %p%")),
//        TsneTask(new DataGenerationTaskWidget(
//                *this,
//                [this](DataGenerationTaskWidget::ProgressCallback const& callback) {
//                    PipelineGroups.DoTsne(2, callback); },
//                "t-SNE",
//                "Doing t-SNE... %p%")),
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
        for (auto* row : TaskWidgets)
            row->SetButtonsEnabled(false);

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
        row->SetButtonsEnabled(false);
}

//DataGenerationTaskWidget::DataGenerationTaskWidget(DataGenerationWidget& parent,
//                                                   DataGenerator&& generator, QString const& name,
//                                                   QString const& progressBarFormat,
//                                                   DataGenerator&& importer, DataGenerator&& exporter) :
//        ParentWidget(parent),
//        Generator(std::move(generator)),
//        Name([&name]() {
//            auto* label = new QLabel(name);
//            label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
//            static int const minWidth = (new QLabel("Features"))->sizeHint().width() + 15;
//            label->setMinimumWidth(minWidth);
//            return label;
//        }()),
//        ProgressBar([&progressBarFormat]() {
//            auto* progressBar = new QProgressBar();
//            progressBar->setRange(0, 1000);
//            progressBar->setFormat(progressBarFormat);
//            return progressBar;
//        }()),
//        ImportButton([]() {
//            auto* button = new QPushButton("Import");
//            static int const minWidth = (new QLabel("Import"))->sizeHint().width() + 5;
//            button->setMinimumWidth(minWidth);
//            auto sizePolicy = button->sizePolicy();
//            sizePolicy.setRetainSizeWhenHidden(true);
//            button->setSizePolicy(sizePolicy);
//            return button;
//        }()),
//        Importer(importer ? std::move(importer) : nullptr),
//        ExportButton([]() {
//            auto* button = new QPushButton("Export");
//            static int const minWidth = (new QLabel("Import"))->sizeHint().width() + 5;
//            button->setMinimumWidth(minWidth);
//            auto sizePolicy = button->sizePolicy();
//            sizePolicy.setRetainSizeWhenHidden(true);
//            button->setSizePolicy(sizePolicy);
//            return button;
//        }()),
//        Exporter(exporter ? std::move(exporter) : nullptr),
//        GenerateButton(new QPushButton("Generate")),
//        StatusWidget(new DataGenerationStatusWidget()) {
//
//    auto* hLayout = new QHBoxLayout(this);
//    hLayout->addWidget(Name);
//    hLayout->addWidget(ProgressBar);
//    hLayout->addWidget(ImportButton);
//    hLayout->addWidget(ExportButton);
//    hLayout->addWidget(GenerateButton);
//    hLayout->addWidget(StatusWidget);
//
//    connect(GenerateButton, &QPushButton::clicked, this, [this]() { Generate(); });
//
//    if (Importer)
//        connect(ImportButton, &QPushButton::clicked, this, [this]() {
//            ParentWidget.DisableButtons();
//
//            QString const oldFormat = ProgressBar->format();
//            ProgressBar->setFormat("Importing... %p%");
//            Importer(ProgressCallback { *ProgressBar, Mutex });
//
//            ProgressBar->reset();
//            ProgressBar->setFormat(oldFormat);
//
//            ParentWidget.UpdateRowStatuses();
//        });
//    else
//        ImportButton->hide();
//
//    if (Exporter)
//        connect(ExportButton, &QPushButton::clicked, this, [this]() {
//            ParentWidget.DisableButtons();
//
//            QString const oldFormat = ProgressBar->format();
//            ProgressBar->setFormat("Exporting... %p%");
//            Exporter(ProgressCallback { *ProgressBar, Mutex });
//
//            ProgressBar->reset();
//            ProgressBar->setFormat(oldFormat);
//
//            ParentWidget.UpdateRowStatuses();
//        });
//    else
//        ExportButton->hide();
//}
//
//auto DataGenerationTaskWidget::ProgressCallback::operator()(double progress) -> void {
//    std::scoped_lock<std::mutex> const lock { Mutex };
//
//    ProgressBar.setValue(static_cast<int>(progress * 1000.0));
//}
//
//auto DataGenerationTaskWidget::Generate() -> void {
//    ParentWidget.DisableButtons();
//
//    Generator(ProgressCallback { *ProgressBar, Mutex });
//
//    ProgressBar->reset();
//
//    ParentWidget.UpdateRowStatuses();
//}
//
//auto DataGenerationTaskWidget::SetButtonEnabled(bool enabled) const -> void {
//    GenerateButton->setEnabled(enabled);
//}
//
//auto DataGenerationTaskWidget::UpdateStatus(Status status, Status previousStepStatus) const -> void {
//    bool const previousIsGenerated = previousStepStatus != Status::NOT_GENERATED;
//    GenerateButton->setEnabled(status != Status::UP_TO_DATE && previousIsGenerated);
//    GenerateButton->setText(status == Status::OUT_OF_DATE ? "Update" : "Generate");
//
//    ImportButton->setEnabled(status != Status::UP_TO_DATE && previousIsGenerated);
//    ExportButton->setEnabled(status != Status::NOT_GENERATED);
//
//    StatusWidget->UpdateStatus(status);
//}


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
            auto* label = new QLabel("");
            label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            static int const minWidth = (new QLabel("Features"))->sizeHint().width() + 15;
            label->setMinimumWidth(minWidth);
            return label;
        }()),
        ProgressBar([]() {
            auto* progressBar = new QProgressBar();
            progressBar->setRange(0, 1000);
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
        ExportButton([]() {
            auto* button = new QPushButton("Export");
            static int const minWidth = (new QLabel("Import"))->sizeHint().width() + 5;
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
}

auto DataGenerationTaskWidget::ProgressCallback::operator()(double progress) -> void {
    std::scoped_lock<std::mutex> const lock { Mutex };

    ProgressBar.setValue(static_cast<int>(progress * 1000.0));
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

auto DataGenerationTaskWidget::DoAfterTask() const -> void {
    ProgressBar->reset();

    ParentWidget.UpdateRowStatuses();
}

DataGenerationTaskWidget::~DataGenerationTaskWidget() {
    WorkerThread.quit();
    WorkerThread.wait();
}

GenerateImagesTaskWidget::GenerateImagesTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

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
    auto* imageImportWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.GenerateImages(ProgressCallback { *ProgressBar, Mutex });
    });

    imageImportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, imageImportWorker, &QObject::deleteLater);
    connect(imageImportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    imageImportWorker->DoWork();
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
    if (importPath.empty())
        return;

    auto* imageImportWorker = new VoidWorker([this, &importPath]() {
        ParentWidget.PipelineGroups.ImportImages(importPath,
                                                 ProgressCallback { *ProgressBar, Mutex });
    });

    imageImportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, imageImportWorker, &QObject::deleteLater);
    connect(imageImportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    imageImportWorker->DoWork();
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
            this, [this, selectImageFormatDialog, &homePath, &exportPath, &exportType]() {
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
            this, [this, selectImageFormatDialog, &homePath, &exportPath, &exportType]() {
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
    if (exportPath.empty())
        return;

    auto* imageExportWorker = new VoidWorker([this, &exportPath, exportType]() {
        switch (exportType) {
            case ExportType::HDF5: {
                ParentWidget.PipelineGroups.ExportImagesHdf5(exportPath, ProgressCallback{ *ProgressBar, Mutex });
                break;
            }
            case ExportType::VTK:
                ParentWidget.PipelineGroups.ExportImagesVtk(exportPath, ProgressCallback{ *ProgressBar, Mutex });
                break;
        }
    });

    imageExportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, imageExportWorker, &QObject::deleteLater);
    connect(imageExportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    imageExportWorker->DoWork();
}

ExtractFeaturesTaskWidget::ExtractFeaturesTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Generating image data... %p%");

        ExtractFeatures();
    });

    connect(ImportButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Extracting features... %p%");

        auto const importPath = PrepareImportFeatures();
        ImportFeatures(importPath);
    });

    connect(ExportButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Exporting images... %p%");

        auto const exportPath = PrepareExportFeatures();
        ExportFeatures(exportPath);
    });
}

auto ExtractFeaturesTaskWidget::ExtractFeatures() -> void {
    auto* imageImportWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.ExtractFeatures(ProgressCallback { *ProgressBar, Mutex });
    });

    imageImportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, imageImportWorker, &QObject::deleteLater);
    connect(imageImportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    imageImportWorker->DoWork();
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
    if (importPath.empty())
        return;

    auto* featureImportWorker = new VoidWorker([this, &importPath]() {
        ParentWidget.PipelineGroups.ImportFeatures(importPath,
                                                   ProgressCallback { *ProgressBar, Mutex });
    });

    featureImportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, featureImportWorker, &QObject::deleteLater);
    connect(featureImportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    featureImportWorker->DoWork();
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
    if (exportPath.empty())
        return;

    auto* featureExportWorker  = new VoidWorker([this, &exportPath]() {
        ParentWidget.PipelineGroups.ExportFeatures(exportPath, ProgressCallback{ *ProgressBar, Mutex });
    });

    featureExportWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, featureExportWorker, &QObject::deleteLater);
    connect(featureExportWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    featureExportWorker->DoWork();
}

DoPcaTaskWidget::DoPcaTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Doing PCA... %p%");

        DoPca();
    });

    ImportButton->hide();
    ExportButton->hide();
}



auto DoPcaTaskWidget::DoPca() -> void {
    auto* pcaWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.DoPCAs(2, ProgressCallback { *ProgressBar, Mutex });
    });

    pcaWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, pcaWorker, &QObject::deleteLater);
    connect(pcaWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    pcaWorker->DoWork();
}

DoTsneTaskWidget::DoTsneTaskWidget(DataGenerationWidget& parent) :
        DataGenerationTaskWidget(parent) {

    connect(GenerateButton, &QPushButton::clicked, this, [this]() {
        DoBeforeTask("Doing t-SNE... %p%");

        DoTsne();
    });

    ImportButton->hide();
    ExportButton->hide();
}



auto DoTsneTaskWidget::DoTsne() -> void {
    auto* pcaWorker = new VoidWorker([this]() {
        ParentWidget.PipelineGroups.DoTsne(2, ProgressCallback { *ProgressBar, Mutex });
    });

    pcaWorker->moveToThread(&WorkerThread);
    connect(&WorkerThread, &QThread::finished, pcaWorker, &QObject::deleteLater);
    connect(pcaWorker, &VoidWorker::Done, this, [this]() { DoAfterTask(); });

    WorkerThread.start();
    pcaWorker->DoWork();
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
