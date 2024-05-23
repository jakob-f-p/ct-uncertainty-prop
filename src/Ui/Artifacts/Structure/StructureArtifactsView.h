#pragma once

#include "../../Utils/WidgetUtils.h"

#include <QListView>

class StructureArtifact;
class StructureArtifactList;
class StructureArtifactsReadOnlyModel;


class StructureArtifactsView : public QListView {
public:
    explicit StructureArtifactsView(StructureArtifactList& structureArtifactList);

private:
    class StructureArtifactsDelegate : public DialogDelegate {
    public:
        explicit StructureArtifactsDelegate(QObject* parent = nullptr);

        auto
        getDialog(const QModelIndex& index, QWidget* parent) const noexcept -> QDialog* override;

        void setEditorData(QWidget* editor, const QModelIndex& index) const override;

        void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    };
};


class StructureArtifactsReadOnlyView : public QListView {
    Q_OBJECT

public:
    explicit StructureArtifactsReadOnlyView(StructureArtifactList const& structureArtifactList);

signals:
    void StructureArtifactChanged(StructureArtifact* structureArtifact);

private slots:
    void OnSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected);

private:
    StructureArtifactsReadOnlyModel* Model;
};
