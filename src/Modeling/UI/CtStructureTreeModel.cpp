#include "CtStructureTreeModel.h"

#include "../CtStructureTree.h"
#include "../../Artifacts/Pipeline.h"

CtStructureTreeModel::CtStructureTreeModel(CtStructureTree& ctStructureTree, QObject* parent)
        : Tree(ctStructureTree) {
    Tree.Register(nullptr);
}

CtStructureTreeModel::CtStructureTreeModel(Pipeline* pipeline, QObject* parent)
        : Tree(*pipeline->GetCtDataTree()) {
    Tree.Register(nullptr);
}

CtStructureTreeModel::~CtStructureTreeModel() {
    Tree.Delete();
}

QModelIndex CtStructureTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    auto* childStructure = !parent.isValid()
            ? Tree.GetRoot()
            : static_cast<CtStructure*>(parent.internalPointer())->IsBasicStructure()
                    ? nullptr
                    : static_cast<CombinedStructure*>(parent.internalPointer())->ChildAt(row);

    if (!childStructure)
        return {};

    return createIndex(row, column, childStructure);
}

QModelIndex CtStructureTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid())
        return {};

    auto* childStructure = static_cast<const CtStructure*>(child.internalPointer());

    CombinedStructure* parentStructure = childStructure->GetParent();
    if (!parentStructure)
        return {};

    CombinedStructure* grandparentStructure = parentStructure->GetParent();
    int rowIdx = grandparentStructure
                        ? grandparentStructure->ChildIndex(*parentStructure)
                        : 0;
    return createIndex(rowIdx, 0, parentStructure);
}

int CtStructureTreeModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid())
        return Tree.GetRoot() ? 1 : 0;

    auto* parentItem = static_cast<const CtStructure*>(parent.internalPointer());
    if (parentItem->IsBasicStructure())
        return 0;

    return dynamic_cast<const CombinedStructure*>(parentItem)->ChildCount();
}

int CtStructureTreeModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QVariant CtStructureTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return {};

    auto* item = static_cast<const CtStructure*>(index.internalPointer());

    return item->Data();
}

QVariant CtStructureTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    return "Structures";
}

Qt::ItemFlags CtStructureTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool CtStructureTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid())
        return false;

    auto* ctStructure = static_cast<CtStructure*>(index.internalPointer());
    Tree.SetData(ctStructure, value);

    emit dataChanged(index, index);
    return true;
}

QModelIndex CtStructureTreeModel::AddBasicStructure(const BasicStructureDetails &basicStructureDetails,
                                                    const QModelIndex &siblingIndex) {
    const QModelIndex& parentIndex = siblingIndex.parent();
    auto* parent = static_cast<CombinedStructure*>(parentIndex.internalPointer());
    int insertionIndex = parent ? parent->ChildCount() : 0;

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);

    Tree.AddBasicStructure(basicStructureDetails, parent);

    endInsertRows();

    QModelIndex newIndex = index(insertionIndex, 0, parentIndex);
    return newIndex;
}

void CtStructureTreeModel::CombineWithBasicStructure(BasicStructureDetails& basicStructureDetails) {
    beginResetModel();

    Tree.CombineWithBasicStructure(basicStructureDetails);

    endResetModel();
}

void CtStructureTreeModel::RefineWithBasicStructure(const BasicStructureDetails& basicStructureDetails,
                                                    const QModelIndex& index) {
    beginResetModel();

    auto* structureToRefine = static_cast<BasicStructure*>(index.internalPointer());
    Tree.RefineWithBasicStructure(basicStructureDetails, *structureToRefine);

    endResetModel();
}

void CtStructureTreeModel::RemoveBasicStructure(const QModelIndex &index) {
    beginResetModel();

    auto* basicStructure = static_cast<BasicStructure*>(index.internalPointer());
    Tree.RemoveBasicStructure(*basicStructure);

    endResetModel();
}

bool CtStructureTreeModel::HasRoot() {
    return Tree.GetRoot();
}
