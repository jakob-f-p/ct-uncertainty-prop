#include "DataGenerationWidget.h"

#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroupList.h"
#include "../../Segmentation/ThresholdFilter.h"

#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

DataGenerationWidget::DataGenerationWidget(PipelineGroupList& pipelineGroups,
                                           ThresholdFilter& thresholdFilter,
                                           QWidget* parent) :
        QWidget(parent),
        PipelineGroups(pipelineGroups),
        ThresholdFilterAlgorithm(thresholdFilter),
        GLayout(new QGridLayout(this)),
        GenerateAllButton(new QPushButton("Generate all")),
        TotalStatusWidget(new DataGenerationStatusWidget()),
        ImageRow(new DataGenerationWidgetRow(
                *this,
                [this](ProgressCallback const& callback) { PipelineGroups.GenerateImages(callback); },
                "Images",
                "Generating Image Data... %p%")),
        FeaturesRow(new DataGenerationWidgetRow(
                *this,
                [this](ProgressCallback const& callback) { PipelineGroups.ExtractFeatures(callback); },
                    "Features",
                    "Extracting Features... %p%")),
        PcaRow(new DataGenerationWidgetRow(
                *this,
                [this](ProgressCallback const& callback) { PipelineGroups.DoPCAs(2, callback); },
                    "PCA",
                    "Doing PCA... %p%")),
        TsneRow(new DataGenerationWidgetRow(
                *this,
                [this](ProgressCallback const& callback) { PipelineGroups.DoTsne(2, callback); },
                "t-SNE",
                "Doing t-SNE... %p%")),
        Rows({ ImageRow, FeaturesRow, PcaRow, TsneRow }) {

    GLayout->setHorizontalSpacing(10);
    GLayout->setVerticalSpacing(10);

    auto* titleLabel = new QLabel("Generate Data");
    titleLabel->setStyleSheet(GetHeader1StyleSheet());
    GLayout->addWidget(titleLabel, 0, 0, 1, 2);
    GLayout->addWidget(GenerateAllButton, 0, 2);
    GLayout->addWidget(TotalStatusWidget, 0, 3);

    for (auto const* row : Rows)
        row->AddToLayout(*GLayout);

    GLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding),
                     GLayout->rowCount() + 1, 0, 1, GLayout->columnCount());

    connect(GenerateAllButton, &QPushButton::clicked, this, [this]() {
        GenerateAllButton->setEnabled(false);
        for (auto* row : Rows)
            row->SetButtonEnabled(false);

        GeneratingAll = true;

        Statuses const statuses { *this };
        if (statuses.Image != DataGenerationWidgetRow::Status::UP_TO_DATE) {
            ImageRow->Generate();
            DisableButtons();
        }
        if (statuses.Feature != DataGenerationWidgetRow::Status::UP_TO_DATE) {
            FeaturesRow->Generate();
            DisableButtons();
        }
        if (statuses.Pca != DataGenerationWidgetRow::Status::UP_TO_DATE) {
            PcaRow->Generate();
            DisableButtons();
        }
        if (statuses.Tsne != DataGenerationWidgetRow::Status::UP_TO_DATE) {
            TsneRow->Generate();
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
            ? DataGenerationWidgetRow::Status::UP_TO_DATE
            : (statuses.AllGenerated()
                    ? DataGenerationWidgetRow::Status::OUT_OF_DATE
                    : DataGenerationWidgetRow::Status::NOT_GENERATED));

    ImageRow->UpdateStatus(statuses.Image);
    FeaturesRow->UpdateStatus(statuses.Feature);
    PcaRow->UpdateStatus(statuses.Pca);
    TsneRow->UpdateStatus(statuses.Tsne);
}

auto DataGenerationWidget::DisableButtons() const -> void {
    GenerateAllButton->setEnabled(false);
    for (auto* row : Rows)
        row->SetButtonEnabled(false);
}

DataGenerationWidget::DataGenerationWidgetRow::DataGenerationWidgetRow(DataGenerationWidget& parent,
                                                                       DataGenerator&& generator,
                                                                       QString const& name,
                                                                       QString const& progressBarFormat) :
        ParentWidget(parent),
        Generator(std::move(generator)),
        Name(new QLabel(name)),
        ProgressBar([&progressBarFormat]() {
            auto* progressBar = new QProgressBar();
            progressBar->setRange(0, 1000);
            progressBar->setFormat(progressBarFormat);
            return progressBar;
        }()),
        StatusWidget(new DataGenerationStatusWidget()),
        GenerateButton(new QPushButton("Generate")) {

    connect(GenerateButton, &QPushButton::clicked, this, [this]() { Generate(); });
}

auto DataGenerationWidget::ProgressCallback::operator()(double progress) -> void {
    std::scoped_lock<std::mutex> const lock { Mutex };

    ProgressBar.setValue(static_cast<int>(progress * 1000.0));
}

auto DataGenerationWidget::DataGenerationWidgetRow::AddToLayout(QGridLayout& gLayout) const noexcept -> void {
    int const currentRowCount = gLayout.rowCount();
    int const row = currentRowCount + 1;

    gLayout.addWidget(Name, row, 0);
    gLayout.addWidget(ProgressBar, row, 1);
    gLayout.addWidget(GenerateButton, row, 2);
    gLayout.addWidget(StatusWidget, row, 3);
}

auto DataGenerationWidget::DataGenerationWidgetRow::Generate() -> void {
    ParentWidget.DisableButtons();

    Generator(ProgressCallback { *ProgressBar, Mutex });

    ProgressBar->reset();

    ParentWidget.UpdateRowStatuses();
}

auto DataGenerationWidget::DataGenerationWidgetRow::SetButtonEnabled(bool enabled) const -> void {
    GenerateButton->setEnabled(enabled);
}

auto DataGenerationWidget::DataGenerationWidgetRow::UpdateStatus(Status status) const -> void {
    GenerateButton->setEnabled(status != Status::UP_TO_DATE);

    GenerateButton->setText(status == Status::OUT_OF_DATE ? "Update" : "Generate");

    StatusWidget->UpdateStatus(status);
}


DataGenerationStatusWidget::DataGenerationStatusWidget() :
        TextLabel([]() {
            auto* label = new QLabel("");
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
