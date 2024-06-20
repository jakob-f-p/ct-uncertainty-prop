#pragma once

#include <QWidget>

#include <mutex>

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

public Q_SLOTS:
    auto
    UpdateRowStatuses() -> void;

    auto
    DisableButtons() const -> void;

private:
    struct Statuses {
        using Status = DataGenerationStatus;

        Statuses(DataGenerationWidget& dataGenerationWidget);

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
    bool GeneratingAll = false;
    DataGenerationStatusWidget* TotalStatusWidget;

    DataGenerationTaskWidget* ImageTask;
    DataGenerationTaskWidget* FeatureTask;
    DataGenerationTaskWidget* PcaTask;
    DataGenerationTaskWidget* TsneTask;
    std::vector<DataGenerationTaskWidget*> TaskWidgets;
};


struct DataGenerationTaskWidget : public QWidget {
    struct ProgressCallback {
        auto
        operator()(double progress) -> void;

       QProgressBar& ProgressBar;
        std::mutex& Mutex;
    };

    using Status = DataGenerationStatus;
    using DataGenerator = std::function<void(ProgressCallback)>;

    explicit DataGenerationTaskWidget(DataGenerationWidget& parent,
                                      DataGenerator&& generator,
                                      QString const& name,
                                      QString const& progressBarFormat,
                                      DataGenerator&& Importer = {}, DataGenerator&& Exporter = {});

    auto
    Generate() -> void;

    auto
    SetButtonEnabled(bool enabled) const -> void;

    auto
    UpdateStatus(Status status, Status previousStepStatus) const -> void;


    DataGenerationWidget& ParentWidget;
    DataGenerator Generator;
    std::mutex Mutex;

    QLabel* Name;
    QProgressBar* ProgressBar;
    QPushButton* ImportButton;
    DataGenerator Importer;
    QPushButton* ExportButton;
    DataGenerator Exporter;
    QPushButton* GenerateButton;
    DataGenerationStatusWidget* StatusWidget;
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