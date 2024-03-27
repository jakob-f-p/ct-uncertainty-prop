#pragma once

#include <QAbstractItemModel>

class ImageArtifactConcatenation;
class Pipeline;
struct ImageArtifactDetails;

class ImageArtifactsModel : public QAbstractItemModel {
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(ImageArtifactsModel)

    explicit ImageArtifactsModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                 QObject* parent = nullptr);

    explicit ImageArtifactsModel(Pipeline& pipeline,
                                 QObject* parent = nullptr);

    ~ImageArtifactsModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex AddSiblingImageArtifact(const ImageArtifactDetails& details, const QModelIndex& siblingIndex);

    QModelIndex AddChildImageArtifact(const ImageArtifactDetails& details, const QModelIndex& parentIndex);

    void RemoveImageArtifact(const QModelIndex& index);

    QModelIndex MoveUp(const QModelIndex& index);

    QModelIndex MoveDown(const QModelIndex& index);

protected:
    QModelIndex AddImageArtifact(const ImageArtifactDetails& details,
                                 const QModelIndex& parentIndex,
                                 int insertionIndex = -1);

    QModelIndex Move(const QModelIndex& sourceIndex, int displacement);

    ImageArtifactConcatenation& Concatenation;
};
