#pragma once

#include <QAbstractItemModel>

class StructureArtifactList;
class StructureArtifactData;

class StructureArtifactsModel : public QAbstractItemModel {
public:
    explicit StructureArtifactsModel(StructureArtifactList& structureWrapper,
                                     QObject* parent = nullptr);

    auto index(int row, int column, const QModelIndex& parent) const -> QModelIndex override;

    auto parent(const QModelIndex& child) const -> QModelIndex override;

    auto rowCount(const QModelIndex& parent) const -> int override;

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

    StructureArtifactList& StructureWrapper;
};
