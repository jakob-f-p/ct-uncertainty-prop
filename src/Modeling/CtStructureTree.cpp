#include "CtStructureTree.h"

#include "CtStructure.h"
#include "BasicStructure.h"
#include "CombinedStructure.h"
#include "SimpleTransform.h"

auto CtStructureTree::GetMTime() const -> vtkMTimeType {
    vtkMTimeType nodeMTime = HasRoot()
            ? std::visit([](auto const& root) { return root.GetMTime(); }, Structures[RootIdx])
            : 0;

    return std::max(MTime.GetMTime(), nodeMTime);
}

void CtStructureTree::AddBasicStructure(BasicStructureVariant&& basicStructureVariant, CombinedStructure* parent) {
    if (!parent) {  // add as root
        if (HasRoot()) {
            throw std::runtime_error("Another root is already present. Cannot add implicit Structure.");
        }

        std::visit([&](auto& basicStructure) {
            basicStructure.SetParentIdx(-1);
            Structures.emplace_back(std::move(basicStructure));
        }, basicStructureVariant);
        RootIdx = 0;
        EmitEvent({ CtStructureTreeEventType::Add, 0 });

        MTime.Modified();
        return;
    }

    // add as child
    if (CtStructureExists(basicStructureVariant))
        throw std::runtime_error("CT Structure already exists. Cannot add existing Structure.");

    StructureIdx parentIdx = FindIndexOf(*parent);
    StructureIdx insertionIdx = parentIdx + 1;

    IncrementParentAndChildIndices(insertionIdx);

    parent->AddStructureIndex(insertionIdx);

    std::visit([&](auto& basicStructure) {
        basicStructure.SetParentIdx(parentIdx);
        Structures.emplace(std::next(Structures.begin(), insertionIdx), std::move(basicStructure));
    }, basicStructureVariant);

    EmitEvent({ CtStructureTreeEventType::Add, insertionIdx });
}

auto CtStructureTree::AddBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant, CombinedStructure* parent) -> void {
    BasicStructureVariant basicStructureVariant = BasicStructure::CreateBasicStructure(basicStructureDataVariant);

    AddBasicStructure(std::move(basicStructureVariant), parent);
}

void CtStructureTree::CombineWithBasicStructure(BasicStructureVariant&& basicStructureVariant, CombinedStructure&& combinedStructure) {
    if (!HasRoot())
        throw std::runtime_error("No root is present yet. Cannot combine.");

    if (CtStructureExists(basicStructureVariant) || CtStructureExists(combinedStructure))
        throw std::runtime_error("CT Structure already exists. Cannot combine existing Structure.");

    StructureIdx combinedInsertionIdx = 0;
    StructureIdx basicInsertionIdx = 1;

    IncrementParentAndChildIndices(combinedInsertionIdx);
    IncrementParentAndChildIndices(basicInsertionIdx);

    combinedStructure.AddStructureIndex(basicInsertionIdx);
    combinedStructure.AddStructureIndex(RootIdx);

    combinedStructure.SetParentIdx(-1);
    StructureIdx newRootIdx = 0;
    std::visit([=](auto& basicStructure) { basicStructure.SetParentIdx(newRootIdx); }, basicStructureVariant);
    std::visit([=](auto& structure)      { structure.SetParentIdx(newRootIdx); },      Structures[0]);

    RootIdx = newRootIdx;

    Structures.emplace(std::next(Structures.begin(), combinedInsertionIdx), std::move(combinedStructure));
    std::visit([&](auto& basicStructure) { Structures.emplace(std::next(Structures.begin(), basicInsertionIdx),
                                                                      std::move(basicStructure)); },
               basicStructureVariant);

    EmitEvent({ CtStructureTreeEventType::Add, combinedInsertionIdx });
    EmitEvent({ CtStructureTreeEventType::Add, basicInsertionIdx });
}

void CtStructureTree::CombineWithBasicStructure(const BasicStructureDataVariant& basicStructureDataVariant,
                                                const CombinedStructureData& combinedStructureData) {
    BasicStructureVariant basicStructureVariant
            = BasicStructure::CreateBasicStructure(basicStructureDataVariant);

    CombinedStructure combinedStructure;
    combinedStructure.SetData(combinedStructureData);

    CombineWithBasicStructure(std::move(basicStructureVariant),
                              std::move(combinedStructure));
}

void CtStructureTree::RefineWithBasicStructure(const BasicStructureDataVariant& newStructureDataVariant,
                                               const CombinedStructureData& combinedStructureData,
                                               StructureIdx structureToRefineIdx) {
    if (!HasRoot())
        throw std::runtime_error("No root is present yet. Cannot refine.");

    if (!StructureIdxExists(structureToRefineIdx))
        throw std::runtime_error("Index of structure to be refined does not exist.");

    BasicStructureVariant newBasicStructureVariant = BasicStructure::CreateBasicStructure(newStructureDataVariant);

    CombinedStructure combinedStructure;
    combinedStructure.SetData(combinedStructureData);

    auto& structureToRefineVariant = GetStructureAt(structureToRefineIdx);
    if (std::holds_alternative<CombinedStructure>(structureToRefineVariant))
        throw std::runtime_error("Structure to refine must not be a combined structure");

    StructureId parentIdx = std::visit([&](auto const& structure) { return GetParentIdxOf(structure); },
                                       structureToRefineVariant);

    StructureIdx combinedInsertionIdx = structureToRefineIdx;
    StructureIdx basicInsertionIdx = combinedInsertionIdx + 1;
    StructureIdx newRefinedIdx = combinedInsertionIdx + 2;

    IncrementParentAndChildIndices(combinedInsertionIdx);
    IncrementParentAndChildIndices(basicInsertionIdx);

    combinedStructure.AddStructureIndex(basicInsertionIdx);
    combinedStructure.AddStructureIndex(newRefinedIdx);

    combinedStructure.SetParentIdx(parentIdx);
    std::visit([=](auto& structure) { structure.SetParentIdx(combinedInsertionIdx); }, newBasicStructureVariant);
    std::visit([=](auto& structure) { structure.SetParentIdx(combinedInsertionIdx); }, structureToRefineVariant);

    Structures.emplace(std::next(Structures.begin(), combinedInsertionIdx), std::move(combinedStructure));
    std::visit([&](auto& structure) {
                    Structures.emplace(std::next(Structures.begin(), basicInsertionIdx), std::move(structure));
               }, newBasicStructureVariant);

    EmitEvent({ CtStructureTreeEventType::Add, combinedInsertionIdx });
    EmitEvent({ CtStructureTreeEventType::Add, basicInsertionIdx });

    if (parentIdx == -1) {
        RootIdx = 0;
        return;
    }

    std::visit(Overload { [](auto&) {},
                          [=](CombinedStructure& combined) { combined.ReplaceChild(newRefinedIdx, combinedInsertionIdx); } },
               Structures[parentIdx]);
}

void CtStructureTree::RemoveBasicStructure(StructureIdx removeIdx) {
    if (!StructureIdxExists(removeIdx))
        throw std::runtime_error("Given index of structure to remove does not exist. Cannot remove non-existing structure");

    auto& structureVariant = GetStructureAt(removeIdx);
    if (std::holds_alternative<CombinedStructure>(structureVariant))
        throw std::runtime_error("Structure to be removed must not be a combined structure");

    if (RootIdx == removeIdx) {
        Structures.erase(Structures.begin());
        RootIdx = -1;
        EmitEvent({ CtStructureTreeEventType::Remove, removeIdx });
        return;
    }

    const StructureId parentIdx = std::visit([&](auto const& basicStructure) { return GetParentIdxOf(basicStructure); },
                                             structureVariant);
    if (parentIdx < 0)
        throw std::runtime_error("Parent cannot be null");

    auto& parent = std::get<CombinedStructure>(Structures[parentIdx]);

    parent.RemoveStructureIndex(removeIdx);
    Structures.erase(std::next(Structures.begin(), removeIdx));
    DecrementParentAndChildIndices(removeIdx);
    EmitEvent({ CtStructureTreeEventType::Remove, static_cast<StructureIdx>(parentIdx) });

    if (parent.StructureCount() == 1) {
        const StructureId grandParentIdx = GetParentIdxOf(parent);
        const StructureIdx remainingIdx = parent.StructureIdxAt(0);

        if (grandParentIdx != -1) {
            auto& grandParent = std::get<CombinedStructure>(Structures[grandParentIdx]);
            grandParent.ReplaceChild(parentIdx, remainingIdx);
        }

        std::visit([=](auto& structure) { structure.SetParentIdx(grandParentIdx); },
                   Structures[remainingIdx]);
        Structures.erase(std::next(Structures.begin(), parentIdx));
        DecrementParentAndChildIndices(0);
        EmitEvent({ CtStructureTreeEventType::Remove, static_cast<StructureIdx>(parentIdx) });

        if (grandParentIdx == -1)
            RootIdx = 0;
    }

    MTime.Modified();
}

struct EvaluateImplicitStructures {

    auto operator() (const auto& basicStructure) const noexcept -> CtStructureBase::ModelingResult {
        return { basicStructure.FunctionValue(point), basicStructure.Tissue.CtNumber, basicStructure.Id };
    }

    auto operator() (const CombinedStructure& combinedStructure) const noexcept -> CtStructureBase::ModelingResult {
        Point transformedPoint = combinedStructure.GetTransformedPoint(point);

        std::vector<CtStructureBase::ModelingResult> results;
        const std::vector<StructureIdx>& childIndices = combinedStructure.GetChildIndices();
        results.reserve(childIndices.size());
        for (const auto childIdx : childIndices) {
            results.emplace_back(std::visit(EvaluateImplicitStructures{structures, transformedPoint},
                                            structures[childIdx]));
        }

        static auto compareModelingResults
                = [](const CtStructureBase::ModelingResult& a, const CtStructureBase::ModelingResult& b) {
            return a.FunctionValue < b.FunctionValue;
        };

        switch (combinedStructure.Operator) {
            case CtStructureBase::OperatorType::UNION:
                return *std::min_element(results.begin(), results.end(), compareModelingResults);

            case CtStructureBase::OperatorType::INTERSECTION:
                return *std::max_element(results.begin(), results.end(), compareModelingResults);

            case CtStructureBase::OperatorType::DIFFERENCE: {
                CtStructureBase::ModelingResult firstResult = results[0];

                firstResult.FunctionValue = std::reduce(std::next(results.begin()), results.end(),
                                                  firstResult.FunctionValue,
                                                  [](float resultFunctionValue,
                                                     const CtStructureBase::ModelingResult current) -> float {
                    float const currentFunctionValue = -current.FunctionValue;

                    return resultFunctionValue <= currentFunctionValue
                            ? resultFunctionValue
                            : currentFunctionValue;
                });

                return firstResult;
            }

            default: return {};
        }
    }

    const std::vector<StructureVariant>& structures;
    const Point& point;
};

CtStructureBase::ModelingResult CtStructureTree::FunctionValueAndRadiodensity(Point point) const {
    if (!HasRoot())
        throw std::runtime_error("Tree does not have a root. Cannot evaluate");

    return std::visit(EvaluateImplicitStructures{Structures, point}, GetRoot());
}

void CtStructureTree::SetData(StructureIdx structureIdx, const QVariant& data) {
    if (!StructureIdxExists(structureIdx))
        throw std::runtime_error("Cannot set data. Given structure idx does not exist.");

    StructureVariant& structureVariant = Structures[structureIdx];

    if (data.canConvert<CombinedStructureData>()) {
        auto combinedData = data.value<CombinedStructureData>();
        std::get<CombinedStructure>(structureVariant).SetData(combinedData);
    } else {
        auto basicDataVariant = data.value<BasicStructureDataVariant>();
        std::visit(Overload {
            [&](SphereData& sphereData) { std::get<SphereStructure>(structureVariant).SetData(sphereData); },
            [&](BoxData& boxData) { std::get<BoxStructure>(structureVariant).SetData(boxData); }
        }, basicDataVariant);
    }

    EmitEvent({ CtStructureTreeEventType::Edit, structureIdx });
}

auto CtStructureTree::HasRoot() const noexcept -> bool {
    return RootIdx != -1;
}

auto CtStructureTree::GetRoot() const -> const StructureVariant& {
    if (!HasRoot())
        throw std::runtime_error("Cannot get root. No root is present.");

    return Structures[RootIdx];
}

auto CtStructureTree::GetStructureAt(StructureIdx idx) const -> const StructureVariant& {
    return Structures.at(idx);
}

auto CtStructureTree::GetStructureAt(StructureIdx idx) -> StructureVariant& {
    return Structures.at(idx);
}

template<TCtStructure TStructure>
auto CtStructureTree::CtStructureExists(const TStructure& ctStructure) const -> bool {
    return std::any_of(Structures.begin(), Structures.end(),
                       [&](const auto& structure) {
        return std::holds_alternative<TStructure>(structure)
                && std::get<TStructure>(structure) == ctStructure;
    });
}

auto CtStructureTree::CtStructureExists(const BasicStructureVariant& basicStructureVariant) const -> bool {
    return std::visit([&](auto& basicStructure) { return CtStructureExists(basicStructure); },
                      basicStructureVariant);
}

void CtStructureTree::AddTreeEventCallback(CtStructureTree::TreeEventCallback&& treeEventCallback) {
    TreeEventCallbacks.emplace_back(treeEventCallback);
}

auto CtStructureTree::GetRootIdx() const noexcept -> StructureId {
    return RootIdx;
}

auto CtStructureTree::StructureCount() const noexcept -> StructureIdx {
    return Structures.size();
}

auto CtStructureTree::StructureIdxExists(StructureId idx) const noexcept -> bool {
    return idx >= 0 && idx < Structures.size();
}

auto CtStructureTree::EmitEvent(CtStructureTreeEvent event) noexcept -> void {
    MTime.Modified();

    for (const auto& callback: TreeEventCallbacks)
        callback(event);
}

template<TCtStructure TStructure>
auto CtStructureTree::FindIndexOf(const TStructure& ctStructure) const -> StructureIdx {
    for (StructureIdx i = 0; i < static_cast<StructureIdx>(Structures.size()); i++) {
        if (!std::holds_alternative<TStructure>(Structures[i]))
            continue;

        if (std::get<TStructure>(Structures[i]) == ctStructure)
            return i;
    }

    throw std::runtime_error("Given structure not within stored structures");
}

auto CtStructureTree::GetParentIdxOf(const TCtStructure auto& ctStructure) const -> StructureId {
    const StructureIdx childIdx = FindIndexOf(ctStructure);

    const StructureId parentIdx = std::visit([](TCtStructure auto const& structure) { return structure.GetParentIdx(); },
                                        Structures[childIdx]);

    return parentIdx;
}

auto CtStructureTree::IncrementParentAndChildIndices(StructureIdx startIdx) -> void {
    if (RootIdx >= startIdx)
        RootIdx++;

    auto incrementParentIndicesGreaterThanOrEqualToStart
            = [=](auto& structure) { if (structure.GetParentIdx() >= startIdx) structure.IncrementParentIdx(); };

    for (auto& structureVariant : Structures) {
        std::visit(incrementParentIndicesGreaterThanOrEqualToStart, structureVariant);
        std::visit(Overload{
            [=](CombinedStructure& combinedStructure) { combinedStructure.UpdateChildIndicesGreaterThanOrEqualToBy(startIdx, +1); },
            [](auto& basicStructure) {},
        }, structureVariant);
    }
}

auto CtStructureTree::DecrementParentAndChildIndices(StructureIdx startIdx) -> void {
    if (RootIdx >= startIdx)
        RootIdx--;

    auto decrementParentIndicesGreaterThanOrEqualToStart
            = [=](auto& structure) { if (structure.GetParentIdx() >= startIdx) structure.DecrementParentIdx(); };

    for (auto& structureVariant : Structures) {
        std::visit(decrementParentIndicesGreaterThanOrEqualToStart, structureVariant);
        std::visit(Overload{
                [=](CombinedStructure& combinedStructure) {
                    combinedStructure.UpdateChildIndicesGreaterThanOrEqualToBy(startIdx, -1); },
                [](auto& basicStructure) {},
        }, structureVariant);
    }
}

