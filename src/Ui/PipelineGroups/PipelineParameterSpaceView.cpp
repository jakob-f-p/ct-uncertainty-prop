#include "PipelineParameterSpaceView.h"

#include "../../PipelineGroups/PipelineParameterSpace.h"

#include <QVBoxLayout>


PipelineParameterSpaceView::PipelineParameterSpaceView(PipelineParameterSpace& parameterSpace) :
        ParameterSpaceModel(new PipelineParameterSpaceModel(parameterSpace, this)) {
    QTreeView::setModel(ParameterSpaceModel);

    setHeaderHidden(true);
}

auto PipelineParameterSpaceView::model() const noexcept -> PipelineParameterSpaceModel* {
    return ParameterSpaceModel;
}


PipelineParameterSpaceModel::PipelineParameterSpaceModel(PipelineParameterSpace& parameterSpace, QObject* parent) :
        QAbstractItemModel(parent),
        ParameterSpace(parameterSpace) {}

auto PipelineParameterSpaceModel::index(int row, int column, QModelIndex const& parent) const -> QModelIndex {
    if (!hasIndex(row, column, parent))
        return {};

    return createIndex(row, column, parent.row());
}

auto PipelineParameterSpaceModel::parent(QModelIndex const& child) const -> QModelIndex {
    if (!child.isValid())
        return {};

    if (static_cast<int>(child.internalId()) == -1)
        return {};

    uint16_t const parentIdx = static_cast<int>(child.internalId());

    return createIndex(parentIdx, 0, -1);
}

auto PipelineParameterSpaceModel::rowCount(QModelIndex const& parent) const -> int {
    if (parent.isValid() && parent.parent().isValid())
        return 0;

    if (parent.isValid())
        return ParameterSpace.GetSpanSet(parent.row()).GetSize();

    return ParameterSpace.GetNumberOfSpanSets();
}

auto PipelineParameterSpaceModel::columnCount(QModelIndex const& /*parent*/) const -> int {
    return 1;
}

auto PipelineParameterSpaceModel::flags(QModelIndex const& index) const -> Qt::ItemFlags {
    if (!index.isValid() || !index.data(IS_PARAMETER_SPAN).toBool())
        return QAbstractItemModel::flags(index);

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

auto PipelineParameterSpaceModel::data(QModelIndex const& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    if (QModelIndex const parentIndex = index.parent(); parentIndex.isValid()) {
        auto& spanSet = ParameterSpace.GetSpanSet(parentIndex.row());
        auto& parameterSpan = spanSet.Get(index.row());

        switch (role) {
            case Qt::DisplayRole: return QString::fromStdString(parameterSpan.GetName());
            case POINTER: return QVariant::fromValue(&parameterSpan); // Qt::UserRole
            case IS_PARAMETER_SPAN: return true;
            default: return {};
        }
    }

    auto& spanSet = ParameterSpace.GetSpanSet(index.row());

    switch (role) {
        case Qt::DisplayRole: return QString::fromStdString(ParameterSpace.GetSpanSetName(spanSet));
        case POINTER: return QVariant::fromValue(&spanSet); // Qt::UserRole
        case IS_PARAMETER_SPAN: return false;
        default: return {};
    }
}

auto PipelineParameterSpaceModel::AddParameterSpan(PipelineParameterSpan&& parameterSpan) -> QModelIndex {
    auto const artifactPointer = parameterSpan.GetArtifact();
    auto const& spanSet = ParameterSpace.GetSetForArtifactPointer(artifactPointer);

    uidx_t const spanSetRowIdx = ParameterSpace.GetSpanSetIdx(spanSet);
    QModelIndex const parentModelIndex = index(spanSetRowIdx, 0, {});

    uidx_t const insertionIdx = spanSet.GetSize();

    beginResetModel();
    ParameterSpace.AddParameterSpan(artifactPointer, std::move(parameterSpan));
    endResetModel();

    return index(insertionIdx, 0, parentModelIndex);
}

auto PipelineParameterSpaceModel::RemoveParameterSpan(QModelIndex const& index) -> void {
    if (!index.isValid() || !index.parent().isValid())
        throw std::runtime_error("Cannot remove parameter span. Given index is not valid");

    auto const* parameterSpan = index.data(Qt::UserRole).value<PipelineParameterSpan*>();
    if (!parameterSpan)
        throw std::runtime_error("Parameter span must not be null");

    auto& spanSet = ParameterSpace.GetSpanSet(index.parent().row());

    beginResetModel();

    spanSet.RemoveParameterSpan(*parameterSpan);

    if (spanSet.GetSize() == 0) {
        auto const spanSetIt = std::ranges::find(std::as_const(ParameterSpace.ParameterSpanSets),
                                           spanSet);

        if (spanSetIt == ParameterSpace.ParameterSpanSets.cend())
            throw std::runtime_error("Parameter span set not found");

        ParameterSpace.ParameterSpanSets.erase(spanSetIt);
    }

    endResetModel();
}
