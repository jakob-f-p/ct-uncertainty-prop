#include "CtDataCsgTreeModel.h"

CtDataCsgTreeModel::CtDataCsgTreeModel(CtDataCsgTree& csgTree, QObject* parent)
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
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole)) {
        return {};
    }

    auto* item = static_cast<const CtStructure*>(index.internalPointer());

    if (role == Qt::UserRole) {
        return item->IsImplicitCtStructure();
    }

    return item->Data();
}

QVariant CtDataCsgTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    return "Structures";
}

Qt::ItemFlags CtDataCsgTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return {};
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool CtDataCsgTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) {
        return false;
    }

    auto* ctStructure = static_cast<CtStructure*>(index.internalPointer());
    ctStructure->SetData(value);

    emit dataChanged(index, index);
    return true;
}

QModelIndex CtDataCsgTreeModel::AddImplicitCtStructure(const ImplicitCtStructureDetails &implicitCtStructureDetails,
                                                       const QModelIndex &siblingIndex) {
    const QModelIndex& parentIndex = siblingIndex.parent();
    auto* parent = static_cast<ImplicitStructureCombination*>(parentIndex.internalPointer());
    int insertionIndex = parent ? parent->ChildCount() : 0;

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);

    Tree.AddImplicitCtStructure(implicitCtStructureDetails, parent);

    endInsertRows();

    QModelIndex newIndex = index(insertionIndex, 0, parentIndex);
    return newIndex;
}

void CtDataCsgTreeModel::CombineWithImplicitCtStructure(ImplicitCtStructureDetails& implicitCtStructureDetails) {
    beginResetModel();

    Tree.CombineWithImplicitCtStructure(implicitCtStructureDetails);

    endResetModel();
}

void CtDataCsgTreeModel::RefineWithImplicitStructure(const ImplicitCtStructureDetails& implicitCtStructureDetails,
                                                     const QModelIndex& index) {
    beginResetModel();

    auto* structureToRefine = static_cast<ImplicitCtStructure*>(index.internalPointer());
    Tree.RefineWithImplicitStructure(implicitCtStructureDetails, *structureToRefine);

    endResetModel();
}

void CtDataCsgTreeModel::RemoveImplicitCtStructure(const QModelIndex &implicitCtStructureIndex) {
    beginResetModel();

    auto* implicitCtStructure = static_cast<ImplicitCtStructure*>(implicitCtStructureIndex.internalPointer());
    Tree.RemoveImplicitCtStructure(*implicitCtStructure);

    endResetModel();
}

bool CtDataCsgTreeModel::HasRoot() {
    return Tree.GetRoot();
}
