#pragma once

#include "../../Utils/WidgetUtils.h"

#include <QTreeView>

class CtStructureTree;


class CtStructureView : public QTreeView {
public:
    explicit CtStructureView(CtStructureTree& ctStructureTree);

private:
    class CtStructureDelegate : public DialogDelegate {
    public:
        explicit CtStructureDelegate(QObject* parent = nullptr);

        [[nodiscard]] auto
        getDialog(QModelIndex const& modelIndex, QWidget* parent) const noexcept -> QDialog* override;

        void setEditorData(QWidget* editor, const QModelIndex& index) const override;

        void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    };
};
