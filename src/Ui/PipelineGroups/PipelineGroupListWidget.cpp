#include "PipelineGroupListWidget.h"

#include "PipelineGroupListView.h"
#include "../Utils/NameLineEdit.h"
#include "../Utils/WidgetUtils.h"
#include "../../PipelineGroups/PipelineGroupList.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

PipelineGroupListWidget::PipelineGroupListWidget(PipelineGroupList& pipelineGroups) :
        PipelineGroups(pipelineGroups),
        NumberOfPipelinesSpinBox([&pipelineGroups]() {
            auto* spinBox = new QSpinBox();
            spinBox->setRange(0, 10000);
            spinBox->setValue(pipelineGroups.GetNumberOfPipelines());
            spinBox->setEnabled(false);
            return spinBox;
        }()),
        AddPipelineGroupButton([]() {
            auto* button = new QPushButton();
            button->setIcon(GenerateIcon("Plus"));
            return button;
        }()),
        RemovePipelineGroupButton([]() {
            auto* button = new QPushButton();
            button->setIcon(GenerateIcon("Minus"));
            return button;
        }()),
        BasePipelineFilterComboBox(new QComboBox()),
        ListView(new PipelineGroupListView(pipelineGroups)),
        SelectionModel(ListView->selectionModel()) {

    auto* fLayout = new QFormLayout(this);

    auto* title = new QLabel("Pipeline Groups");
    title->setStyleSheet(GetHeader1StyleSheet());
    fLayout->addRow(title);

    fLayout->addRow("Number of pipelines", NumberOfPipelinesSpinBox);

    fLayout->addRow("Filter by pipeline", BasePipelineFilterComboBox);
    UpdateBasePipelineFilterComboBoxItems();

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->addStretch();
    buttonBarHLayout->addWidget(AddPipelineGroupButton);
    buttonBarHLayout->addWidget(RemovePipelineGroupButton);
    fLayout->addRow(buttonBar);

    fLayout->addRow(ListView);

    UpdateButtonStatus();

    connect(AddPipelineGroupButton, &QPushButton::clicked, this, &PipelineGroupListWidget::OnAddPipelineGroup);
    connect(RemovePipelineGroupButton, &QPushButton::clicked, this, &PipelineGroupListWidget::OnRemovePipelineGroup);

    connect(SelectionModel, &QItemSelectionModel::selectionChanged, this, &PipelineGroupListWidget::OnSelectionChanged);
}

void PipelineGroupListWidget::UpdatePipelineList() noexcept {
    UpdateBasePipelineFilterComboBoxItems();
}

void PipelineGroupListWidget::UpdateNumberOfPipelines() {
    NumberOfPipelinesSpinBox->setValue(PipelineGroups.GetNumberOfPipelines());
}

auto PipelineGroupListWidget::UpdateBasePipelineFilterComboBoxItems() noexcept -> void {
    BasePipelineFilterComboBox->clear();

    BasePipelineFilterComboBox->addItem("All");
    for (auto const* pipeline : PipelineGroups.GetBasePipelines())
        BasePipelineFilterComboBox->addItem(QString::fromStdString(pipeline->GetName()),
                                            QVariant::fromValue(pipeline));
}

auto PipelineGroupListWidget::UpdateButtonStatus() -> void {
    bool const isValid = !SelectionModel->selection().indexes().empty();

    AddPipelineGroupButton->setEnabled(true);
    RemovePipelineGroupButton->setEnabled(isValid);
}

void PipelineGroupListWidget::OnAddPipelineGroup() {
    std::vector<std::pair<QString, QVariant>> options;

    for (auto const* pipeline : PipelineGroups.GetBasePipelines())
        options.emplace_back(QString::fromStdString(pipeline->GetName()),
                             QVariant::fromValue(pipeline));

    auto* dialog = new PipelineGroupCreateDialog(options, this);

    connect(dialog, &PipelineGroupCreateDialog::accepted, [this, dialog]() {
        auto const data = dialog->GetData();
        QModelIndex const index = ListView->model()->AddPipelineGroup(*data.BasePipeline, data.Name);

        SelectionModel->clearSelection();
        SelectionModel->select(index, QItemSelectionModel::SelectionFlag::Select);
        UpdateButtonStatus();
        UpdateNumberOfPipelines();
    });

    dialog->show();
}

void PipelineGroupListWidget::OnRemovePipelineGroup() {
    QModelIndex const index = SelectionModel->selection().indexes().at(0);
    ListView->model()->RemovePipelineGroup(index);

    SelectionModel->clearSelection();
    UpdateButtonStatus();
}

void PipelineGroupListWidget::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QModelIndexList const selectedIndices = selected.indexes();

    if (selectedIndices.empty()) {
        Q_EMIT PipelineGroupChanged(nullptr);
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);

    auto* pipelineGroup = selectedIndex.data(Qt::UserRole).value<PipelineGroup*>();

    Q_EMIT PipelineGroupChanged(pipelineGroup);
}


PipelineGroupCreateDialog::PipelineGroupCreateDialog(std::vector<std::pair<QString, QVariant>> const& options,
                                                     QWidget* parent) :
        QDialog(parent),
        NameEdit(new NameLineEdit()),
        BasePipelineComboBox([&]() -> QComboBox* {
            auto* comboBox = new QComboBox();
            for (auto const& option : options)
                comboBox->addItem(option.first, option.second);
            return comboBox;
        }()) {

    auto* fLayout = new QFormLayout(this);

    setMinimumSize(200, 100);

    setModal(true);

    setWindowTitle("Create Pipeline Group");

    fLayout->setAlignment(Qt::AlignTop);

    fLayout->addRow("Name", NameEdit);
    fLayout->addRow("Base Pipeline", BasePipelineComboBox);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    fLayout->addWidget(dialogButtonBar);

    connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

auto PipelineGroupCreateDialog::GetData() const noexcept -> Data {
    return { NameEdit->GetText().toStdString(),
             BasePipelineComboBox->currentData().value<Pipeline const*>() };
}
