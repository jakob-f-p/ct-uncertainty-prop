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
    AddStructureIndex(StructureIdx idx) -> void;

    auto
    RemoveStructureIndex(StructureIdx idx) -> void;

    template<typename C, typename U>
    auto
    UpdateIndices(C check, StructureIdx threshold, U update, StructureId change) noexcept -> void;

    [[nodiscard]] auto
    GetChildIndices() const noexcept -> const std::vector<StructureIdx>&;

    auto
    UpdateChildIndicesGreaterThanOrEqualToBy(StructureIdx startIdx, int8_t change) noexcept -> void;

    [[nodiscard]] auto
    StructureCount() const noexcept -> StructureIdx;

    [[nodiscard]] auto
    StructureIdxAt(StructureIdx positionIdx) const -> StructureIdx;

    [[nodiscard]] auto
    PositionIndex(StructureIdx childIdx) const -> int;

    [[nodiscard]] auto
    HasStructureIndex(StructureIdx childIdx) const noexcept -> bool;

    auto
    ReplaceChild(StructureId oldIdx, StructureId newIdx) -> void;

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
    std::vector<StructureIdx> ChildStructureIndices;
};


class CombinedStructureUi {
public:
    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> CombinedStructureData;

    [[nodiscard]] auto static
    GetWidget() -> QWidget*;
};
