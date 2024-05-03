#pragma once

#include "../../WidgetUtils.h"

#include <QListView>

class StructureArtifactList;

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
