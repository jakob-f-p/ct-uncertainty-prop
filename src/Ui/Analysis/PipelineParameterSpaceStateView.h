#pragma once

#include <QTreeView>

class PipelineParameterSpace;
class PipelineParameterSpaceState;
class PipelineParameterSpaceStateModel;


class PipelineParameterSpaceStateView : public QTreeView {
public:
    explicit PipelineParameterSpaceStateView(PipelineParameterSpaceState const& parameterSpaceState);
};


class PipelineParameterSpaceStateModel : public QAbstractItemModel {
public:
    explicit PipelineParameterSpaceStateModel(PipelineParameterSpaceState const& parameterSpaceState,
                                              QObject* parent = nullptr);

    [[nodiscard]] auto
    index(int row, int column, const QModelIndex& parent) const -> QModelIndex override;

    [[nodiscard]] auto
    parent(const QModelIndex& child) const -> QModelIndex override;

    [[nodiscard]] auto
    rowCount(const QModelIndex& parent) const -> int override;

    [[nodiscard]] auto
    columnCount(const QModelIndex& parent) const -> int override;

    [[nodiscard]] auto
    flags(QModelIndex const& index) const -> Qt::ItemFlags override;

    [[nodiscard]] auto
    data(const QModelIndex& index, int role) const -> QVariant override;

    [[nodiscard]] auto
    headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;

private:

    [[nodiscard]] static auto
    IsParameterSpan(const QModelIndex& index) noexcept -> bool;

    PipelineParameterSpaceState const& ParameterSpaceState;
    PipelineParameterSpace const& ParameterSpace;

    struct NumberFormat {
        uint16_t Width;
        uint16_t Precision;
    } const Format;
};
