#pragma once

#include <QAbstractItemModel>
#include <QVariant>

[[nodiscard]] auto static
Search(QAbstractItemModel& model, int role, QVariant const& target, QModelIndex const& index) noexcept -> QModelIndex {

    if (index.isValid() && index.data(role) == target)
        return index;

    auto const rows = model.rowCount(index);
    for (int i = 0; i < rows; ++i) {
        if (auto result = Search(model, role, target, model.index(i, 0, index));
            result != QModelIndex{})
            return result;
    }

    return {};
}
