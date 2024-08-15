#pragma once

#include <QAbstractItemModel>

class CtStructureTree;
class Pipeline;

struct BasicStructureData;
struct CombinedStructureData;

enum TreeModelRoles : uint16_t {
    STRUCTURE_DATA_VARIANT = Qt::UserRole,
    IS_BASIC_STRUCTURE,
    POINTER_CONST
};

class CtStructureTreeModel : public QAbstractItemModel {
public:
    explicit CtStructureTreeModel(CtStructureTree& ctStructureTree, QObject* parent = nullptr);

    [[nodiscard]] auto
    index(int row, int column, const QModelIndex& parentIndex) const -> QModelIndex override;

    [[nodiscard]] auto
    parent(const QModelIndex& child) const -> QModelIndex override;

    [[nodiscard]] auto
    rowCount(const QModelIndex& parent) const -> int override;

    [[nodiscard]] auto
    columnCount(const QModelIndex& parent) const -> int override;

    [[nodiscard]] auto
    data(const QModelIndex& index, int role) const -> QVariant override;

    [[nodiscard]] auto
    headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;

    [[nodiscard]] auto
    flags(const QModelIndex& index) const -> Qt::ItemFlags override;

    [[nodiscard]] auto
    setData(const QModelIndex &index, const QVariant &value, int role) -> bool override;

    [[nodiscard]] auto
    AddBasicStructure(const BasicStructureData& basicStructureData, const QModelIndex& siblingIndex) -> QModelIndex;

    void CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                                   const CombinedStructureData& combinedStructureData);

    void RefineWithBasicStructure(const BasicStructureData& basicStructureData,
                                  const CombinedStructureData& combinedStructureData,
                                  const QModelIndex& index);

    void RemoveBasicStructure(const QModelIndex& index);

    [[nodiscard]] auto
    HasRoot() -> bool;

protected:
    friend class StructureArtifactsWidget;

    CtStructureTree& Tree;
};


class CtStructureTreeReadOnlyModel : public CtStructureTreeModel {
public:
    explicit CtStructureTreeReadOnlyModel(CtStructureTree const& ctStructureTree, QObject* parent = nullptr);

    [[nodiscard]] auto
    flags(QModelIndex const& index) const -> Qt::ItemFlags override;
};


class CtStructureArtifactsModel : public CtStructureTreeModel {
public:
    explicit CtStructureArtifactsModel(CtStructureTree& ctStructureTree, Pipeline& pipeline, QObject* parent = nullptr);

    [[nodiscard]] auto
    data(const QModelIndex& index, int role) const -> QVariant override;

private:
    Pipeline& Pipeline_;
};

