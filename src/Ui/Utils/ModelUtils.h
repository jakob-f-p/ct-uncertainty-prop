#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

[[nodiscard]] auto static
Search(QAbstractItemModel& model, int role, QVariant const& target, QModelIndex const& index) noexcept -> QModelIndex {

    if (index.isValid() && index.data(role) == target)
        return index;

    auto rows = model.rowCount(index);
    for (int i = 0; i < rows; ++i) {
        auto result = Search(model, role, target, model.index(i, 0, index));
        if (result != QModelIndex{})
            return result;
    }

    return {};
}
