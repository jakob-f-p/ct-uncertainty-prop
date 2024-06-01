#include "ImageArtifactsModel.h"

#include "../../../Artifacts/Image/ImageArtifact.h"
#include "../../../Artifacts/Image/ImageArtifactConcatenation.h"
#include "../../../Utils/Types.h"

ImageArtifactsModel::ImageArtifactsModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                         QObject* parent) :
        QAbstractItemModel(parent),
        Concatenation(imageArtifactConcatenation) {}

auto ImageArtifactsModel::index(int row, int column, const QModelIndex& parentModelIndex) const -> QModelIndex {
    if (!hasIndex(row, column, parentModelIndex))
        return {};

    ImageArtifact const& parent = parentModelIndex.isValid()
            ? Concatenation.Get(static_cast<uidx_t>(parentModelIndex.internalId()))
            : Concatenation.GetStart();

    ImageArtifact const& child = parent.ToCompositeConst().ChildArtifact(row);

    uint16_t const childTreeIdx = Concatenation.IndexOf(child);
    uint16_t const parentChildIdx = parent.ToCompositeConst().GetChildIdx(child);

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
        return {};

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
        case Qt::DisplayRole:
            return QString::fromStdString(
                    Concatenation.Get(static_cast<uidx_t>(index.internalId())).GetViewName());

        case ImageArtifactsModel::DATA: // Qt::UserRole
            return QVariant::fromValue(ImageArtifactData(
                    Concatenation.Get(static_cast<uidx_t>(index.internalId()))));

        case ImageArtifactsModel::POINTER:
            return QVariant::fromValue(&Concatenation.Get(static_cast<uidx_t>(index.internalId())));

        default: return {};
    }
}

auto ImageArtifactsModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    return "Image Artifacts";
}

auto ImageArtifactsModel::flags(const QModelIndex& index) const -> Qt::ItemFlags {
    if (!index.isValid())
        return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

auto ImageArtifactsModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool {
    if (!index.isValid()) {
        return false;
    }

    ImageArtifact& artifact = Concatenation.Get(static_cast<uidx_t>(index.internalId()));
    auto data = value.value<ImageArtifactData>();
    data.PopulateArtifact(artifact);

    Q_EMIT dataChanged(index, index);
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
    auto& imageArtifact = Concatenation.Get(static_cast<uidx_t>(index.internalId()));

    beginRemoveRows(parentIndex, index.row(), index.row());
    Concatenation.RemoveImageArtifact(imageArtifact);
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

auto ImageArtifactsModel::AddImageArtifact(const ImageArtifactData& data,
                                           const QModelIndex& parentIndex,
                                           int insertionIndex) -> QModelIndex {
    auto* parentArtifact = parentIndex.isValid()
                           ? &Concatenation.Get(static_cast<uidx_t>(parentIndex.internalId()))
                           : nullptr;

    ImageArtifact newImageArtifact(data);

    beginInsertRows(parentIndex, insertionIndex, insertionIndex);
    Concatenation.AddImageArtifact(std::move(newImageArtifact), parentArtifact, insertionIndex);
    endInsertRows();

    int const childIndex = insertionIndex == -1 ? rowCount(parentIndex) - 1 : insertionIndex;
    return index(childIndex, 0, parentIndex);
}

auto ImageArtifactsModel::Move(const QModelIndex& sourceIndex, int displacement) -> QModelIndex {
    QModelIndex const parentIndex = sourceIndex.parent();
    auto& imageArtifact = Concatenation.Get(static_cast<uidx_t>(sourceIndex.internalId()));

    int const prevIdx = sourceIndex.row();
    int const newIdx = sourceIndex.row() + displacement;

    beginMoveRows(parentIndex, prevIdx, prevIdx,
                  parentIndex, newIdx > prevIdx ? newIdx + 1 : newIdx);
    Concatenation.MoveChildImageArtifact(imageArtifact, newIdx);
    endMoveRows();

    return index(newIdx, 0, sourceIndex.parent());
}


ImageArtifactsReadOnlyModel::ImageArtifactsReadOnlyModel(ImageArtifactConcatenation const& imageArtifactConcatenation,
                                                         QObject* parent) :
        ImageArtifactsModel(const_cast<ImageArtifactConcatenation&>(imageArtifactConcatenation), parent) {}

auto ImageArtifactsReadOnlyModel::flags(QModelIndex const& index) const -> Qt::ItemFlags {
    return QAbstractItemModel::flags(index);
}
