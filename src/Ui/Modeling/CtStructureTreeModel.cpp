#include "CtStructureTreeModel.h"

#include "../../Modeling/CtStructureTree.h"
#include "../../Artifacts/Pipeline.h"
#include "../../Artifacts/Structure/StructureArtifactListCollection.h"
#include "../../Utils/Overload.h"

CtStructureTreeModel::CtStructureTreeModel(CtStructureTree& ctStructureTree, QObject* parent) :
        QAbstractItemModel(parent),
        Tree(ctStructureTree) {
}

auto CtStructureTreeModel::index(int row, int column, const QModelIndex& parentIndex) const -> QModelIndex {
    if (!hasIndex(row, column, parentIndex))
        return {};

    idx_t childStructureIdx;
    if (!parentIndex.isValid())
        childStructureIdx = Tree.GetRootIdx();
    else {
        uidx_t const parentIdx = idx_t::SignedAsUnsignedIdx(parentIndex.internalId());
        auto const& parentStructure = std::get<CombinedStructure>(Tree.GetStructureAt(parentIdx));
        childStructureIdx = parentStructure.StructureIdxAt(row);
    }

    return createIndex(row, column, childStructureIdx.ToSigned());
}

auto CtStructureTreeModel::parent(const QModelIndex& child) const -> QModelIndex {
    if (!child.isValid())
        return {};

    auto const childIdx = static_cast<uidx_t>(child.internalId());
    if (childIdx == Tree.GetRootIdx())
        return {};

    auto const& childVariant = Tree.GetStructureAt(childIdx);

    uidx_t const parentIdx = std::visit([](auto& structure) { return *structure.ParentIdx; }, childVariant);
    auto const& parentVariant = Tree.GetStructureAt(parentIdx);
    auto const& parentStructure = std::get<CombinedStructure>(parentVariant);

    idx_t const grandParentIdx = parentStructure.ParentIdx;
    if (grandParentIdx < 0)
        return createIndex(Tree.GetRootIdx().ToSigned(), 0, parentIdx);

    auto const& grandParentVariant = Tree.GetStructureAt(grandParentIdx.ToUnsigned());
    auto const& grandParentStructure = std::get<CombinedStructure>(grandParentVariant);

    uidx_t const parentChildIdx = grandParentStructure.PositionIndex(parentIdx);
    return createIndex(parentChildIdx, 0, parentIdx);
}

auto CtStructureTreeModel::rowCount(const QModelIndex& parent) const -> int {
    if (!parent.isValid())
        return Tree.HasRoot() ? 1 : 0;

    const uidx_t parentIdx = parent.internalId();
    const auto& parentVariant = Tree.GetStructureAt(parentIdx);
    return std::visit(Overload {
            [](const CombinedStructure& combinedStructure) { return combinedStructure.StructureCount(); },
            [](const BasicStructure&) -> uidx_t { return 0; }
        }, parentVariant);
}

auto CtStructureTreeModel::columnCount(const QModelIndex& /*parent*/) const -> int {
    return 1;
}

auto CtStructureTreeModel::data(const QModelIndex& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    const uidx_t structureIdx = index.internalId();
    const auto& structureVariant = Tree.GetStructureAt(structureIdx);

    switch (role) {
        case Qt::DisplayRole:
            return QString::fromStdString(
                    std::visit([](const auto& structure) { return structure.GetViewName(); },
                               structureVariant));

        case STRUCTURE_DATA_VARIANT: {
            auto const& structureDataVariant
                    = std::visit(Overload {
                            [](BasicStructure const& structure) {
                                    BasicStructureData data {};
                                    data.PopulateFromStructure(structure);
                                    return StructureDataVariant { data };
                                },
                            [](CombinedStructure const& structure) {
                                CombinedStructureData data {};
                                data.PopulateFromStructure(structure);
                                return StructureDataVariant { data };
                            }
                        }, structureVariant);
            return QVariant::fromValue(std::move(structureDataVariant));
        }

        case IS_BASIC_STRUCTURE: {
            bool const isBasic = std::visit(Overload { [](const CombinedStructure&) { return false; },
                                                 [](const BasicStructure&)    { return true;  } },
                              structureVariant);
            return isBasic;
        }

        case POINTER_CONST:
            return std::visit([](auto const& structure) { return QVariant::fromValue(&structure); },
                              structureVariant);

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

    Q_EMIT dataChanged(index, index);
    return true;
}

auto CtStructureTreeModel::AddBasicStructure(const BasicStructureData& basicStructureData,
                                             const QModelIndex &siblingIndex) -> QModelIndex {
    const QModelIndex& parentIndex = siblingIndex.parent();
    auto *const parentStructure = parentIndex.isValid()
            ? std::get_if<CombinedStructure>(&Tree.GetStructureAt(parentIndex.internalId()))
            : nullptr;
    const int insertionIndex = parentStructure // iff parentIndex.isValid()
            ? parentStructure->StructureCount()
            : 0;

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);

    Tree.AddBasicStructure(basicStructureData, parentStructure);

    endInsertRows();

    QModelIndex const newIndex = index(insertionIndex, 0, parentIndex);
    return newIndex;
}

void CtStructureTreeModel::CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                                                     const CombinedStructureData& combinedStructureData) {
    beginResetModel();

    Tree.CombineWithBasicStructure(basicStructureData, combinedStructureData);

    endResetModel();
}

void CtStructureTreeModel::RefineWithBasicStructure(const BasicStructureData& basicStructureData,
                                                    const CombinedStructureData& combinedStructureData,
                                                    const QModelIndex& structureToRefineIndex) {
    const uidx_t structureToRefineIdx = structureToRefineIndex.internalId();

    beginResetModel();
    Tree.RefineWithBasicStructure(basicStructureData, combinedStructureData, structureToRefineIdx);
    endResetModel();
}

void CtStructureTreeModel::RemoveBasicStructure(const QModelIndex &index) {
    const uidx_t structureToRemoveIdx = index.internalId();

    beginResetModel();
    Tree.RemoveBasicStructure(structureToRemoveIdx);
    endResetModel();
}

auto CtStructureTreeModel::HasRoot() const -> bool {
    return Tree.HasRoot();
}

CtStructureTreeReadOnlyModel::CtStructureTreeReadOnlyModel(CtStructureTree const& ctStructureTree, QObject*) :
        CtStructureTreeModel(const_cast<CtStructureTree&>(ctStructureTree)) {}

auto CtStructureTreeReadOnlyModel::flags(QModelIndex const& index) const -> Qt::ItemFlags {
    return QAbstractItemModel::flags(index);
}

CtStructureArtifactsModel::CtStructureArtifactsModel(CtStructureTree& ctStructureTree,
                                                     Pipeline& pipeline,
                                                     QObject*) :
        CtStructureTreeModel(ctStructureTree),
        Pipeline_(pipeline) {}

auto CtStructureArtifactsModel::data(QModelIndex const& index, int role) const -> QVariant {
    if (!index.isValid() || role != Qt::DisplayRole)
        return CtStructureTreeModel::data(index, role);

    uidx_t const structureIdx = index.internalId();

    auto const& structureVariant = Tree.GetStructureAt(structureIdx);
    auto const viewName = std::visit([](const auto& structure) { return structure.GetViewName(); },
                                     structureVariant);

    auto const& structureArtifacts = Pipeline_.GetStructureArtifactList(structureIdx);
    std::vector<std::string> artifactNames {};
    for (auto const& artifact : structureArtifacts.Artifacts)
        artifactNames.emplace_back(artifact.GetViewName());

    std::stringstream structureArtifactNamesStream;
    for (auto const& name : artifactNames)
        structureArtifactNamesStream << name << ", ";
    std::string structureArtifactNames = structureArtifactNamesStream.str();
    if (!structureArtifactNames.empty())
        structureArtifactNames.erase(structureArtifactNames.size() - 2);

    return QString::fromStdString(std::format("{} {{ {} }}", viewName, structureArtifactNames));
}
