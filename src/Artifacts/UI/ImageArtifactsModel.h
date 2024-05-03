#pragma once

#include <QAbstractItemModel>

class ImageArtifactData;
class ImageArtifactConcatenation;
class Pipeline;

class ImageArtifactsModel : public QAbstractItemModel {
public:
    explicit ImageArtifactsModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                 QObject* parent = nullptr);

    auto
    index(int row, int column, const QModelIndex& parent) const -> QModelIndex override;

    auto
    parent(const QModelIndex& childModelIndex) const -> QModelIndex override;

    auto
    rowCount(const QModelIndex& parentModelIndex) const -> int override;

    auto
    columnCount(const QModelIndex& parent) const -> int override;

    auto
    data(const QModelIndex& index, int role) const -> QVariant override;

    auto
    headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;

    auto
    flags(const QModelIndex& index) const -> Qt::ItemFlags override;

    auto
    setData(const QModelIndex &index, const QVariant &value, int role) -> bool override;

    auto
    AddSiblingImageArtifact(const ImageArtifactData& data, const QModelIndex& siblingIndex) -> QModelIndex;

    auto
    AddChildImageArtifact(const ImageArtifactData& data, const QModelIndex& parentIndex) -> QModelIndex;

    void
    RemoveImageArtifact(const QModelIndex& index);

    auto
    MoveUp(const QModelIndex& index) -> QModelIndex;

    auto
    MoveDown(const QModelIndex& index) -> QModelIndex;

private:
    auto
    AddImageArtifact(const ImageArtifactData& data, const QModelIndex& parentIndex, int insertionIndex) -> QModelIndex;

    auto
    Move(const QModelIndex& sourceIndex, int displacement) -> QModelIndex;

    ImageArtifactConcatenation& Concatenation;
};
