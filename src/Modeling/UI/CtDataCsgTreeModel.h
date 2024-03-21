#pragma once

#include "../CtDataCsgTree.h"

#include <QAbstractItemModel>

class CtDataCsgTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(CtDataCsgTreeModel)

    explicit CtDataCsgTreeModel(CtDataCsgTree& csgTree, QObject* parent = nullptr);
    ~CtDataCsgTreeModel() override = default;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex AddImplicitCtStructure(const ImplicitCtStructureDetails& implicitCtStructureDetails,
                                       const QModelIndex& siblingIndex);

    void CombineWithImplicitCtStructure(ImplicitCtStructureDetails& implicitCtStructureDetails);

    void RefineWithImplicitStructure(const ImplicitCtStructureDetails& implicitCtStructureDetails,
                                     const QModelIndex& index);

    void RemoveImplicitCtStructure(const QModelIndex& implicitCtStructureIndex);

    bool HasRoot();
protected:
    CtDataCsgTree& Tree;
};
