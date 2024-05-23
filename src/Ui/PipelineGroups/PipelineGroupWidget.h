#pragma once

#include <QWidget>

struct ArtifactVariantPointer;
class PipelineGroup;
class PipelineParameterSpaceView;
class PipelineParameterSpan;

class QItemSelection;
class QItemSelectionModel;
class QPushButton;


class PipelineGroupWidget : public QWidget {
    Q_OBJECT

public:
    explicit PipelineGroupWidget(PipelineGroup& pipelineGroup, QWidget* parent = nullptr);

signals:
    void ParameterSpanChanged(PipelineParameterSpan* parameterSpan, ArtifactVariantPointer artifactVariantPointer);

    void RequestCreateParameterSpan();

private slots:
    void OnRemoveParameterSpan();

    void OnSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected);

private:
    void AddParameterSpan(ArtifactVariantPointer artifactVariantPointer, PipelineParameterSpan&& parameterSpan);

    auto
    UpdateButtonStatus() -> void;

    PipelineGroup const& Group;

    QPushButton* AddParameterSpanButton;
    QPushButton* RemoveParameterSpanButton;
    PipelineParameterSpaceView* ParameterSpaceView;
    QItemSelectionModel* SelectionModel;
};
