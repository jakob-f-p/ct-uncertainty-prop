#include "StructureArtifactsModel.h"

#include "../../../Artifacts/Structure/StructureArtifact.h"
#include "../../../Artifacts/Structure/StructureArtifactListCollection.h"

StructureArtifactsModel::StructureArtifactsModel(StructureArtifactList& artifactList,
                                                 QObject* parent) :
        QAbstractItemModel(parent),
        StructureWrapper(artifactList) {
}

auto StructureArtifactsModel::index(int row, int column, QModelIndex const& parent) const -> QModelIndex {
    if (!hasIndex(row, column, parent))
        return {};

    auto* structureArtifact = &StructureWrapper.Get(row);

    return createIndex(row, column, structureArtifact);
}

auto StructureArtifactsModel::parent(QModelIndex const& child) const -> QModelIndex {
    return {};
}

auto StructureArtifactsModel::rowCount(QModelIndex const& parent) const -> int {
    if (parent.isValid())
        return 0;

    return StructureWrapper.GetNumberOfArtifacts();
}

auto StructureArtifactsModel::columnCount(QModelIndex const& parent) const -> int {
    return 1;
}

auto StructureArtifactsModel::data(QModelIndex const& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());

    if (!artifact)
        throw std::runtime_error("Artifact must not be null");

    switch (role) {
        case Qt::DisplayRole: return QString::fromStdString(artifact->GetViewName());

        case Roles::DATA: { // Qt::UserRole
            StructureArtifactData data {};

            data.PopulateFromArtifact(*artifact);

            return QVariant::fromValue(data);
        }

        case Roles::POINTER: return QVariant::fromValue(artifact);

        default: return {};
    }
}

auto StructureArtifactsModel::flags(QModelIndex const& index) const -> Qt::ItemFlags {
    if (!index.isValid()) return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

auto StructureArtifactsModel::setData(QModelIndex const& index, QVariant const& value, int role) -> bool {
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());
    auto data = value.value<StructureArtifactData>();
    data.PopulateArtifact(*artifact);

    emit dataChanged(index, index);

    return true;
}

auto StructureArtifactsModel::AddStructureArtifact(StructureArtifactData const& data,
                                                   QModelIndex const& siblingIndex) -> QModelIndex {
    StructureArtifact artifact(data);

    int const insertionIndex = siblingIndex.isValid()
            ? siblingIndex.row() + 1
            : StructureWrapper.GetNumberOfArtifacts();

    beginInsertRows({}, insertionIndex, insertionIndex);
    StructureWrapper.AddStructureArtifact(std::move(artifact), insertionIndex);
    endInsertRows();

    return index(insertionIndex, 0, {});
}

void StructureArtifactsModel::RemoveStructureArtifact(QModelIndex const& index) {
    if (!index.isValid())
        throw std::runtime_error("Cannot remove image artifact. Given index is not valid");

    auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());

    beginRemoveRows({}, index.row(), index.row());
    StructureWrapper.RemoveStructureArtifact(*artifact);
    endRemoveRows();
}

auto StructureArtifactsModel::MoveUp(QModelIndex const& index) -> QModelIndex {
    if (!index.isValid() || index.row() == 0)
        throw std::runtime_error("Cannot move image artifact up. Given index is not valid or it is first child");

    return Move(index, -1);
}

auto StructureArtifactsModel::MoveDown(QModelIndex const& index) -> QModelIndex {
    if (!index.isValid() || index.row() == rowCount({}) - 1)
        throw std::runtime_error("Cannot move image artifact up. Given index is not valid or it is last child");

    return Move(index, 1);
}

auto StructureArtifactsModel::Move(QModelIndex const& sourceIndex, int displacement) -> QModelIndex {
    auto* structureArtifact = static_cast<StructureArtifact*>(sourceIndex.internalPointer());

    int const prevIdx = sourceIndex.row();
    int const newIdx = sourceIndex.row() + displacement;

    beginMoveRows({}, prevIdx, prevIdx,
                  {}, newIdx > prevIdx ? newIdx + 1 : newIdx);
    StructureWrapper.MoveStructureArtifact(*structureArtifact, newIdx);
    endMoveRows();

    return index(newIdx, 0, {});
}

StructureArtifactsReadOnlyModel::StructureArtifactsReadOnlyModel(StructureArtifactList const& artifactList,
                                                                 QObject* parent) :
        StructureArtifactsModel(const_cast<StructureArtifactList&>(artifactList), parent) {}

auto StructureArtifactsReadOnlyModel::flags(QModelIndex const& index) const -> Qt::ItemFlags {
    return QAbstractItemModel::flags(index);
}

auto StructureArtifactsReadOnlyModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    return "Artifacts";
}
