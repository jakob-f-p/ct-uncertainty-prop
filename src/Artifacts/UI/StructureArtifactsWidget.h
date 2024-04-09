#pragma once

#include <QStackedLayout>
#include <QWidget>
#include <QTreeView>

class CtStructureTreeModel;
class Pipeline;
class StructureArtifactsDialog;

class StructureArtifactsWidget : public QWidget {
    Q_OBJECT

public:
    explicit StructureArtifactsWidget(QWidget* parent = nullptr);

    void SetCurrentView(int pipelineIdx);
    void AddView(Pipeline* pipeline);
    void RemoveCurrentView();

private:
    QTreeView* GetCurrentView();
    CtStructureTreeModel* GetCurrentModel();
    QItemSelectionModel* GetCurrentSelectionModel();

    QStackedLayout* Views;

    StructureArtifactsDialog* Dialog;
};
