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
    StructureIdx Idx;

    constexpr CtStructureTreeEvent(CtStructureTreeEventType type, StructureIdx idx) noexcept : Type(type), Idx(idx) {};
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
                             StructureIdx structureToRefineIdx) -> void;

    auto
    RemoveBasicStructure(StructureIdx structureToRemoveIdx) -> void;

    [[nodiscard]] auto
    HasRoot() const noexcept -> bool;

    [[nodiscard]] auto
    GetRoot() const -> const StructureVariant&;

    [[nodiscard]] auto
    GetStructureAt(StructureIdx idx) const -> const StructureVariant&;

    [[nodiscard]] auto
    GetStructureAt(StructureIdx idx) -> StructureVariant&;

    [[nodiscard]] auto
    StructureCount() const noexcept -> StructureIdx;

    [[nodiscard]] auto
    FunctionValueAndRadiodensity(Point point) const -> CtStructureBase::ModelingResult;

    void SetData(StructureIdx structureIdx, const QVariant& data);

    using TreeEventCallback = std::function<void(const CtStructureTreeEvent&)>;
    void AddTreeEventCallback(TreeEventCallback&& treeEventCallback);

    auto GetRootIdx() const noexcept -> StructureId;

private:
    template<TCtStructure TStructure>
    [[nodiscard]] auto
    CtStructureExists(const TStructure& ctStructure) const -> bool;

    [[nodiscard]] auto
    CtStructureExists(const BasicStructureVariant& basicStructureVariant) const -> bool;

    [[nodiscard]] auto
    StructureIdxExists(StructureId idx) const noexcept -> bool;

    auto
    EmitEvent(CtStructureTreeEvent event) noexcept -> void;

    template<TCtStructure TStructure>
    [[nodiscard]] auto
    FindIndexOf(const TStructure& structure) const -> StructureIdx;

    [[nodiscard]] auto
    GetParentIdxOf(const TCtStructure auto& ctStructure) const -> StructureId;

    auto
    IncrementParentAndChildIndices(StructureIdx startIdx) -> void;

    auto
    DecrementParentAndChildIndices(StructureIdx startIdx) -> void;

    StructureId RootIdx = -1;
    std::vector<StructureVariant> Structures;
    vtkTimeStamp MTime;
    std::vector<TreeEventCallback> TreeEventCallbacks;
};


