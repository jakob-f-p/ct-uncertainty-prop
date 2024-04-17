#pragma once

#include "CtStructure.h"

#include <vtkTimeStamp.h>

class CombinedStructure;

struct CombinedStructureDataImpl {
    using Structure = CombinedStructure;

    CtStructureBase::OperatorType Operator = CtStructureBase::OperatorType::INVALID;

    auto
    PopulateDerivedStructure(Structure& structure) const noexcept -> void;

    auto
    PopulateFromDerivedStructure(const Structure& structure) noexcept -> void;

    static auto
    AddSubTypeWidgets(QFormLayout* fLayout) -> void;

    auto
    PopulateStructureWidget(QWidget* widget) const -> void;

    auto
    PopulateFromStructureWidget(QWidget* widget) -> void;

private:
    static const QString OperatorTypeComboBoxName;
};

using CombinedStructureData = CtStructureBaseData<CombinedStructureDataImpl>;


class CtStructureTree;

class CombinedStructure : public CtStructureBase {
public:
    using Data = CombinedStructureData;

    auto
    SetOperatorType(OperatorType operatorType) noexcept-> void;

    auto
    AddStructureIndex(uidx_t idx) -> void;

    auto
    RemoveStructureIndex(uidx_t idx) -> void;

    [[nodiscard]] auto
    GetChildIndices() const noexcept -> const std::vector<uidx_t>&;

    auto
    UpdateChildIndicesGreaterThanOrEqualToBy(uidx_t startIdx, int8_t change) noexcept -> void;

    [[nodiscard]] auto
    StructureCount() const noexcept -> uidx_t;

    [[nodiscard]] auto
    StructureIdxAt(uidx_t positionIdx) const -> uidx_t;

    [[nodiscard]] auto
    PositionIndex(uidx_t childIdx) const -> int;

    [[nodiscard]] auto
    HasStructureIndex(uidx_t childIdx) const noexcept -> bool;

    auto
    ReplaceChild(idx_t oldIdx, idx_t newIdx) -> void;

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    auto
    GetData() const noexcept -> Data;

    auto
    SetData(const Data& data) noexcept -> void;

    auto
    operator==(const CombinedStructure& other) const noexcept -> bool;

private:
    [[nodiscard]] auto
    GetOperatorTypeName() const -> std::string;

    friend CombinedStructureDataImpl;
    friend struct EvaluateImplicitStructures;

    OperatorType Operator = OperatorType::INVALID;
    std::vector<uidx_t> ChildStructureIndices;
};


class CombinedStructureUi {
public:
    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> CombinedStructureData;

    [[nodiscard]] auto static
    GetWidget() -> QWidget*;
};
