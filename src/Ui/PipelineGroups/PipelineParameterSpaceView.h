#pragma once

#include <QTreeView>

class PipelineParameterSpace;
class PipelineParameterSpan;
class PipelineParameterSpanSet;
class PipelineParameterSpaceModel;


class PipelineParameterSpaceView : public QTreeView {

public:
    explicit PipelineParameterSpaceView(PipelineParameterSpace& parameterSpace);

    [[nodiscard]] auto
    model() const noexcept -> PipelineParameterSpaceModel*;

private:
    PipelineParameterSpaceModel* ParameterSpaceModel;
};


class PipelineParameterSpaceModel : public QAbstractItemModel {
public:
    explicit PipelineParameterSpaceModel(PipelineParameterSpace& parameterSpace, QObject* parent = nullptr);

    enum Roles : uint16_t {
        POINTER = Qt::UserRole,
        IS_PARAMETER_SPAN = 1 + Qt::UserRole
    };

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
    AddParameterSpan(PipelineParameterSpan&& parameterSpan) -> QModelIndex;

    auto
    RemoveParameterSpan(QModelIndex const& index) -> void;

private:
    PipelineParameterSpace& ParameterSpace;
};
