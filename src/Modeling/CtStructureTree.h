#pragma once

#include "CtStructure.h"
#include "BasicStructure.h"
#include "CombinedStructure.h"

#include <vtkTimeStamp.h>

#include <variant>
#include <vector>

class PipelineList;

class QVariant;

using StructureVariant = std::variant<SphereStructure, BoxStructure, CombinedStructure>;
using StructureDataVariant = std::variant<SphereData, BoxData, CombinedStructureData>;

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

class CtStructureTree {
public:
    [[nodiscard]] auto
    GetMTime() const -> vtkMTimeType;

    auto
    AddBasicStructure(BasicStructureVariant&& boxStructure, CombinedStructure* parent = nullptr) -> void;

    auto
    AddBasicStructure(const BasicStructureDataVariant& basicStructureData,
                      CombinedStructure* parent = nullptr) -> void;

    auto
    CombineWithBasicStructure(BasicStructureVariant&& basicStructureVariant,
                              CombinedStructure&& combinedStructure) -> void;

    auto
    CombineWithBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                              const CombinedStructureData& combinedStructureData) -> void;

    auto
    RefineWithBasicStructure(const BasicStructureDataVariant& newStructureDataVariant,
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

    [[nodiscard]] auto
    FunctionValueAndRadiodensity(Point point) const -> CtStructureBase::ModelingResult;

    void SetData(uidx_t structureIdx, const QVariant& data);

    using TreeEventCallback = std::function<void(const CtStructureTreeEvent&)>;
    void AddTreeEventCallback(TreeEventCallback&& treeEventCallback);

    auto GetRootIdx() const noexcept -> idx_t;

private:
    template<TCtStructure TStructure>
    [[nodiscard]] auto
    CtStructureExists(const TStructure& ctStructure) const -> bool;

    [[nodiscard]] auto
    CtStructureExists(const BasicStructureVariant& basicStructureVariant) const -> bool;

    [[nodiscard]] auto
    StructureIdxExists(idx_t idx) const noexcept -> bool;

    auto
    EmitEvent(CtStructureTreeEvent event) noexcept -> void;

    template<TCtStructure TStructure>
    [[nodiscard]] auto
    FindIndexOf(const TStructure& structure) const -> uidx_t;

    [[nodiscard]] auto
    GetParentIdxOf(const TCtStructure auto& ctStructure) const -> idx_t;

    auto
    IncrementParentAndChildIndices(uidx_t startIdx) -> void;

    auto
    DecrementParentAndChildIndices(uidx_t startIdx) -> void;

    idx_t RootIdx = -1;
    std::vector<StructureVariant> Structures;
    vtkTimeStamp MTime;
    std::vector<TreeEventCallback> TreeEventCallbacks;
};


