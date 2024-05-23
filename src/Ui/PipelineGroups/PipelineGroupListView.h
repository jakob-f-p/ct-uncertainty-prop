#pragma once

#include <QListView>

class Pipeline;
class PipelineGroupList;
class PipelineGroupListModel;


class PipelineGroupListView : public QListView {

public:
    explicit PipelineGroupListView(PipelineGroupList& pipelineGroups);

    [[nodiscard]] auto
    model() const noexcept -> PipelineGroupListModel*;

private:
    PipelineGroupListModel* GroupListModel;
};


class PipelineGroupListModel : public QAbstractItemModel {
public:
    explicit PipelineGroupListModel(PipelineGroupList& pipelineGroups, QObject* parent = nullptr);

    [[nodiscard]] auto
    index(int row, int column, const QModelIndex& parent) const -> QModelIndex override;

    [[nodiscard]] auto
    parent(const QModelIndex& child) const -> QModelIndex override;

    [[nodiscard]] auto
    rowCount(const QModelIndex& parent) const -> int override;

    [[nodiscard]] auto
    columnCount(const QModelIndex& parent) const -> int override;

    [[nodiscard]] auto
    data(const QModelIndex& index, int role) const -> QVariant override;

    [[nodiscard]] auto
    AddPipelineGroup(Pipeline const& basePipeline, std::string name = "") -> QModelIndex;

    auto
    RemovePipelineGroup(const QModelIndex& index) -> void;

private:
    PipelineGroupList& PipelineGroups;
};
