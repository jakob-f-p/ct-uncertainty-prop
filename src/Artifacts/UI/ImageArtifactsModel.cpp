#include "ImageArtifactsModel.h"

#include "../CompositeArtifact.h"
#include "../ImageArtifactConcatenation.h"

ImageArtifactsModel::ImageArtifactsModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                         QObject* parent) :
        QAbstractItemModel(parent),
        Concatenation(imageArtifactConcatenation) {
}

QModelIndex ImageArtifactsModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return {};

    auto* childArtifact = parent.isValid()
                            ? static_cast<CompositeArtifact*>(parent.internalPointer())->ChildArtifact(row)
                            : Concatenation.GetStart().ChildArtifact(row);

    return createIndex(row, column, childArtifact);
}

QModelIndex ImageArtifactsModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return {};

    auto* childArtifact = static_cast<ImageArtifact*>(child.internalPointer());

    CompositeArtifact* parentArtifact = childArtifact->GetParent();
    if (!parentArtifact) return {};

    CompositeArtifact* grandparentArtifact = parentArtifact->GetParent();
    if (!grandparentArtifact) return {};

    int rowIdx = grandparentArtifact->GetChildIdx(*parentArtifact);
    return createIndex(rowIdx, 0, parentArtifact);
}

int ImageArtifactsModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return Concatenation.GetStart().NumberOfChildren();

    auto* parentArtifact = static_cast<ImageArtifact*>(parent.internalPointer());
    return parentArtifact->IsComposition()
            ? dynamic_cast<CompositeArtifact*>(parentArtifact)->NumberOfChildren()
            : 0;
}

int ImageArtifactsModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QVariant ImageArtifactsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto* artifact = static_cast<ImageArtifact*>(index.internalPointer());

    return QString::fromStdString(artifact->GetViewName());
}

QVariant ImageArtifactsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

    return "Image Artifacts";
}

Qt::ItemFlags ImageArtifactsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool ImageArtifactsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) {
        return false;
    }

    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());
    ImageArtifactData::SetData(*imageArtifact, value);

    emit dataChanged(index, index);
    return true;
}

QModelIndex ImageArtifactsModel::AddSiblingImageArtifact(const ImageArtifactData& data,
                                                         const QModelIndex& siblingIndex) {
    if (!siblingIndex.isValid()) {
        qWarning("Cannot add sibling image artifact. Given index is not valid");
        return {};
    }

    return AddImageArtifact(data, siblingIndex.parent(), siblingIndex.row() + 1);
}

QModelIndex ImageArtifactsModel::AddChildImageArtifact(const ImageArtifactData& data,
                                                       const QModelIndex& parentIndex) {
    return AddImageArtifact(data, parentIndex, rowCount(parentIndex));
}

void ImageArtifactsModel::RemoveImageArtifact(const QModelIndex& index) {
    if (!index.isValid()) {
        qWarning("Cannot remove image artifact. Given index is not valid");
        return;
    }

    QModelIndex parentIndex = index.parent();
    auto* parentImageArtifactComposition = parentIndex.isValid()
                                           ? static_cast<CompositeArtifact*>(parentIndex.internalPointer())
                                           : &Concatenation.GetStart();
    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());

    beginRemoveRows(parentIndex, index.row(), index.row());
    parentImageArtifactComposition->RemoveImageArtifact(*imageArtifact);
    endRemoveRows();
}

QModelIndex ImageArtifactsModel::MoveUp(const QModelIndex& index) {
    if (!index.isValid() || index.row() == 0) {
        qWarning("Cannot move image artifact up. Given index is not valid or it is first child");
        return {};
    }

    return Move(index, -1);
}

QModelIndex ImageArtifactsModel::MoveDown(const QModelIndex& index) {
    if (!index.isValid() || index.row() == rowCount(index.parent()) - 1) {
        qWarning("Cannot move image artifact up. Given index is not valid or it is last child");
        return {};
    }

    return Move(index, 1);
}

QModelIndex ImageArtifactsModel::AddImageArtifact(const ImageArtifactData& data,
                                                  const QModelIndex& parentIndex,
                                                  int insertionIndex) {
    auto* parentImageArtifactComposition = parentIndex.isValid()
                                           ? static_cast<CompositeArtifact*>(parentIndex.internalPointer())
                                           : &Concatenation.GetStart();

    auto* newImageArtifact = dynamic_cast<ImageArtifact*>(Artifact::NewArtifact(data.SubType));
    ImageArtifactData::SetData(*newImageArtifact, data);

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);
    parentImageArtifactComposition->AddImageArtifact(*newImageArtifact, insertionIndex);
    endInsertRows();

    newImageArtifact->Delete();

    int childIndex = insertionIndex == -1 ? rowCount(parentIndex) - 1 : insertionIndex;
    return index(childIndex, 0, parentIndex);
}

QModelIndex ImageArtifactsModel::Move(const QModelIndex& sourceIndex, int displacement) {
    QModelIndex parentIndex = sourceIndex.parent();
    auto* parentImageArtifactComposition = parentIndex.isValid()
                                           ? static_cast<CompositeArtifact*>(parentIndex.internalPointer())
                                           : &Concatenation.GetStart();

    auto* imageArtifact = static_cast<ImageArtifact*>(sourceIndex.internalPointer());

    int prevIdx = sourceIndex.row();
    int newIdx = sourceIndex.row() + displacement;

    beginMoveRows(parentIndex, prevIdx, prevIdx,
                  parentIndex, newIdx > prevIdx ? newIdx + 1 : newIdx);
    parentImageArtifactComposition->MoveChildImageArtifact(imageArtifact, newIdx);
    endMoveRows();

    return index(newIdx, 0, sourceIndex.parent());
}
