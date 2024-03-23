#include "ImageArtifactConcatenationModel.h"

ImageArtifactConcatenationModel::ImageArtifactConcatenationModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                                                 QObject* parent) :
        QAbstractItemModel(parent),
        Concatenation(imageArtifactConcatenation) {
    Concatenation.Register(nullptr);
}

ImageArtifactConcatenationModel::~ImageArtifactConcatenationModel() {
    Concatenation.Delete();
}

QModelIndex ImageArtifactConcatenationModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return {};

    auto* childArtifact = parent.isValid()
                            ? static_cast<ImageArtifactComposition*>(parent.internalPointer())->ChildArtifact(row)
                            : &Concatenation.GetStart();

    if (!childArtifact) return {};

    return createIndex(row, column, childArtifact);
}

QModelIndex ImageArtifactConcatenationModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return {};

    auto* childArtifact = static_cast<ImageArtifact*>(child.internalPointer());

    ImageArtifactComposition* parentArtifact = childArtifact->GetParent();
    if (!parentArtifact) return {};

    ImageArtifactComposition* grandparentArtifact = parentArtifact->GetParent();
    int rowIdx = grandparentArtifact
                    ? grandparentArtifact->GetChildIdx(*parentArtifact)
                    : 0;
    return createIndex(rowIdx, 0, parentArtifact);
}

int ImageArtifactConcatenationModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return 1;

    auto* parentArtifact = static_cast<ImageArtifact*>(parent.internalPointer());
    return parentArtifact->GetArtifactSubType() == Artifact::IMAGE_COMPOSITION
            ? dynamic_cast<ImageArtifactComposition*>(parentArtifact)->NumberOfChildren()
            : 0;
}

int ImageArtifactConcatenationModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QVariant ImageArtifactConcatenationModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) return {};

    auto* artifact = static_cast<ImageArtifact*>(index.internalPointer());

    return artifact->Data();
}

QVariant ImageArtifactConcatenationModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (section != 0 || orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

    return "Image Artifacts";
}

Qt::ItemFlags ImageArtifactConcatenationModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return {};

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

