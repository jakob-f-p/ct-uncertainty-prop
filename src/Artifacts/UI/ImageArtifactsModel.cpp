#include "ImageArtifactsModel.h"

#include "../ImageArtifact.h"
#include "../ImageArtifactConcatenation.h"

ImageArtifactsModel::ImageArtifactsModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                         QObject* parent) :
        QAbstractItemModel(parent),
        Concatenation(imageArtifactConcatenation) {
}

auto ImageArtifactsModel::index(int row, int column, const QModelIndex& parentModelIndex) const -> QModelIndex {
    if (!hasIndex(row, column, parentModelIndex))
        return {};

    idx_t childTreeIdx;
    idx_t parentChildIdx;
    if (parentModelIndex.isValid()) {
        ImageArtifact const& parent = Concatenation.Get(static_cast<uidx_t>(parentModelIndex.internalId()));
        ImageArtifact const& child = parent.ToCompositeConst().ChildArtifact(row);

        childTreeIdx = Concatenation.IndexOf(child);
        parentChildIdx = parent.ToCompositeConst().GetChildIdx(child);
    } else {
        ImageArtifact const& artifact = Concatenation.GetStart().ChildArtifact(row);
        childTreeIdx = Concatenation.IndexOf(artifact);
        parentChildIdx = Concatenation.GetStart().GetChildIdx(artifact);
    }

    return createIndex(parentChildIdx, 0, childTreeIdx);
}

auto ImageArtifactsModel::parent(const QModelIndex& childModelIndex) const -> QModelIndex {
    if (!childModelIndex.isValid())
        return {};

    ImageArtifact const& childArtifact = Concatenation.Get(static_cast<uidx_t>(childModelIndex.internalId()));

    ImageArtifact* parentArtifact = childArtifact.GetParent();
    if (!parentArtifact)
        return {};

    uidx_t const parentIdx = Concatenation.IndexOf(*parentArtifact);

    ImageArtifact* grandparentArtifact = parentArtifact->GetParent();
    if (!grandparentArtifact)
        return createIndex(Concatenation.GetStart().GetChildIdx(*parentArtifact), 0, parentIdx);

    uidx_t const rowIdx = grandparentArtifact->ToComposite().GetChildIdx(*parentArtifact);
    return createIndex(rowIdx, 0, parentIdx);
}

auto ImageArtifactsModel::rowCount(const QModelIndex& parentModelIndex) const -> int {
    if (!parentModelIndex.isValid())
        return Concatenation.GetStart().NumberOfChildren();

    ImageArtifact const& parentArtifact = Concatenation.Get(static_cast<uidx_t>(parentModelIndex.internalId()));

    return parentArtifact.NumberOfChildren();
}

auto ImageArtifactsModel::columnCount(const QModelIndex& /*parent*/) const -> int {
    return 1;
}

auto ImageArtifactsModel::data(const QModelIndex& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    switch (role) {
        case Qt::DisplayRole: {
            ImageArtifact const& artifact = Concatenation.Get(static_cast<uidx_t>(index.internalId()));

            return QString::fromStdString(artifact.GetViewName());
        }

        case Qt::UserRole: {
            ImageArtifact const& artifact = Concatenation.Get(static_cast<uidx_t>(index.internalId()));

            return QVariant::fromValue(ImageArtifactData(artifact));
        }
    }

    return {};
}

auto ImageArtifactsModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

    return "Image Artifacts";
}

Qt::ItemFlags ImageArtifactsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

auto ImageArtifactsModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool {
    if (!index.isValid()) {
        return false;
    }

    ImageArtifact& artifact = Concatenation.Get(static_cast<uidx_t>(index.internalId()));
    auto data = value.value<ImageArtifactData>();
    data.PopulateArtifact(artifact);

    emit dataChanged(index, index);
    return true;
}

auto ImageArtifactsModel::AddSiblingImageArtifact(const ImageArtifactData& data,
                                                  const QModelIndex& siblingIndex) -> QModelIndex {
    if (!siblingIndex.isValid())
        throw std::runtime_error("Cannot add sibling image artifact. Given index is not valid");

    return AddImageArtifact(data, siblingIndex.parent(), siblingIndex.row() + 1);
}

auto ImageArtifactsModel::AddChildImageArtifact(const ImageArtifactData& data,
                                                const QModelIndex& parentIndex) -> QModelIndex {
    return AddImageArtifact(data, parentIndex, rowCount(parentIndex));
}

void ImageArtifactsModel::RemoveImageArtifact(const QModelIndex& index) {
    if (!index.isValid())
        throw std::runtime_error("Cannot remove image artifact. Given index is not valid");

    QModelIndex const parentIndex = index.parent();
    auto& parentArtifact = parentIndex.isValid()
                                ? Concatenation.Get(static_cast<uidx_t>(parentIndex.internalId())).ToComposite()
                                : Concatenation.GetStart();
    auto& imageArtifact = Concatenation.Get(static_cast<uidx_t>(index.internalId()));

    beginRemoveRows(parentIndex, index.row(), index.row());
    parentArtifact.RemoveImageArtifact(imageArtifact);
    endRemoveRows();
}

auto ImageArtifactsModel::MoveUp(const QModelIndex& index) -> QModelIndex {
    if (!index.isValid() || index.row() == 0)
        throw std::runtime_error("Cannot move image artifact up. Given index is not valid or it is first child");

    return Move(index, -1);
}

auto ImageArtifactsModel::MoveDown(const QModelIndex& index) -> QModelIndex {
    if (!index.isValid() || index.row() == rowCount(index.parent()) - 1)
        throw std::runtime_error("Cannot move image artifact up. Given index is not valid or it is last child");

    return Move(index, 1);
}

QModelIndex ImageArtifactsModel::AddImageArtifact(const ImageArtifactData& data,
                                                  const QModelIndex& parentIndex,
                                                  int insertionIndex) {
    auto* parentArtifact = parentIndex.isValid()
                           ? &Concatenation.Get(static_cast<uidx_t>(parentIndex.internalId()))
                           : nullptr;

    ImageArtifact newImageArtifact(data);

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);
    Concatenation.AddImageArtifact(std::move(newImageArtifact), parentArtifact, insertionIndex);
    endInsertRows();

    int childIndex = insertionIndex == -1 ? rowCount(parentIndex) - 1 : insertionIndex;
    return index(childIndex, 0, parentIndex);
}

auto ImageArtifactsModel::Move(const QModelIndex& sourceIndex, int displacement) -> QModelIndex {
    QModelIndex parentIndex = sourceIndex.parent();
    auto& parentArtifact = parentIndex.isValid()
                           ? Concatenation.Get(static_cast<uidx_t>(parentIndex.internalId())).ToComposite()
                           : Concatenation.GetStart();

    auto& imageArtifact = Concatenation.Get(static_cast<uidx_t>(sourceIndex.internalId()));

    int prevIdx = sourceIndex.row();
    int newIdx = sourceIndex.row() + displacement;

    beginMoveRows(parentIndex, prevIdx, prevIdx,
                  parentIndex, newIdx > prevIdx ? newIdx + 1 : newIdx);
    parentArtifact.MoveChildImageArtifact(imageArtifact, newIdx);
    endMoveRows();

    return index(newIdx, 0, sourceIndex.parent());
}
