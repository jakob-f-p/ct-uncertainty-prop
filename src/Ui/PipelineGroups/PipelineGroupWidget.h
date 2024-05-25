#pragma once

#include <QWidget>

struct ArtifactVariantPointer;
class PipelineGroup;
class PipelineParameterSpaceView;
class PipelineParameterSpan;
class PipelineParameterSpanSet;

class QItemSelection;
class QItemSelectionModel;
class QPushButton;
class QSpinBox;


class PipelineGroupWidget : public QWidget {
    Q_OBJECT

public:
    explicit PipelineGroupWidget(PipelineGroup& pipelineGroup, QWidget* parent = nullptr);

    void AddParameterSpan(PipelineParameterSpan&& parameterSpan);

public slots:
    void UpdateNumberOfPipelines();

signals:
    void ParameterSpanChanged(PipelineParameterSpan* parameterSpan);

    void RequestCreateParameterSpan();

    void NumberOfPipelinesUpdated();

private slots:
    void OnRemoveParameterSpan();

    void OnSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected);

private:
    auto
    UpdateButtonStatus() -> void;

    PipelineGroup const& Group;

    QSpinBox* NumberOfPipelinesSpinBox;
    QPushButton* AddParameterSpanButton;
    QPushButton* RemoveParameterSpanButton;
    PipelineParameterSpaceView* ParameterSpaceView;
    QItemSelectionModel* SelectionModel;
};
