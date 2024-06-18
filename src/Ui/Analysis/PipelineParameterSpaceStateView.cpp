#include "PipelineParameterSpaceStateView.h"

#include "../../PipelineGroups/PipelineParameterSpace.h"
#include "../../Utils/Overload.h"

#include <QFontDatabase>


PipelineParameterSpaceStateView::PipelineParameterSpaceStateView(
        PipelineParameterSpaceState const& parameterSpaceState) {

    setModel(new PipelineParameterSpaceStateModel(parameterSpaceState));

    setMinimumWidth(350);

    expandAll();
}



PipelineParameterSpaceStateModel::PipelineParameterSpaceStateModel(
        PipelineParameterSpaceState const& parameterSpaceState, QObject* parent) :
        QAbstractItemModel(parent),
        ParameterSpaceState(parameterSpaceState),
        ParameterSpace(parameterSpaceState.ParameterSpace),
        Format([this]() {
            float max = 0.1;
            for (int i = 0; i < ParameterSpace.GetNumberOfSpanSets(); i++) {
                auto const& spanSet = ParameterSpace.GetSpanSet(i);

                for (int j = 0; j < spanSet.GetSize(); j++) {
                    auto const& span = spanSet.Get(j);

                    auto const range = span.GetRange();
                    max = std::max({ max, std::abs(range.Min), std::abs(range.Max) });
                }
            }
            uint16_t const numberOfNonDecimals = std::floor(std::log10(max)) + 1;
            uint16_t const numberOfDecimals = 2;
            uint16_t const width = numberOfNonDecimals + numberOfDecimals + 2;
            return NumberFormat { width, numberOfDecimals };
        }()) {}

auto PipelineParameterSpaceStateModel::index(int row, int column, QModelIndex const& parent) const -> QModelIndex {
    if (!hasIndex(row, column, parent))
        return {};

    return createIndex(row, column, parent.row());
}

auto PipelineParameterSpaceStateModel::parent(QModelIndex const& child) const -> QModelIndex {
    if (!child.isValid())
        return {};

    bool const isSpanSet = static_cast<int>(child.internalId()) == -1; // === parent.isInvalid()
    if (isSpanSet)
        return {};

    uint16_t const parentIdx = static_cast<int>(child.internalId());

    return createIndex(parentIdx, 0, -1);
}

auto PipelineParameterSpaceStateModel::rowCount(QModelIndex const& parent) const -> int {
    if (parent.isValid() && parent.parent().isValid())
        return 0;

    if (parent.isValid())
        return ParameterSpace.GetSpanSet(parent.row()).GetSize();

    return ParameterSpace.GetNumberOfSpanSets();
}

auto PipelineParameterSpaceStateModel::columnCount(QModelIndex const& /*parent*/) const -> int {
    return 2;
}

auto PipelineParameterSpaceStateModel::flags(QModelIndex const& index) const -> Qt::ItemFlags {
    if (!index.isValid() || !IsParameterSpan(index))
        return {};

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

auto PipelineParameterSpaceStateModel::headerData(int section,
                                                  Qt::Orientation orientation,
                                                  int role) const -> QVariant {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
        case 0:  return "Artifact / Span";
        case 1:  return "Value";
        default: throw std::runtime_error("invalid section index");
    }
}

auto PipelineParameterSpaceStateModel::data(QModelIndex const& index, int role) const -> QVariant {
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::FontRole && role != Qt::ToolTipRole))
        return {};

    QModelIndex const parentIndex = index.parent();

    if (parentIndex.isValid()) {
        auto const& spanSet = ParameterSpace.GetSpanSet(parentIndex.row());
        auto const& parameterSpan = spanSet.Get(index.row());

        if (role == Qt::FontRole) {
            switch (index.column()) {
                case 0: return {};
                case 1: return QFontDatabase::systemFont(QFontDatabase::FixedFont);
                default: throw std::runtime_error("invalid column index");
            }
        }

        switch (index.column()) {
            case 0: return QString::fromStdString(parameterSpan.GetName());
            case 1: {
                auto const& spanState = ParameterSpaceState.FindSpanStateBySpan(parameterSpan);
                auto const stateValue = spanState.GetValue();

                std::string const valueString = std::visit(Overload {
                    [this](float const val) { return std::format("{:{}.{}f}", val, Format.Width, Format.Precision); },
                    [this](FloatPoint const val) {
                        return std::format("({:{}.{}f}, {:{}.{}f}, {:{}.{}f})",
                                           val[0], Format.Width, Format.Precision,
                                           val[1], Format.Width, Format.Precision,
                                           val[2], Format.Width, Format.Precision); }
                }, stateValue);

                return QString::fromStdString(valueString);
            }
            default: throw std::runtime_error("invalid column index");
        }
    }

    if (role == Qt::FontRole || index.column() == 1)
        return {};

    auto const& spanSet = ParameterSpace.GetSpanSet(index.row());

    return QString::fromStdString(ParameterSpace.GetSpanSetName(spanSet));
}

auto PipelineParameterSpaceStateModel::IsParameterSpan(QModelIndex const& index) noexcept -> bool {
    return index.isValid() && index.parent().isValid();
}
