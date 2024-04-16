#pragma once

#include "../CtStructureTree.h"

#include <QAbstractItemModel>

class Pipeline;

enum TreeModelRoles : uint16_t {
    STRUCTURE_DATA_VARIANT = Qt::UserRole,
    IS_BASIC_STRUCTURE
};

class CtStructureTreeModel : public QAbstractItemModel {
public:
    explicit CtStructureTreeModel(CtStructureTree& ctStructureTree, QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex AddBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                  const QModelIndex& siblingIndex);

    void CombineWithBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                   const CombinedStructureData& combinedStructureData);

    void RefineWithBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                  const CombinedStructureData& combinedStructureData,
                                  const QModelIndex& index);

    void RemoveBasicStructure(const QModelIndex& index);

    bool HasRoot();

protected:
    friend class StructureArtifactsWidget;

    CtStructureTree& Tree;
};
