#include "StructureArtifactsModel.h"

#include "../StructureArtifact.h"
#include "../StructureWrapper.h"

#include <vtkSmartPointer.h>

StructureArtifactsModel::StructureArtifactsModel(StructureArtifacts& structureWrapper,
                                                 QObject* parent) :
        QAbstractItemModel(parent),
        StructureWrapper(structureWrapper) {
}

QModelIndex StructureArtifactsModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    auto* structureArtifact = &StructureWrapper.Get(row);

    return createIndex(row, column, structureArtifact);
}

QModelIndex StructureArtifactsModel::parent(const QModelIndex& child) const {
    return {};
}

int StructureArtifactsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;

    return StructureWrapper.GetNumberOfArtifacts();
}

int StructureArtifactsModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QVariant StructureArtifactsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());

    return QString::fromStdString(artifact->GetViewName());
}

Qt::ItemFlags StructureArtifactsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool StructureArtifactsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) {
        return false;
    }

    auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());
    StructureArtifactData::SetData(*artifact, value);

    emit dataChanged(index, index);
    return true;
}

QModelIndex StructureArtifactsModel::AddStructureArtifact(const StructureArtifactData& data,
                                                          const QModelIndex& siblingIndex) {
    vtkSmartPointer<StructureArtifact> artifact = dynamic_cast<StructureArtifact*>(Artifact::NewArtifact(data.SubType));
    StructureArtifactData::SetData(*artifact, data);

    int insertionIndex = siblingIndex.isValid()
            ? siblingIndex.row() + 1
            : StructureWrapper.GetNumberOfArtifacts();

    beginInsertRows({}, insertionIndex, insertionIndex);
    StructureWrapper.AddStructureArtifact(*artifact, insertionIndex);
    endInsertRows();

    return index(insertionIndex, 0, {});
}

void StructureArtifactsModel::RemoveStructureArtifact(const QModelIndex& index) {
    if (!index.isValid()) {
        qWarning("Cannot remove image artifact. Given index is not valid");
        return;
    }

    auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());

    beginRemoveRows({}, index.row(), index.row());
    StructureWrapper.RemoveStructureArtifact(*artifact);
    endRemoveRows();
}

QModelIndex StructureArtifactsModel::MoveUp(const QModelIndex& index) {
    if (!index.isValid() || index.row() == 0) {
        qWarning("Cannot move image artifact up. Given index is not valid or it is first child");
        return {};
    }

    return Move(index, -1);
}

QModelIndex StructureArtifactsModel::MoveDown(const QModelIndex& index) {
    if (!index.isValid() || index.row() == rowCount({}) - 1) {
        qWarning("Cannot move image artifact up. Given index is not valid or it is last child");
        return {};
    }

    return Move(index, 1);
}

QModelIndex StructureArtifactsModel::Move(const QModelIndex& sourceIndex, int displacement) {
    auto* structureArtifact = static_cast<StructureArtifact*>(sourceIndex.internalPointer());

    int prevIdx = sourceIndex.row();
    int newIdx = sourceIndex.row() + displacement;

    beginMoveRows({}, prevIdx, prevIdx,
                  {}, newIdx > prevIdx ? newIdx + 1 : newIdx);
    StructureWrapper.MoveStructureArtifact(structureArtifact, newIdx);
    endMoveRows();

    return index(newIdx, 0, {});
}
