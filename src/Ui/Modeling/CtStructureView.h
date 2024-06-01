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

    auto
    Select(BasicStructure const& basicStructure) -> void;

    auto
    Select(CombinedStructure const& combinedStructure) -> void;

Q_SIGNALS:
    void CtStructureChanged(idx_t structureIdx);

protected:
    void selectionChanged(QItemSelection const& selected, QItemSelection const& deselected) override;

private:
    friend class PipelineStructureArtifactsView;
    CtStructureTreeReadOnlyModel* StructureTreeModel;
};
