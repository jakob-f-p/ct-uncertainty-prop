#include "ImageArtifactsModel.h"

#include "../ImageArtifactDetails.h"
#include "../ImageArtifactConcatenation.h"
#include "../Pipeline.h"

ImageArtifactsModel::ImageArtifactsModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                         QObject* parent) :
        QAbstractItemModel(parent),
        Concatenation(imageArtifactConcatenation) {
    Concatenation.Register(nullptr);
}

ImageArtifactsModel::ImageArtifactsModel(Pipeline& pipeline, QObject* parent) :
        QAbstractItemModel(parent),
        Concatenation(pipeline.GetImageArtifactConcatenation()) {
    Concatenation.Register(nullptr);
}

ImageArtifactsModel::~ImageArtifactsModel() {
    Concatenation.Delete();
}

QModelIndex ImageArtifactsModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return {};

    auto* childArtifact = parent.isValid()
                            ? static_cast<ImageArtifactComposition*>(parent.internalPointer())->ChildArtifact(row)
                            : Concatenation.GetStart().ChildArtifact(row);

    return createIndex(row, column, childArtifact);
}

QModelIndex ImageArtifactsModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return {};

    auto* childArtifact = static_cast<ImageArtifact*>(child.internalPointer());

    ImageArtifactComposition* parentArtifact = childArtifact->GetParent();
    if (!parentArtifact) return {};

    ImageArtifactComposition* grandparentArtifact = parentArtifact->GetParent();
    if (!grandparentArtifact) return {};

    int rowIdx = grandparentArtifact->GetChildIdx(*parentArtifact);
    return createIndex(rowIdx, 0, parentArtifact);
}

int ImageArtifactsModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return Concatenation.GetStart().NumberOfChildren();

    auto* parentArtifact = static_cast<ImageArtifact*>(parent.internalPointer());
    return parentArtifact->GetArtifactSubType() == Artifact::IMAGE_COMPOSITION
            ? dynamic_cast<ImageArtifactComposition*>(parentArtifact)->NumberOfChildren()
            : 0;
}

int ImageArtifactsModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QVariant ImageArtifactsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) return {};

    auto* artifact = static_cast<ImageArtifact*>(index.internalPointer());

    return artifact->Data();
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
    auto imageArtifactDetails = value.value<ImageArtifactDetails>();
    imageArtifact->SetData(imageArtifactDetails);

    emit dataChanged(index, index);
    return true;
}

QModelIndex ImageArtifactsModel::AddSiblingImageArtifact(const ImageArtifactDetails& details,
                                                         const QModelIndex& siblingIndex) {
    if (!siblingIndex.isValid()) {
        qWarning("Cannot add sibling image artifact. Given index is not valid");
        return {};
    }

    return AddImageArtifact(details, siblingIndex.parent(), siblingIndex.row() + 1);
}

QModelIndex ImageArtifactsModel::AddChildImageArtifact(const ImageArtifactDetails& details,
                                                       const QModelIndex& parentIndex) {
    return AddImageArtifact(details, parentIndex);
}

void ImageArtifactsModel::RemoveImageArtifact(const QModelIndex& index) {
    if (!index.isValid()) {
        qWarning("Cannot remove image artifact. Given index is not valid");
        return;
    }

    QModelIndex parentIndex = index.parent();
    auto* parentImageArtifactComposition = parentIndex.isValid()
                                           ? static_cast<ImageArtifactComposition*>(parentIndex.internalPointer())
                                           : &Concatenation.GetStart();
    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());

    beginResetModel();
    parentImageArtifactComposition->RemoveImageArtifact(*imageArtifact);
    endResetModel();
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

QModelIndex ImageArtifactsModel::AddImageArtifact(const ImageArtifactDetails& details,
                                                  const QModelIndex& parentIndex,
                                                  int insertionIndex) {
    auto* parentImageArtifactComposition = parentIndex.isValid()
                                           ? static_cast<ImageArtifactComposition*>(parentIndex.internalPointer())
                                           : &Concatenation.GetStart();

    auto* newImageArtifact = dynamic_cast<ImageArtifact*>(Artifact::NewArtifact(details.SubType));
    newImageArtifact->SetData(details);

    beginResetModel();
    parentImageArtifactComposition->AddImageArtifact(*newImageArtifact, insertionIndex);
    endResetModel();

    newImageArtifact->Delete();

    int childIndex = insertionIndex == -1 ? rowCount(parentIndex) - 1 : insertionIndex;
    return index(childIndex, 0, parentIndex);
}

QModelIndex ImageArtifactsModel::Move(const QModelIndex& sourceIndex, int displacement) {
    QModelIndex parentIndex = sourceIndex.parent();
    auto* parentImageArtifactComposition = parentIndex.isValid()
                                           ? static_cast<ImageArtifactComposition*>(parentIndex.internalPointer())
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
