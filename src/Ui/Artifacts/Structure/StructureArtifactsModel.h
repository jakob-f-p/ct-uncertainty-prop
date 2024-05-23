#pragma once

#include <QAbstractItemModel>

class StructureArtifactList;
class StructureArtifactData;

class StructureArtifactsModel : public QAbstractItemModel {
public:
    explicit StructureArtifactsModel(StructureArtifactList& artifactList, QObject* parent = nullptr);

    auto index(int row, int column, const QModelIndex& parent) const -> QModelIndex override;

    auto parent(const QModelIndex& child) const -> QModelIndex override;

    auto rowCount(const QModelIndex& parent) const -> int override;

    auto columnCount(const QModelIndex& parent) const -> int override;

    enum Roles : uint16_t {
        DATA = Qt::UserRole,
        POINTER = 1 + Qt::UserRole
    };

    auto data(const QModelIndex& index, int role) const -> QVariant override;

    auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;

    auto setData(const QModelIndex &index, const QVariant &value, int role) -> bool override;

    auto AddStructureArtifact(const StructureArtifactData& data, const QModelIndex& siblingIndex) -> QModelIndex;

    void RemoveStructureArtifact(const QModelIndex& index);

    auto MoveUp(const QModelIndex& index) -> QModelIndex;

    auto MoveDown(const QModelIndex& index) -> QModelIndex;

protected:
    auto Move(const QModelIndex& sourceIndex, int displacement) -> QModelIndex;

    StructureArtifactList& StructureWrapper;
};


class StructureArtifactsReadOnlyModel : public StructureArtifactsModel {
public:
    explicit StructureArtifactsReadOnlyModel(StructureArtifactList const& artifactList, QObject* parent = nullptr);

    [[nodiscard]] auto
    flags(QModelIndex const& index) const -> Qt::ItemFlags override;
};
