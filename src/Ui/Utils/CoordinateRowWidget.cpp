#include "CoordinateRowWidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

template<typename ValueT, typename SpinBoxT>
CoordinateRowWidget<ValueT, SpinBoxT>::CoordinateRowWidget(NumericSettings numericSettings,
                                           const QString& labelText) :
        HasLabel(!labelText.isEmpty()),
        GLayout(new QGridLayout(this)) {
    GLayout->setContentsMargins(0, 0, 0, 0);

    AppendCoordinatesRow(numericSettings, labelText);
}

template<typename ValueT, typename SpinBoxT>
CoordinateRowWidget<ValueT, SpinBoxT>::CoordinateRowWidget(bool hasLabel) :
        HasLabel(hasLabel),
        GLayout(new QGridLayout(this)) {

    GLayout->setContentsMargins(0, 0, 0, 0);
}

template<typename ValueT, typename SpinBoxT>
auto CoordinateRowWidget<ValueT, SpinBoxT>::AppendCoordinatesRow(const NumericSettings& numericSettings,
                                               const QString& labelText) -> void {

    if (labelText.isEmpty() != !HasLabel)
        throw std::runtime_error("Cannot add row. Either all rows have a label or none have.");

    const size_t gridLayoutRow = Rows.size();
    int gridLayoutColumn = 0;

    if (!labelText.isEmpty())
        GLayout->addWidget(new QLabel(labelText), gridLayoutRow, gridLayoutColumn++);

    SpinBoxRow spinBoxRow {};
    for (int i = 0; i < AxisNames.size(); i++) {
        auto* coordinateLabel = new QLabel(AxisNames[i]);
        if (i > 0)
            coordinateLabel->setMinimumWidth(15);
        coordinateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        GLayout->addWidget(coordinateLabel, gridLayoutRow, gridLayoutColumn++);

        auto* coordinateSpinBox = new SpinBoxT();
        coordinateSpinBox->setRange(numericSettings.Min, numericSettings.Max);
        coordinateSpinBox->setSingleStep(numericSettings.StepSize);
        coordinateSpinBox->setValue(numericSettings.DefaultValue);
        GLayout->addWidget(coordinateSpinBox, gridLayoutRow, gridLayoutColumn++);

        spinBoxRow.SetSpinBox(i, *coordinateSpinBox);
    }
    Rows.emplace_back(spinBoxRow);

    for (auto* spinBox : std::vector({ spinBoxRow.X, spinBoxRow.Y, spinBoxRow.Z }))
        connect(spinBox, &SpinBoxT::valueChanged,
                this, [this] { Q_EMIT ValueChanged(); });
}

template<typename ValueT, typename SpinBoxT>
auto CoordinateRowWidget<ValueT, SpinBoxT>::GetRowData(uint8_t rowIdx) const -> RowData {
    if (rowIdx >= Rows.size())
        throw std::runtime_error("Given row index too large");

    const auto& row = Rows[rowIdx];

    return { row.X->value(), row.Y->value(), row.Z->value() };
}

template<typename ValueT, typename SpinBoxT>
auto CoordinateRowWidget<ValueT, SpinBoxT>::GetRowData() const noexcept -> std::vector<RowData> {
    std::vector<RowData> rowData;

    std::transform(Rows.cbegin(), Rows.cend(), std::back_inserter(rowData),
                   [](const auto& row) -> RowData { return { row.X->value(), row.Y->value(), row.Z->value() }; });

    return rowData;
}

template<typename ValueT, typename SpinBoxT>
auto CoordinateRowWidget<ValueT, SpinBoxT>::SetRowData(uint8_t rowIdx, RowData data) -> void {
    if (rowIdx >= Rows.size())
        throw std::runtime_error("Given row index too large");

    const auto& row = Rows[rowIdx];
    row.X->setValue(data.X);
    row.Y->setValue(data.Y);
    row.Z->setValue(data.Z);
}

template class CoordinateRowWidget<double, QDoubleSpinBox>;
template class CoordinateRowWidget<int, QSpinBox>;
