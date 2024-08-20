#pragma once

#include <QDialog>
#include <QFrame>

class Pipeline;
class PipelineGroup;
class PipelineGroupList;
class PipelineGroupListView;

class NameLineEdit;
class QComboBox;
class QItemSelection;
class QItemSelectionModel;
class QPushButton;
class QSpinBox;


class PipelineGroupListWidget : public QFrame {
    Q_OBJECT

public:
    explicit PipelineGroupListWidget(PipelineGroupList& pipelineGroups);

public Q_SLOTS:
    void UpdatePipelineList() noexcept;

    void UpdateNumberOfPipelines();

Q_SIGNALS:
    void PipelineGroupChanged(PipelineGroup* pipelineGroup);

protected:
    void showEvent(QShowEvent* event) override;

private Q_SLOTS:
    void OnAddPipelineGroup();
    void OnRemovePipelineGroup();

    void OnSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected);

private:
    auto
    UpdateBasePipelineFilterComboBoxItems() noexcept -> void;

    auto
    UpdateButtonStatus() -> void;

    PipelineGroupList& PipelineGroups;

    QSpinBox* NumberOfPipelinesSpinBox;
    QPushButton* AddPipelineGroupButton;
    QPushButton* RemovePipelineGroupButton;
    QComboBox* BasePipelineFilterComboBox;
    PipelineGroupListView* ListView;
    QItemSelectionModel* SelectionModel;
};


class PipelineGroupCreateDialog : public QDialog {
public:
    explicit PipelineGroupCreateDialog(std::vector<std::pair<QString, QVariant>> const& options,
                                       QWidget* parent = nullptr);

    struct Data {
        std::string Name;
        Pipeline const* BasePipeline;
    };

    [[nodiscard]] auto
    GetData() const noexcept -> Data;

private:
    NameLineEdit* NameEdit;
    QComboBox* BasePipelineComboBox;
};
