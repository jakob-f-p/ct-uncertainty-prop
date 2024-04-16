#include "CombinedStructure.h"

#include <QComboBox>
#include <QLabel>
#include <QFormLayout>

auto
CombinedStructureDataImpl::PopulateDerivedStructure(Structure& structure) const noexcept -> void {
    structure.Operator = Operator;
}

auto CombinedStructureDataImpl::PopulateFromDerivedStructure(const Structure& structure) noexcept -> void {
    Operator = structure.Operator;
}

auto CombinedStructureDataImpl::PopulateStructureWidget(QWidget* widget) const -> void {
    auto* operatorTypeComboBox = widget->findChild<QComboBox*>(OperatorTypeComboBoxName);

    if (int idx = operatorTypeComboBox->findData(QVariant::fromValue(Operator));
            idx != -1) {
        operatorTypeComboBox->setCurrentIndex(idx);
    }
}

auto CombinedStructureDataImpl::PopulateFromStructureWidget(QWidget* widget) -> void {
    auto* operatorTypeComboBox = widget->findChild<QComboBox*>(OperatorTypeComboBoxName);

    Operator = operatorTypeComboBox->currentData().value<CtStructureBase::OperatorType>();
}

const QString CombinedStructureDataImpl::OperatorTypeComboBoxName = "OperatorType";

void CombinedStructureDataImpl::AddSubTypeWidgets(QFormLayout* fLayout) {
    auto* operatorTypeComboBox = new QComboBox();
    operatorTypeComboBox->setObjectName(OperatorTypeComboBoxName);
    for (const auto &operatorAndName : CtStructureBase::GetOperatorTypeValues())
        operatorTypeComboBox->addItem(operatorAndName.Name, QVariant::fromValue(operatorAndName.EnumValue));

    fLayout->addRow("Operator Type", operatorTypeComboBox);
}

void CombinedStructure::SetOperatorType(CtStructureBase::OperatorType operatorType) noexcept {
    Operator = operatorType;

    Modified();
}

auto CombinedStructure::AddStructureIndex(StructureIdx idx) -> void {
    ChildStructureIndices.push_back(idx);
}

auto CombinedStructure::RemoveStructureIndex(StructureIdx idx) -> void {
    auto removeIt = std::find(ChildStructureIndices.begin(), ChildStructureIndices.end(), idx);
    if (removeIt == ChildStructureIndices.end())
        throw std::runtime_error("Given index could not be removed because it was not present");

    ChildStructureIndices.erase(removeIt);
}

template<typename C, typename U>
auto CombinedStructure::UpdateIndices(C check, StructureIdx threshold, U update, StructureId change) noexcept -> void {
    for (auto& idx: ChildStructureIndices) {
        if (check(idx, threshold))
            update(idx, change);
    }

    auto parentIdx = GetParentIdx();
    if (check(parentIdx, threshold))
        update(parentIdx, change);
}

auto CombinedStructure::GetChildIndices() const noexcept -> const std::vector<StructureIdx>& {
    return ChildStructureIndices;
}

auto CombinedStructure::UpdateChildIndicesGreaterThanOrEqualToBy(StructureIdx startIdx, int8_t change) noexcept -> void {
    for (auto& childIdx : ChildStructureIndices) {
        if (childIdx >= startIdx)
            childIdx += change;
    }
}

auto CombinedStructure::StructureCount() const noexcept -> StructureIdx {
    return static_cast<StructureIdx>(ChildStructureIndices.size());
}

auto CombinedStructure::StructureIdxAt(StructureIdx positionIdx) const -> StructureIdx {
    return ChildStructureIndices.at(positionIdx);
}

auto CombinedStructure::HasStructureIndex(StructureIdx childIdx) const noexcept -> bool {
    return std::find(ChildStructureIndices.begin(), ChildStructureIndices.end(), childIdx)
           != ChildStructureIndices.end();
}

auto CombinedStructure::PositionIndex(StructureIdx childIdx) const -> int {
    auto searchIt = std::find(ChildStructureIndices.begin(), ChildStructureIndices.end(), childIdx);
    if(searchIt == ChildStructureIndices.end())
        throw std::runtime_error("Given Structure is not contained in children");

    return static_cast<int>(std::distance(ChildStructureIndices.begin(), searchIt));
}

auto CombinedStructure::GetViewName() const noexcept -> std::string {
    return GetOperatorTypeName() + (Name.empty() ? "" : " (" + Name + ")");
}

auto CombinedStructure::GetOperatorTypeName() const -> std::string {
    return OperatorTypeToString(Operator);
}

auto CombinedStructure::ReplaceChild(StructureId oldIdx, StructureId newIdx) -> void {
    auto oldIt = std::find(ChildStructureIndices.begin(), ChildStructureIndices.end(), oldIdx);
    if (oldIt == ChildStructureIndices.end())
        throw std::runtime_error("Given old child index is not a child index of this structure");

    *oldIt = newIdx;
}

auto CombinedStructure::GetData() const noexcept -> Data {
    Data data {};
    data.PopulateFromStructure(*this);
    return data;
}

auto CombinedStructure::SetData(const Data& data) noexcept -> void {
    data.PopulateStructure(*this);
}

auto CombinedStructure::operator==(const CombinedStructure& other) const noexcept -> bool {
    return Operator == other.Operator
            && ChildStructureIndices == other.ChildStructureIndices;
}

auto CombinedStructureUi::GetWidgetData(QWidget* widget) -> CombinedStructureData {
    CombinedStructureData data {};
    data.PopulateFromWidget(widget);
    return data;
}

auto CombinedStructureUi::GetWidget() -> QWidget* {
    return CombinedStructureData::GetWidget();
}
