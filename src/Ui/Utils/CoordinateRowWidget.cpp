#include "CoordinateRowWidget.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>

CoordinateRowWidget::CoordinateRowWidget(CoordinateRowWidget::NumericSettings numericSettings,
                                         const QString& labelText) :
        GLayout(new QGridLayout(this)),
        HasLabel(!labelText.isEmpty()) {
    GLayout->setContentsMargins(0, 0, 0, 0);

    AppendCoordinatesRow(std::move(numericSettings), labelText);
}

CoordinateRowWidget::CoordinateRowWidget(bool hasLabel) :
        GLayout(new QGridLayout(this)),
        HasLabel(hasLabel) {
    GLayout->setContentsMargins(0, 0, 0, 0);
}

auto CoordinateRowWidget::AppendCoordinatesRow(const NumericSettings& numericSettings,
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

        auto* coordinateSpinBox = new QDoubleSpinBox();
        coordinateSpinBox->setRange(numericSettings.Min, numericSettings.Max);
        coordinateSpinBox->setSingleStep(numericSettings.StepSize);
        coordinateSpinBox->setValue(numericSettings.DefaultValue);
        GLayout->addWidget(coordinateSpinBox, gridLayoutRow, gridLayoutColumn++);

        spinBoxRow.SetSpinBox(i, *coordinateSpinBox);
    }
    Rows.emplace_back(spinBoxRow);
}

auto CoordinateRowWidget::GetRowData(uint8_t rowIdx) const -> CoordinateRowWidget::RowData {
    if (rowIdx >= Rows.size())
        throw std::runtime_error("Given row index too large");

    const auto& row = Rows[rowIdx];

    return { row.X->value(), row.Y->value(), row.Z->value() };
}

auto CoordinateRowWidget::GetRowData() const noexcept -> std::vector<RowData> {
    std::vector<RowData> rowData;

    std::transform(Rows.cbegin(), Rows.cend(), std::back_inserter(rowData),
                   [](const auto& row) -> RowData { return { row.X->value(), row.Y->value(), row.Z->value() }; });

    return rowData;
}

auto CoordinateRowWidget::SetRowData(uint8_t rowIdx, CoordinateRowWidget::RowData data) -> void {
    if (rowIdx >= Rows.size())
        throw std::runtime_error("Given row index too large");

    const auto& row = Rows[rowIdx];
    row.X->setValue(data.X);
    row.Y->setValue(data.Y);
    row.Z->setValue(data.Z);
}
