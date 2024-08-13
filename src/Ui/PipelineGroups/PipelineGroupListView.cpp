#include "PipelineGroupListView.h"

#include "../../PipelineGroups/PipelineGroupList.h"

#include <QVBoxLayout>


PipelineGroupListView::PipelineGroupListView(PipelineGroupList& pipelineGroups) :
        GroupListModel(new PipelineGroupListModel(pipelineGroups, this)) {
    setModel(GroupListModel);
}

auto PipelineGroupListView::model() const noexcept -> PipelineGroupListModel* {
    return GroupListModel;
}

auto PipelineGroupListView::showEvent(QShowEvent* event) -> void {
    GroupListModel->Reset();

    QWidget::showEvent(event);
}


PipelineGroupListModel::PipelineGroupListModel(PipelineGroupList& pipelineGroups, QObject* parent) :
        QAbstractItemModel(parent),
        PipelineGroups(pipelineGroups) {
}

auto PipelineGroupListModel::index(int row, int column, QModelIndex const& parent) const -> QModelIndex {
    if (!hasIndex(row, column, parent))
        return {};

    return createIndex(row, column);
}

auto PipelineGroupListModel::parent(QModelIndex const& /*child*/) const -> QModelIndex {
    return {};
}

auto PipelineGroupListModel::rowCount(QModelIndex const& parent) const -> int {
    if (parent.isValid())
        return 0;

    return PipelineGroups.GetSize();
}

auto PipelineGroupListModel::columnCount(QModelIndex const& /*parent*/) const -> int {
    return 1;
}

auto PipelineGroupListModel::data(QModelIndex const& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    auto& pipelineGroup = PipelineGroups.Get(index.row());

    switch (role) {
        case Qt::DisplayRole: return QString::fromStdString(pipelineGroup.GetName());

        case Qt::UserRole: return QVariant::fromValue(&pipelineGroup);

        default: return {};
    }
}

auto PipelineGroupListModel::AddPipelineGroup(Pipeline const& basePipeline, std::string name) -> QModelIndex {
    int const insertionIndex = PipelineGroups.GetSize();

    beginInsertRows({}, insertionIndex, insertionIndex);
    PipelineGroups.AddPipelineGroup(basePipeline, std::move(name));
    endInsertRows();

    return index(insertionIndex, 0, {});
}

auto PipelineGroupListModel::RemovePipelineGroup(QModelIndex const& index) -> void {
    if (!index.isValid())
        throw std::runtime_error("Cannot remove image artifact. Given index is not valid");

    auto* pipelineGroup = index.data(Qt::UserRole).value<PipelineGroup*>();
    if (!pipelineGroup)
        throw std::runtime_error("Pipeline group must not be null");

    beginRemoveRows({}, index.row(), index.row());
    PipelineGroups.RemovePipelineGroup(*pipelineGroup);
    endRemoveRows();
}

auto PipelineGroupListModel::Reset() -> void {
    beginResetModel();
    endResetModel();
}

