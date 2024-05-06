#pragma once

#include "BasicStructure.h"
#include "CombinedStructure.h"

#include <vtkTimeStamp.h>

#include <variant>
#include <vector>

class PipelineList;

class QVariant;

enum struct CtStructureTreeEventType : uint8_t {
    Add,
    Remove,
    Edit
};

struct CtStructureTreeEvent {
    CtStructureTreeEventType Type;
    uidx_t Idx;

    constexpr CtStructureTreeEvent(CtStructureTreeEventType type, uidx_t idx) noexcept : Type(type), Idx(idx) {};
};

using StructureDataVariant = std::variant<BasicStructureData, CombinedStructureData>;

class CtStructureTree {
public:
    using StructureVariant = std::variant<BasicStructure, CombinedStructure>;

    [[nodiscard]] auto
    GetMTime() const -> vtkMTimeType;

    auto
    AddBasicStructure(BasicStructure&& basicStructure, CombinedStructure* parent = nullptr) -> void;

    auto
    AddBasicStructure(const BasicStructureData& basicStructureData,
                      CombinedStructure* parent = nullptr) -> void;

    auto
    CombineWithBasicStructure(BasicStructure&& basicStructure,
                              CombinedStructure&& combinedStructure) -> void;

    auto
    CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                              const CombinedStructureData& combinedStructureData) -> void;

    auto
    RefineWithBasicStructure(const BasicStructureData& newStructureData,
                             const CombinedStructureData& combinedStructureData,
                             uidx_t structureToRefineIdx) -> void;

    auto
    RemoveBasicStructure(uidx_t structureToRemoveIdx) -> void;

    [[nodiscard]] auto
    HasRoot() const noexcept -> bool;

    [[nodiscard]] auto
    GetRoot() const -> const StructureVariant&;

    [[nodiscard]] auto
    GetStructureAt(uidx_t idx) const -> const StructureVariant&;

    [[nodiscard]] auto
    GetStructureAt(uidx_t idx) -> StructureVariant&;

    [[nodiscard]] auto
    StructureCount() const noexcept -> uidx_t;

    struct ModelingResult {
        float FunctionValue;
        float Radiodensity;
        StructureId BasicCtStructureId;
    };

    [[nodiscard]] auto
    FunctionValueAndRadiodensity(Point point, StructureVariant const* = nullptr) const -> ModelingResult;

    [[nodiscard]] auto
    FunctionValue(Point point, StructureVariant const& structure) const -> float;

    void SetData(uidx_t structureIdx, const QVariant& data);

    using TreeEventCallback = std::function<void(const CtStructureTreeEvent&)>;
    void AddTreeEventCallback(TreeEventCallback&& treeEventCallback);

    [[nodiscard]] auto
    GetRootIdx() const noexcept -> idx_t;

    [[nodiscard]] auto
    GetBasicStructureIdsOfStructureAt(uidx_t idx) const noexcept -> std::vector<StructureId>;

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
    TransformPointUntilStructure(Point point, StructureVariant const& structure) const -> Point {
        std::vector<StructureVariant const*> ancestors;
        uidx_t const structureIdx = std::visit([&](auto const& s) { return FindIndexOf(s); }, structure);

        idx_t ancestorIdx = std::visit([](auto const& s) { return s.ParentIdx; }, structure);
        while (ancestorIdx) {
            auto const& ancestor = GetStructureAt(*ancestorIdx);
            ancestors.push_back(&ancestor);

            ancestorIdx = std::visit([](auto const& s) { return s.ParentIdx; }, ancestor);
        }

        return std::accumulate(ancestors.crbegin(), ancestors.crend(), point,
                        [](Point point, StructureVariant const* ancestor) {
            return std::visit([&](auto const& s) { return s.GetTransformedPoint(point); }, *ancestor);
        });
    }

    vtkTimeStamp MTime;
    std::vector<StructureVariant> Structures;
    std::vector<TreeEventCallback> TreeEventCallbacks;
    idx_t RootIdx;
};


