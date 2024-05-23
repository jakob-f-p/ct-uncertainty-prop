#pragma once

#include "../Utils/WidgetUtils.h"
#include "../../Utils/Types.h"

#include <QTreeView>

class BasicStructure;
class CombinedStructure;
class CtStructureTree;
class CtStructureTreeReadOnlyModel;


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



class CtStructureReadOnlyView : public CtStructureView {
    Q_OBJECT

public:
    explicit CtStructureReadOnlyView(CtStructureTree const& ctStructureTree);

    [[nodiscard]] auto
    model() const noexcept -> CtStructureTreeReadOnlyModel*;

signals:
    void CtStructureChanged(idx_t structureIdx);

private slots:
    void OnSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected);

private:
    CtStructureTreeReadOnlyModel* StructureTreeModel;
};
