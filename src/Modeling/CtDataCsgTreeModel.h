#pragma once

#include <QAbstractItemModel>
#include "CtDataCsgTree.h"

class CtDataCsgTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(CtDataCsgTreeModel)

    explicit CtDataCsgTreeModel(const CtDataCsgTree& csgTree, QObject* parent = nullptr);
    ~CtDataCsgTreeModel() override = default;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    const CtDataCsgTree& Tree;
};
