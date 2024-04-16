#include "CtStructureTreeModel.h"

#include "../CtStructureTree.h"
#include "../../Artifacts/Pipeline.h"

CtStructureTreeModel::CtStructureTreeModel(CtStructureTree& ctStructureTree, QObject* parent) :
        QAbstractItemModel(parent),
        Tree(ctStructureTree) {
}

auto CtStructureTreeModel::index(int row, int column, const QModelIndex& parentIndex) const -> QModelIndex {
    if (!hasIndex(row, column, parentIndex))
        return {};

    StructureIdx childStructureIdx;
    if (!parentIndex.isValid())
        childStructureIdx = Tree.GetRootIdx();
    else {
        const StructureIdx parentIdx = parentIndex.internalId();
        const StructureVariant& parentVariant = Tree.GetStructureAt(parentIdx);
        if (!holds_alternative<CombinedStructure>(parentVariant))
            throw std::runtime_error("Parent has to be a combined structure");

        const auto& parentStructure = std::get<CombinedStructure>(parentVariant);
        childStructureIdx = parentStructure.StructureIdxAt(row);
    }

    return createIndex(row, column, childStructureIdx);
}

QModelIndex CtStructureTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid())
        return {};

    const auto childIdx = static_cast<StructureIdx>(child.internalId());
    if (childIdx == Tree.GetRootIdx())
        return {};

    const auto& childVariant = Tree.GetStructureAt(childIdx);

    const StructureIdx parentIdx = std::visit([](auto& structure) { return structure.GetParentIdx(); }, childVariant);
    const auto& parentVariant = Tree.GetStructureAt(parentIdx);
    const auto& parentStructure = std::get<CombinedStructure>(parentVariant);

    const StructureId grandParentIdx = parentStructure.GetParentIdx();
    if (grandParentIdx < 0)
        return createIndex(Tree.GetRootIdx(), 0, parentIdx);

    const auto& grandParentVariant = Tree.GetStructureAt(grandParentIdx);
    const auto& grandParentStructure = std::get<CombinedStructure>(grandParentVariant);

    const StructureIdx parentChildIdx = grandParentStructure.PositionIndex(parentIdx);
    return createIndex(parentChildIdx, 0, parentIdx);
}

auto CtStructureTreeModel::rowCount(const QModelIndex& parent) const -> int {
    if (!parent.isValid())
        return Tree.HasRoot() ? 1 : 0;

    const StructureIdx parentIdx = parent.internalId();
    const auto& parentVariant = Tree.GetStructureAt(parentIdx);
    return std::visit(Overload {
            [](const CombinedStructure& combinedStructure) { return combinedStructure.StructureCount(); },
            [](const auto&) -> StructureIdx { return 0; }
        }, parentVariant);
}

auto CtStructureTreeModel::columnCount(const QModelIndex& parent) const -> int {
    return 1;
}

auto CtStructureTreeModel::data(const QModelIndex& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    const StructureIdx structureIdx = index.internalId();
    const auto& structureVariant = Tree.GetStructureAt(structureIdx);

    switch (role) {
        case Qt::DisplayRole:
            return QString::fromStdString(std::visit([](const auto& structure) { return structure.GetViewName(); },
                                                     structureVariant));

        case TreeModelRoles::STRUCTURE_DATA_VARIANT: {
            auto&& structureDataVariant
                    = std::visit([](auto& structure) { return StructureDataVariant{ structure.GetData()}; },
                                 structureVariant);
            return QVariant::fromValue(std::move(structureDataVariant));
        }

        case TreeModelRoles::IS_BASIC_STRUCTURE: {
            bool isBasic = std::visit(Overload{ [](const CombinedStructure&) { return false; },
                                                [](const auto&)              { return true;  } },
                              structureVariant);
            return isBasic;
        }

        default: return {};
    }
}

auto CtStructureTreeModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    return "Structures";
}

auto CtStructureTreeModel::flags(const QModelIndex& index) const -> Qt::ItemFlags {
    if (!index.isValid())
        return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

auto CtStructureTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool {
    if (!index.isValid())
        return false;

    Tree.SetData(index.internalId(), value);

    emit dataChanged(index, index);
    return true;
}

auto CtStructureTreeModel::AddBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                             const QModelIndex &siblingIndex) -> QModelIndex {
    const QModelIndex& parentIndex = siblingIndex.parent();
    auto *const parentStructure = parentIndex.isValid()
            ? std::get_if<CombinedStructure>(&Tree.GetStructureAt(parentIndex.internalId()))
            : nullptr;
    const int insertionIndex = parentStructure // iff parentIndex.isValid()
            ? parentStructure->StructureCount()
            : 0;

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);

    Tree.AddBasicStructure(basicStructureDataVariant, parentStructure);

    endInsertRows();

    QModelIndex newIndex = index(insertionIndex, 0, parentIndex);
    return newIndex;
}

void CtStructureTreeModel::CombineWithBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                                     const CombinedStructureData& combinedStructureData) {
    beginResetModel();

    Tree.CombineWithBasicStructure(basicStructureDataVariant, combinedStructureData);

    endResetModel();
}

void CtStructureTreeModel::RefineWithBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                                    const CombinedStructureData& combinedStructureData,
                                                    const QModelIndex& structureToRefineIndex) {
    const StructureIdx structureToRefineIdx = structureToRefineIndex.internalId();

    beginResetModel();
    Tree.RefineWithBasicStructure(basicStructureDataVariant, combinedStructureData, structureToRefineIdx);
    endResetModel();
}

void CtStructureTreeModel::RemoveBasicStructure(const QModelIndex &index) {
    const StructureIdx structureToRemoveIdx = index.internalId();

    beginResetModel();
    Tree.RemoveBasicStructure(structureToRemoveIdx);
    endResetModel();
}

auto CtStructureTreeModel::HasRoot() -> bool {
    return Tree.HasRoot();
}
