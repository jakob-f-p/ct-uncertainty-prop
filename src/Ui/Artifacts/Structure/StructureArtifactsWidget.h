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

    void SetCurrentView(int pipelineIdx) const;
    void AddView(Pipeline& pipeline) const;
    void RemoveCurrentView() const;

public Q_SLOTS:
    void ResetModel() const;

private:
    QStackedLayout* Views;

    StructureArtifactsWidgetDialog* Dialog;
};
