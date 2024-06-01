#include "CtStructureView.h"

#include "CtStructureDialog.h"
#include "CtStructureTreeModel.h"
#include "../Utils/ModelUtils.h"
#include "../../Modeling/CtStructureTree.h"
#include "../../Utils/Overload.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>

CtStructureView::CtStructureView(CtStructureTree& ctStructureTree) {
    setModel(new CtStructureTreeModel(ctStructureTree));
    setItemDelegate(new CtStructureDelegate());
}

CtStructureView::CtStructureDelegate::CtStructureDelegate(QObject* parent) : DialogDelegate(parent) {}

auto CtStructureView::CtStructureDelegate::getDialog(QModelIndex const& modelIndex,
                                                     QWidget* parent) const noexcept -> QDialog* {
    auto const& dataVariant = modelIndex.data(Qt::UserRole).value<StructureDataVariant>();

    CtStructureDialog* dialog = std::visit(Overload {
            [=](CombinedStructureData const&) -> CtStructureDialog* {
                return new CombinedStructureDialog(CtStructureDialog::DialogMode::EDIT, parent);
            },
            [=](BasicStructureData const&) -> CtStructureDialog*    {
                return new BasicStructureDialog(CtStructureDialog::DialogMode::EDIT, parent);
            },
    }, dataVariant);

    return dialog;
}

void CtStructureView::CtStructureDelegate::setEditorData(QWidget* editor, QModelIndex const& index) const {
    auto const& dataVariant = index.data(Qt::UserRole).value<StructureDataVariant>();

    std::visit(Overload {
        [editor](BasicStructureData const& data)    { SetWidgetData<BasicStructureWidget>(editor, data); },
        [editor](CombinedStructureData const& data) { SetWidgetData<CombinedStructureWidget>(editor, data); }
    }, dataVariant);
}

void CtStructureView::CtStructureDelegate::setModelData(QWidget* editor,
                                                        QAbstractItemModel* model,
                                                        QModelIndex const& index) const {
    QVariant const editedData = index.data(TreeModelRoles::IS_BASIC_STRUCTURE).toBool()
            ? QVariant::fromValue(GetWidgetData<BasicStructureWidget>(editor))
            : QVariant::fromValue(GetWidgetData<CombinedStructureWidget>(editor));

    model->setData(index, editedData);
}

CtStructureReadOnlyView::CtStructureReadOnlyView(CtStructureTree const& ctStructureTree) :
        CtStructureView(const_cast<CtStructureTree&>(ctStructureTree)),
        StructureTreeModel(new CtStructureTreeReadOnlyModel(ctStructureTree)) {

    setModel(StructureTreeModel);
}

auto CtStructureReadOnlyView::model() const noexcept -> CtStructureTreeReadOnlyModel* {
    return StructureTreeModel;
}

void CtStructureReadOnlyView::selectionChanged(QItemSelection const& selected, QItemSelection const& deselected) {
    QTreeView::selectionChanged(selected, deselected);

    QModelIndexList const selectedIndices = selected.indexes();

    if (selectedIndices.empty()) {
        Q_EMIT CtStructureChanged({});
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);

    auto structureIdx = idx_t::FromSigned(selectedIndex.internalId());

    Q_EMIT CtStructureChanged(structureIdx);
}

auto CtStructureReadOnlyView::Select(BasicStructure const& basicStructure) -> void {
    auto match = Search(*StructureTreeModel, TreeModelRoles::POINTER_CONST,
                        QVariant::fromValue(&basicStructure), rootIndex());

    if (match == QModelIndex{})
        throw std::runtime_error("Basic structure not found");

    collapseAll();
    expand(match);

    selectionModel()->clearSelection();
    selectionModel()->select(match, QItemSelectionModel::SelectionFlag::Select);
}

auto CtStructureReadOnlyView::Select(CombinedStructure const& combinedStructure) -> void {
    auto match = Search(*StructureTreeModel, TreeModelRoles::POINTER_CONST,
                        QVariant::fromValue(&combinedStructure), rootIndex());

    if (match == QModelIndex{})
        throw std::runtime_error("Combined structure not found");

    selectionModel()->clearSelection();
    selectionModel()->select(match, QItemSelectionModel::SelectionFlag::Select);
}
