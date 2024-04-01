#pragma once

#include <QAbstractItemModel>

class CtStructureTree;
class Pipeline;

struct BasicStructureData;
struct CombinedStructureData;

class CtStructureTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(CtStructureTreeModel)

    explicit CtStructureTreeModel(CtStructureTree& ctStructureTree, QObject* parent = nullptr);
    explicit CtStructureTreeModel(Pipeline* pipeline, QObject* parent = nullptr);

    ~CtStructureTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex AddBasicStructure(const BasicStructureData& basicStructureData,
                                  const QModelIndex& siblingIndex);

    void CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                                   const CombinedStructureData& combinedStructureData);

    void RefineWithBasicStructure(const BasicStructureData& basicStructureData,
                                  const CombinedStructureData& combinedStructureData,
                                  const QModelIndex& index);

    void RemoveBasicStructure(const QModelIndex& index);

    bool HasRoot();

protected:
    CtStructureTree& Tree;
};
