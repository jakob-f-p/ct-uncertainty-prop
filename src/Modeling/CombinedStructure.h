#pragma once

#include "CtStructure.h"

#include "../Utils/Enum.h"
#include "../Utils/IndexTypes.h"

class QComboBox;

class CombinedStructure;

namespace CombinedStructureDetails {
    Q_NAMESPACE

    enum struct OperatorType : uint8_t {
        UNION,
        INTERSECTION,
        DIFFERENCE_
    };
    Q_ENUM_NS(OperatorType)

    [[nodiscard]] auto static
    OperatorTypeToString(OperatorType operatorType) noexcept -> std::string {
        switch (operatorType) {
            case OperatorType::UNION:        return "Union";
            case OperatorType::INTERSECTION: return "Intersection";
            case OperatorType::DIFFERENCE_:  return "Difference";
        }
        return "";
    }
    ENUM_GET_VALUES(OperatorType);

    struct CombinedStructureDataImpl {
        OperatorType Operator = OperatorType::UNION;

        using Structure = CombinedStructure;

        auto
        PopulateStructure(Structure& structure) const noexcept -> void;

        auto
        PopulateFromStructure(const Structure& structure) noexcept -> void;
    };

    class CombinedStructureWidgetImpl : public QWidget {
    public:
        using Data = CombinedStructureDataImpl;

        CombinedStructureWidgetImpl();

        auto
        GetData() noexcept -> Data;

        auto
        Populate(const Data& data) noexcept -> void;

    private:
        QFormLayout* Layout;
        QComboBox* OperatorComboBox;
    };
}

class CombinedStructureData : public CtStructureBaseData<CombinedStructureDetails::CombinedStructureDataImpl> {};

class CombinedStructureWidget : public CtStructureBaseWidget<CombinedStructureDetails::CombinedStructureWidgetImpl,
                                                             CombinedStructureData> {
    Q_OBJECT
};




class CombinedStructure : public CtStructureBase {
public:
    using Data = CombinedStructureData;
    using OperatorType = CombinedStructureDetails::OperatorType;

    explicit CombinedStructure(OperatorType operatorType = OperatorType::UNION);
    explicit CombinedStructure(CombinedStructureData const& data);

    auto
    SetOperatorType(OperatorType operatorType) noexcept-> void;

    auto
    AddStructureIndex(uidx_t idx) -> void;

    auto
    RemoveStructureIndex(uidx_t idx) -> void;

    [[nodiscard]] auto
    GetViewName() const noexcept -> std::string;

    auto
    operator==(const CombinedStructure& other) const noexcept -> bool;

    [[nodiscard]] auto
    StructureCount() const noexcept -> uidx_t;

private:
    friend struct EvaluateImplicitStructures;
    friend struct EvaluateFunctionValue;
    friend struct AddBasicStructureIndices;
    friend struct MaxTissueValueAlgorithm;
    friend struct FindClosestPointOnXYPlane;
    friend struct CombinedStructureDetails::CombinedStructureDataImpl;
    friend class CtStructureTree;
    friend class CtStructureTreeModel;

    [[nodiscard]] auto
    GetChildIndices() const noexcept -> const std::vector<uidx_t>&;

    auto
    UpdateChildIndicesGreaterThanOrEqualToBy(uidx_t startIdx, int8_t change) noexcept -> void;

    [[nodiscard]] auto
    StructureIdxAt(uidx_t positionIdx) const -> uidx_t;

    [[nodiscard]] auto
    PositionIndex(uidx_t childIdx) const -> int;

    auto
    ReplaceChild(uidx_t oldIdx, uidx_t newIdx) -> void;

    OperatorType Operator = OperatorType::UNION;
    std::vector<uidx_t> ChildStructureIndices;
};
