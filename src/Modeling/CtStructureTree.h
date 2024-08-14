#pragma once

#include "BasicStructure.h"
#include "CombinedStructure.h"

#include <vtkTimeStamp.h>

#include <variant>
#include <vector>

class PipelineList;

class QVariant;

enum struct CtStructureTreeEventType : uint8_t {
    ADD,
    REMOVE,
    EDIT
};

struct CtStructureTreeEvent {
    CtStructureTreeEventType Type;
    uidx_t Idx;

    constexpr CtStructureTreeEvent(CtStructureTreeEventType type, uidx_t idx) noexcept : Type(type), Idx(idx) {};
};

using StructureDataVariant = std::variant<BasicStructureData, CombinedStructureData>;

class CtStructureVariant : public std::variant<BasicStructure, CombinedStructure> {};

class CtStructureTree {
public:

    [[nodiscard]] auto
    GetMTime() const -> vtkMTimeType;

    auto
    AddBasicStructure(BasicStructure&& basicStructure, CombinedStructure* parent = nullptr) -> void;

    auto
    AddBasicStructure(BasicStructureData const& basicStructureData,
                      CombinedStructure* parent = nullptr) -> void;

    auto
    CombineWithBasicStructure(BasicStructure&& basicStructure,
                              CombinedStructure&& combinedStructure) -> void;

    auto
    CombineWithBasicStructure(BasicStructureData const& basicStructureData,
                              CombinedStructureData const& combinedStructureData) -> void;

    auto
    RefineWithBasicStructure(BasicStructure&& basicStructure,
                             CombinedStructure&& combinedStructure,
                             uidx_t structureToRefineIdx) -> void;

    auto
    RefineWithBasicStructure(BasicStructureData const& newStructureData,
                             CombinedStructureData const& combinedStructureData,
                             uidx_t structureToRefineIdx) -> void;

    auto
    RemoveBasicStructure(uidx_t structureToRemoveIdx) -> void;

    [[nodiscard]] auto
    HasRoot() const noexcept -> bool;

    [[nodiscard]] auto
    GetRoot() const -> CtStructureVariant const&;

    [[nodiscard]] auto
    GetRoot() -> CtStructureVariant&;

    [[nodiscard]] auto
    GetStructureAt(uidx_t idx) const -> CtStructureVariant const&;

    [[nodiscard]] auto
    GetStructureAt(uidx_t idx) -> CtStructureVariant&;

    [[nodiscard]] auto
    StructureCount() const noexcept -> uidx_t;

    struct ModelingResult {
        float FunctionValue;
        float Radiodensity;
        StructureId BasicCtStructureId;
    };

    [[nodiscard]] auto
    FunctionValueAndRadiodensity(Point point, CtStructureVariant const* = nullptr) const -> ModelingResult;

    [[nodiscard]] auto
    FunctionValue(Point point, CtStructureVariant const& structure) const -> float;

    [[nodiscard]] auto
    ClosestPointOnXYPlane(Point const& point, CtStructureVariant const& structure) const -> std::optional<DoublePoint>;

    void SetData(uidx_t structureIdx, const QVariant& data);

    using TreeEventCallback = std::function<void(CtStructureTreeEvent const&)>;
    void AddTreeEventCallback(TreeEventCallback&& treeEventCallback);

    [[nodiscard]] auto
    GetRootIdx() const noexcept -> idx_t;

    [[nodiscard]] auto
    GetBasicStructureIdsOfStructureAt(uidx_t idx) const -> std::vector<StructureId>;

    [[nodiscard]] auto
    GetBasicStructureIds(CtStructureVariant const& structure) const -> std::vector<StructureId>;

    [[nodiscard]] auto
    GetMaxTissueValue(CtStructureVariant const& structure) const -> float;

private:
    [[nodiscard]] auto
    CtStructureExists(auto const& ctStructure) const -> bool;

    [[nodiscard]] auto
    StructureIdxExists(uidx_t idx) const noexcept -> bool;

    auto
    EmitEvent(CtStructureTreeEvent event) noexcept -> void;

    [[nodiscard]] auto
    FindIndexOf(auto const& structure) const -> uidx_t;

    [[nodiscard]] auto
    GetParentIdxOf(auto const& ctStructure) const -> idx_t;

    auto
    IncrementParentAndChildIndices(uidx_t startIdx) -> void;

    auto
    DecrementParentAndChildIndices(uidx_t startIdx) -> void;

    [[nodiscard]] auto
    TransformPointUntilStructure(Point point, CtStructureVariant const& structure) const -> Point {
        std::vector<CtStructureVariant const*> ancestors;
        uidx_t const structureIdx = std::visit([&](auto const& s) { return FindIndexOf(s); }, structure);

        idx_t ancestorIdx = std::visit([](auto const& s) { return s.ParentIdx; }, structure);
        while (ancestorIdx) {
            auto const& ancestor = GetStructureAt(*ancestorIdx);
            ancestors.push_back(&ancestor);

            ancestorIdx = std::visit([](auto const& s) { return s.ParentIdx; }, ancestor);
        }

        return std::accumulate(ancestors.crbegin(), ancestors.crend(), point,
                        [](Point point, CtStructureVariant const* ancestor) {
            return std::visit([&](auto const& s) { return s.GetTransformedPoint(point); }, *ancestor);
        });
    }

    [[nodiscard]] auto
    TransformPointFromStructure(Point point, CtStructureVariant const& structure) const -> Point {
        std::vector<CtStructureVariant const*> ancestors;
        uidx_t const structureIdx = std::visit([&](auto const& s) { return FindIndexOf(s); }, structure);

        idx_t ancestorIdx = std::visit([](auto const& s) { return s.ParentIdx; }, structure);
        while (ancestorIdx) {
            auto const& ancestor = GetStructureAt(*ancestorIdx);
            ancestors.push_back(&ancestor);

            ancestorIdx = std::visit([](auto const& s) { return s.ParentIdx; }, ancestor);
        }

        return std::accumulate(ancestors.cbegin(), ancestors.cend(), point,
                               [](Point point, CtStructureVariant const* ancestor) {
            return std::visit([&](auto const& s) { return s.GetInverselyTransformedPoint(point); }, *ancestor);
        });
    }

    vtkTimeStamp MTime;
    std::vector<CtStructureVariant> Structures;
    std::vector<TreeEventCallback> TreeEventCallbacks;
    idx_t RootIdx;
};


