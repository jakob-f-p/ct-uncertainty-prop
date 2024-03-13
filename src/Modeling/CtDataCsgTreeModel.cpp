#include "CtDataCsgTreeModel.h"

CtDataCsgTreeModel::CtDataCsgTreeModel(const CtDataCsgTree& csgTree, QObject* parent)
        : Tree(csgTree) {

}

QModelIndex CtDataCsgTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    auto* childItem = parent.isValid()
            ? static_cast<CtStructure*>(parent.internalPointer())->ChildAt(row)
            : Tree.GetRoot();

    if (!childItem) {
        return {};
    }

    auto idx = createIndex(row, column, childItem);
    return idx;
//    return createIndex(row, column, childItem);
}

QModelIndex CtDataCsgTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    auto* childItem = static_cast<const CtStructure*>(child.internalPointer());
    if (childItem == Tree.GetRoot()) {
        return {};
    }

    CtStructure* parentItem = childItem->GetParent();
    auto res = createIndex(parentItem->ChildIndex(), 0, parentItem);
    return res;
//    return createIndex(parentItem->ChildIndex(), 0, parentItem);
}

int CtDataCsgTreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) { // assume parent is the imaginary parent item of root item (if a root item is present)
        return Tree.GetRoot() ? 1 : 0;
    }

    auto* parentItem = static_cast<const CtStructure*>(parent.internalPointer());
    int res = parentItem->ChildCount();
    return res;
//    return static_cast<const CtStructure*>(parent.internalPointer())->ChildCount();
}

int CtDataCsgTreeModel::columnCount(const QModelIndex& parent) const {
    if (!parent.isValid()) { // assume parent is the imaginary parent item of root item
        return Tree.GetRoot() ? Tree.GetRoot()->ColumnCount() : 0;
    }

    return static_cast<const CtStructure*>(parent.internalPointer())->ColumnCount();
}

QVariant CtDataCsgTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }

    auto* item = static_cast<const CtStructure*>(index.internalPointer());
    return item->Data(index.column());
}
