#include "CombinedStructure.h"

#include <QComboBox>
#include <QLabel>
#include <QFormLayout>

auto
CombinedStructureDetails::CombinedStructureDataImpl::PopulateStructure(Structure& structure) const noexcept -> void {
    structure.Operator = Operator;
}

auto
CombinedStructureDetails::CombinedStructureDataImpl::PopulateFromStructure(const Structure& structure) noexcept -> void {
    Operator = structure.Operator;
}

CombinedStructureDetails::CombinedStructureWidgetImpl::CombinedStructureWidgetImpl() :
        Layout(new QFormLayout(this)),
        OperatorComboBox(new QComboBox()) {

    Layout->setContentsMargins(0, 5, 0, 5);
    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    for (auto const & [name, enumValue] : GetOperatorTypeValues())
        OperatorComboBox->addItem(name, QVariant::fromValue(enumValue));

    Layout->addRow("Operator Type", OperatorComboBox);
}

auto CombinedStructureDetails::CombinedStructureWidgetImpl::GetData() const noexcept -> Data {
    Data data {};

    data.Operator = OperatorComboBox->currentData().value<OperatorType>();

    return data;
}

auto CombinedStructureDetails::CombinedStructureWidgetImpl::Populate(const Data& data) const noexcept -> void {
    if (int const idx = OperatorComboBox->findData(QVariant::fromValue(data.Operator));
            idx != -1)
        OperatorComboBox->setCurrentIndex(idx);
}



CombinedStructure::CombinedStructure(OperatorType operatorType) :
        Operator(operatorType) {
}

CombinedStructure::CombinedStructure(const CombinedStructureData& data) : CombinedStructure(data.Data.Operator) {
    data.PopulateStructure(*this);
}

void CombinedStructure::SetOperatorType(OperatorType operatorType) noexcept {
    Operator = operatorType;

    Modified();
}

auto CombinedStructure::AddStructureIndex(uidx_t idx) -> void {
    ChildStructureIndices.push_back(idx);
}

auto CombinedStructure::RemoveStructureIndex(uidx_t const idx) -> void {
    auto const removeIt = std::ranges::find(ChildStructureIndices, idx);
    if (removeIt == ChildStructureIndices.end())
        throw std::runtime_error("Given index could not be removed because it was not present");

    ChildStructureIndices.erase(removeIt);
}

auto CombinedStructure::GetChildIndices() const noexcept -> const std::vector<uidx_t>& {
    return ChildStructureIndices;
}

auto CombinedStructure::UpdateChildIndicesGreaterThanOrEqualToBy(uidx_t startIdx, int8_t change) noexcept -> void {
    for (auto& childIdx : ChildStructureIndices) {
        if (childIdx >= startIdx)
            childIdx += change;
    }
}

auto CombinedStructure::StructureCount() const noexcept -> uidx_t {
    return static_cast<uidx_t>(ChildStructureIndices.size());
}

auto CombinedStructure::StructureIdxAt(uidx_t positionIdx) const -> uidx_t {
    return ChildStructureIndices.at(positionIdx);
}

auto CombinedStructure::PositionIndex(uidx_t const childIdx) const -> int {
    auto const searchIt = std::ranges::find(ChildStructureIndices, childIdx);
    if(searchIt == ChildStructureIndices.end())
        throw std::runtime_error("Given Structure is not contained in children");

    return static_cast<int>(std::distance(ChildStructureIndices.begin(), searchIt));
}

auto CombinedStructure::GetViewName() const noexcept -> std::string {
    return OperatorTypeToString(Operator) + (GetName().empty() ? "" : " (" + GetName() + ")");
}

auto CombinedStructure::ReplaceChild(uidx_t const oldIdx, uidx_t const newIdx) -> void {
    auto const oldIt = std::ranges::find(ChildStructureIndices, oldIdx);
    if (oldIt == ChildStructureIndices.end())
        throw std::runtime_error("Given old child index is not a child index of this structure");

    *oldIt = newIdx;
}

auto CombinedStructure::operator==(const CombinedStructure& other) const noexcept -> bool {
    return Operator == other.Operator
            && ChildStructureIndices == other.ChildStructureIndices;
}
