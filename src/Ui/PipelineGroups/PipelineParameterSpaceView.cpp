#include "PipelineParameterSpaceView.h"

#include "../../PipelineGroups/PipelineParameterSpace.h"

#include <QVBoxLayout>


PipelineParameterSpaceView::PipelineParameterSpaceView(PipelineParameterSpace& pipelineGroups) :
        ParameterSpaceModel(new PipelineParameterSpaceModel(pipelineGroups)) {
    setModel(ParameterSpaceModel);
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

    return createIndex(row, column);
}

auto PipelineParameterSpaceModel::parent(QModelIndex const& child) const -> QModelIndex {
    if (!child.isValid() || !child.parent().isValid())
        return {};

    QModelIndex const parent = child.parent();

    PipelineParameterSpanSet const& spanSet = ParameterSpace.GetSpanSet(child.row());

    uint16_t const parentIdx = ParameterSpace.GetSpanSetIdx(spanSet);

    return createIndex(parentIdx, 0);
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
    if (index.isValid() || !index.data(Roles::IS_PARAMETER_SPAN).toBool())
        return QAbstractItemModel::flags(index);

    return Qt::ItemIsEnabled;
}

auto PipelineParameterSpaceModel::data(QModelIndex const& index, int role) const -> QVariant {
    if (!index.isValid())
        return {};

    QModelIndex const parentIndex = index.parent();

    if (parentIndex.isValid()) {
        auto& spanSet = ParameterSpace.GetSpanSet(parentIndex.row());
        auto& parameterSpan = spanSet.Get(index.row());

        switch (role) {
            case Qt::DisplayRole: return QString::fromStdString(parameterSpan.GetName());
            case Roles::POINTER: return QVariant::fromValue(&parameterSpan); // Qt::UserRole
            case Roles::IS_PARAMETER_SPAN: return true;
            default: return {};
        }
    }

    auto& spanSet = ParameterSpace.GetSpanSet(index.row());

    switch (role) {
        case Qt::DisplayRole: return QString::fromStdString(ParameterSpace.GetSpanSetName(spanSet));
        case Roles::POINTER: return QVariant::fromValue(&spanSet); // Qt::UserRole
        case Roles::IS_PARAMETER_SPAN: return false;
        default: return {};
    }
}

auto PipelineParameterSpaceModel::AddParameterSpan(ArtifactVariantPointer artifactPointer,
                                                   PipelineParameterSpan&& parameterSpan) -> QModelIndex {
    auto const& spanSet = ParameterSpace.GetSetForArtifactPointer(artifactPointer);

    uidx_t const spanSetRowIdx = ParameterSpace.GetSpanSetIdx(spanSet);
    QModelIndex const parentModelIndex = index(spanSetRowIdx, 0, {});

    uidx_t const insertionIdx = spanSet.GetSize();

    beginInsertRows(parentModelIndex, insertionIdx, insertionIdx);
    auto const& span = ParameterSpace.AddParameterSpan(artifactPointer, std::move(parameterSpan));
    endInsertRows();

    return index(insertionIdx, 0, parentModelIndex);
}

auto PipelineParameterSpaceModel::AddParameterSpan(PipelineParameterSpanSet& spanSet,
                                                   PipelineParameterSpan&& parameterSpan) -> QModelIndex {
    uidx_t const spanSetRowIdx = ParameterSpace.GetSpanSetIdx(spanSet);
    QModelIndex const parentModelIndex = index(spanSetRowIdx, 0, {});

    uidx_t const insertionIdx = spanSet.GetSize();

    beginInsertRows(parentModelIndex, insertionIdx, insertionIdx);
    auto const& span = ParameterSpace.AddParameterSpan(spanSet, std::move(parameterSpan));
    endInsertRows();

    return index(insertionIdx, 0, parentModelIndex);
}

auto PipelineParameterSpaceModel::RemoveParameterSpan(QModelIndex const& index) -> void {
    if (!index.isValid() || !index.parent().isValid())
        throw std::runtime_error("Cannot remove parameter span. Given index is not valid");

    auto* parameterSpan = index.data(Qt::UserRole).value<PipelineParameterSpan*>();
    if (!parameterSpan)
        throw std::runtime_error("Parameter span must not be null");

    auto& spanSet = ParameterSpace.GetSpanSet(index.parent().row());

    beginResetModel();
    spanSet.RemoveParameterSpan(*parameterSpan);
    endResetModel();
}
