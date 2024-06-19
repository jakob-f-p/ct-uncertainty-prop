#pragma once

#include <QWidget>

#include <mutex>

class QGridLayout;
class QLabel;
class QProgressBar;
class QPushButton;

class DataGenerationStatusWidget;
class PipelineGroupList;
class ThresholdFilter;


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

public:
    struct ProgressCallback {
        auto
        operator()(double progress) -> void;

        QProgressBar& ProgressBar;
        std::mutex& Mutex;
    };

    struct DataGenerationWidgetRow : public QObject {
        using DataGenerator = std::function<void(ProgressCallback)>;

        explicit DataGenerationWidgetRow(DataGenerationWidget& parent,
                                         DataGenerator&& generator,
                                         QString const& name,
                                         QString const& progressBarFormat);

        auto
        AddToLayout(QGridLayout& gLayout) const noexcept -> void;

        auto
        Generate() -> void;

        auto
        SetButtonEnabled(bool enabled) const -> void;

        enum struct Status : uint8_t {
            NOT_GENERATED,
            OUT_OF_DATE,
            UP_TO_DATE
        };

        auto
        UpdateStatus(Status status) const -> void;


        DataGenerationWidget& ParentWidget;
        DataGenerator Generator;

        QLabel* Name;
        QProgressBar* ProgressBar;
        std::mutex Mutex;
        DataGenerationStatusWidget* StatusWidget;
        QPushButton* GenerateButton;
    };

private:
    struct Statuses {
        using Status = DataGenerationWidgetRow::Status;

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

    QGridLayout* GLayout;
    QPushButton* GenerateAllButton;
    bool GeneratingAll = false;
    DataGenerationStatusWidget* TotalStatusWidget;

    DataGenerationWidgetRow* ImageRow;
    DataGenerationWidgetRow* FeaturesRow;
    DataGenerationWidgetRow* PcaRow;
    DataGenerationWidgetRow* TsneRow;
    std::vector<DataGenerationWidgetRow*> Rows;
};


class DataGenerationStatusWidget : public QWidget {
public:
    using Status = DataGenerationWidget::DataGenerationWidgetRow::Status;

    explicit DataGenerationStatusWidget();

    auto
    UpdateStatus(Status status) -> void;

private:
    QLabel* TextLabel;
};