#pragma once

#include <QWidget>

class CtStructureTreeModel;
class Pipeline;
class StructureArtifactsWidgetDialog;

class QStackedLayout;

class StructureArtifactsWidget : public QWidget {
    Q_OBJECT

public:
    explicit StructureArtifactsWidget(QWidget* parent = nullptr);

    void SetCurrentView(int pipelineIdx);
    void AddView(Pipeline& pipeline);
    void RemoveCurrentView();

public slots:
    void ResetModel();

private:
    QStackedLayout* Views;

    StructureArtifactsWidgetDialog* Dialog;
};
