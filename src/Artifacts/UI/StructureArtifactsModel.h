#pragma once

#include <QAbstractItemModel>

class StructureArtifacts;
class StructureArtifactData;

class StructureArtifactsModel : public QAbstractItemModel {
public:
    explicit StructureArtifactsModel(StructureArtifacts& structureWrapper,
                                     QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex AddStructureArtifact(const StructureArtifactData& data, const QModelIndex& siblingIndex);

    void RemoveStructureArtifact(const QModelIndex& index);

    QModelIndex MoveUp(const QModelIndex& index);

    QModelIndex MoveDown(const QModelIndex& index);

protected:
    QModelIndex Move(const QModelIndex& sourceIndex, int displacement);

    StructureArtifacts& StructureWrapper;
};
