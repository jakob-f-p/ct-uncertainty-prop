#pragma once

#include <QWidget>
#include <QThread>

#include <filesystem>

class QLabel;
class QProgressBar;
class QPushButton;
class QVBoxLayout;

class DataGenerationStatusWidget;
class DataGenerationTaskWidget;
class PipelineGroupList;
class ThresholdFilter;


enum struct DataGenerationStatus : uint8_t {
    NOT_GENERATED,
    OUT_OF_DATE,
    UP_TO_DATE
};

class DataGenerationWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataGenerationWidget(PipelineGroupList& pipelineGroups,
                                  ThresholdFilter& thresholdFilter,
                                  QWidget* parent = nullptr);
    ~DataGenerationWidget() override;

public Q_SLOTS:
    auto
    UpdateRowStatuses() const -> void;

    auto
    DisableButtons() const -> void;

private:
    friend class GenerateImagesTaskWidget;
    friend class ExtractFeaturesTaskWidget;
    friend class DoPcaTaskWidget;
    friend class DoTsneTaskWidget;


    struct Statuses {
        using Status = DataGenerationStatus;

        explicit Statuses(DataGenerationWidget const& dataGenerationWidget);

        [[nodiscard]] auto
        AllUpToDate() const noexcept -> bool { return Image == Status::UP_TO_DATE
                                                      && Feature == Status::UP_TO_DATE
                                                      && Pca == Status::UP_TO_DATE
                                                      && Tsne == Status::UP_TO_DATE; }

        [[nodiscard]] auto
        AllGenerated() const noexcept -> bool { return Image != Status::NOT_GENERATED
                                                       && Feature != Status::NOT_GENERATED
                                                       && Pca != Status::NOT_GENERATED
                                                       && Tsne != Status::NOT_GENERATED; }

        Status Image   = Status::UP_TO_DATE;
        Status Feature = Status::UP_TO_DATE;
        Status Pca     = Status::UP_TO_DATE;
        Status Tsne    = Status::UP_TO_DATE;
    };


    PipelineGroupList& PipelineGroups;
    ThresholdFilter& ThresholdFilterAlgorithm;

    QPushButton* GenerateAllButton;
    DataGenerationStatusWidget* TotalStatusWidget;

    DataGenerationTaskWidget* ImageTask;
    DataGenerationTaskWidget* FeatureTask;
    DataGenerationTaskWidget* PcaTask;
    DataGenerationTaskWidget* TsneTask;
    std::vector<DataGenerationTaskWidget*> TaskWidgets;
    QThread WorkerThread;
    std::atomic_bool TaskIsRunning;
};


class DataGenerationTaskWidget : public QWidget {
    Q_OBJECT

public:
    struct ProgressCallback {
        auto
        operator()(double progress) const -> void;

        DataGenerationTaskWidget& Widget;
    };

    using Status = DataGenerationStatus;
    using DataGenerator = std::function<void(ProgressCallback)>;

    explicit DataGenerationTaskWidget(DataGenerationWidget& parent);
    ~DataGenerationTaskWidget() override;

    auto
    SetButtonsEnabled(bool enabled) const -> void;

    auto
    UpdateStatus(Status status, Status previousStepStatus) const -> void;

    virtual auto
    Generate() -> void = 0;

protected:
    auto
    DoBeforeTask(QString const& progressBarFormat) const -> void;

    auto
    DoAfterTask() -> void;

protected Q_SLOTS:
    auto
    UpdateProgress(double progress) const -> void;

Q_SIGNALS:
    auto
    ProgressUpdated(double progress) -> void;

    auto
    TaskDone() -> void;

protected:
    friend struct ProgressCallback;

    DataGenerationWidget& ParentWidget;
    QThread WorkerThread;

    QLabel* Name;
    QProgressBar* ProgressBar;
    QPushButton* ImportButton;
    QPushButton* ExportButton;
    QPushButton* GenerateButton;
    DataGenerationStatusWidget* StatusWidget;
};

class GenerateImagesTaskWidget : public DataGenerationTaskWidget {
    Q_OBJECT

public:
    explicit GenerateImagesTaskWidget(DataGenerationWidget& parent);

    auto
    Generate() -> void override { GenerateImages(); }

private:
    auto
    GenerateImages() -> void;

    [[nodiscard]] auto
    PrepareImportImages() -> std::filesystem::path;

    auto
    ImportImages(std::filesystem::path const& importPath) -> void;

    enum struct ExportType : uint8_t { HDF5, VTK };

    [[nodiscard]] auto
    PrepareExportImages() -> std::pair<std::filesystem::path, ExportType>;

    auto
    ExportImages(std::filesystem::path const& exportPath, ExportType exportType) -> void;

Q_SIGNALS:
    void StartWork();
};

class ExtractFeaturesTaskWidget : public DataGenerationTaskWidget {
public:
    explicit ExtractFeaturesTaskWidget(DataGenerationWidget& parent);

    auto
    Generate() -> void override { ExtractFeatures(); }

private:
    auto
    ExtractFeatures() -> void;

    [[nodiscard]] auto
    PrepareImportFeatures() -> std::filesystem::path;

    auto
    ImportFeatures(std::filesystem::path const& importPath) -> void;

    [[nodiscard]] auto
    PrepareExportFeatures() -> std::filesystem::path;

    auto
    ExportFeatures(std::filesystem::path const& exportPath) -> void;
};

class DoPcaTaskWidget : public DataGenerationTaskWidget {
public:
    explicit DoPcaTaskWidget(DataGenerationWidget& parent);

    auto
    Generate() -> void override { DoPca(); }

private:
    auto
    DoPca() -> void;
};

class DoTsneTaskWidget : public DataGenerationTaskWidget {
public:
    explicit DoTsneTaskWidget(DataGenerationWidget& parent);

    auto
    Generate() -> void override { DoTsne(); }

private:
    auto
    DoTsne() -> void;
};



class DataGenerationStatusWidget : public QWidget {
public:
    using Status = DataGenerationStatus;

    explicit DataGenerationStatusWidget();

    auto
    UpdateStatus(Status status) -> void;

private:
    QLabel* TextLabel;
};