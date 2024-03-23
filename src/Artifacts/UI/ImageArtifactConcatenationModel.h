#pragma once

#include "../ImageArtifactConcatenation.h"

#include <QAbstractItemModel>

class ImageArtifactConcatenationModel : public QAbstractItemModel {
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(ImageArtifactConcatenationModel)

    explicit ImageArtifactConcatenationModel(ImageArtifactConcatenation& imageArtifactConcatenation,
                                             QObject* parent = nullptr);
    ~ImageArtifactConcatenationModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

//    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
    ImageArtifactConcatenation& Concatenation;
};
