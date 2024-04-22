#pragma once

#include "CtStructure.h"

class QComboBox;

class CombinedStructure;

namespace CombinedStructureDetails {
    Q_NAMESPACE

    enum struct OperatorType : uint8_t {
        UNION,
        INTERSECTION,
        DIFFERENCE
    };
    Q_ENUM_NS(OperatorType);

    [[nodiscard]] auto static
    OperatorTypeToString(OperatorType operatorType) noexcept -> std::string {
        switch (operatorType) {
            case OperatorType::UNION:        return "Union";
            case OperatorType::INTERSECTION: return "Intersection";
            case OperatorType::DIFFERENCE:   return "Difference";
        }
        return "";
    }
    ENUM_GET_VALUES(OperatorType, false);

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
        AddData(Data& data) noexcept -> void;

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

public:
    [[nodiscard]] auto static
    GetWidgetData(QWidget* widget) -> CombinedStructureData { return FindWidget(widget).GetData(); }

    auto static
    SetWidgetData(QWidget* widget, const CombinedStructureData& data) -> void { FindWidget(widget).Populate(data); }

private:
    [[nodiscard]] auto static
    FindWidget(QWidget* widget) -> CombinedStructureWidget& {
        if (!widget)
            throw std::runtime_error("Given widget must not be nullptr");

        auto* combinedStructureWidget = widget->findChild<CombinedStructureWidget*>();

        if (!combinedStructureWidget)
            throw std::runtime_error("No basic structure widget contained in given widget");

        return *combinedStructureWidget;
    }
};

using CombinedStructureDetails::OperatorType;



class CombinedStructure : public CtStructureBase {
public:
    using Data = CombinedStructureData;

    explicit CombinedStructure(OperatorType operatorType = OperatorType::UNION);
    explicit CombinedStructure(const CombinedStructureData & data);

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
    friend struct EvaluateImplicitStructures;
    friend CombinedStructureDetails::CombinedStructureDataImpl;

    OperatorType Operator = OperatorType::UNION;
    std::vector<uidx_t> ChildStructureIndices;
};
