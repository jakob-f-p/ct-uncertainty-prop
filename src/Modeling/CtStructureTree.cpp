#include "CtStructureTree.h"

#include "../Overload.h"

#include "BasicStructure.h"
#include "CombinedStructure.h"

auto CtStructureTree::GetMTime() const -> vtkMTimeType {
    vtkMTimeType const nodeMTime = HasRoot()
            ? std::visit([](auto const& root) { return root.GetMTime(); }, Structures[RootIdx])
            : 0;

    return std::max(MTime.GetMTime(), nodeMTime);
}

void CtStructureTree::AddBasicStructure(BasicStructure&& basicStructure, CombinedStructure* parent) {
    if (!parent) {  // add as root
        if (HasRoot()) {
            throw std::runtime_error("Another root is already present. Cannot add implicit Structure.");
        }

        basicStructure.SetParentIdx(-1);
        Structures.emplace_back(std::move(basicStructure));
        RootIdx = 0;
        EmitEvent({ CtStructureTreeEventType::Add, 0 });

        MTime.Modified();
        return;
    }

    // add as child
    if (CtStructureExists(basicStructure))
        throw std::runtime_error("CT Structure already exists. Cannot add existing Structure.");

    const uidx_t parentIdx = FindIndexOf(*parent);
    const uidx_t insertionIdx = parentIdx + 1;

    IncrementParentAndChildIndices(insertionIdx);

    parent->AddStructureIndex(insertionIdx);

    basicStructure.SetParentIdx(parentIdx);
    Structures.emplace(std::next(Structures.begin(), insertionIdx), std::move(basicStructure));

    EmitEvent({ CtStructureTreeEventType::Add, insertionIdx });
}

auto CtStructureTree::AddBasicStructure(const BasicStructureData& basicStructureData, CombinedStructure* parent) -> void {
    BasicStructure basicStructure(basicStructureData);

    AddBasicStructure(std::move(basicStructure), parent);
}

void CtStructureTree::CombineWithBasicStructure(BasicStructure&& basicStructure, CombinedStructure&& combinedStructure) {
    if (!HasRoot())
        throw std::runtime_error("No root is present yet. Cannot combine.");

    if (CtStructureExists(basicStructure) || CtStructureExists(combinedStructure))
        throw std::runtime_error("CT Structure already exists. Cannot combine existing Structure.");

    const uidx_t combinedInsertionIdx = 0;
    const uidx_t basicInsertionIdx = 1;

    IncrementParentAndChildIndices(combinedInsertionIdx);
    IncrementParentAndChildIndices(basicInsertionIdx);

    combinedStructure.AddStructureIndex(basicInsertionIdx);
    combinedStructure.AddStructureIndex(RootIdx);

    combinedStructure.SetParentIdx(-1);
    uidx_t newRootIdx = 0;
    basicStructure.SetParentIdx(newRootIdx);
    std::visit([=](auto& structure)      { structure.SetParentIdx(newRootIdx); },      Structures[0]);

    RootIdx = newRootIdx;

    Structures.emplace(std::next(Structures.begin(), combinedInsertionIdx), std::move(combinedStructure));
    Structures.emplace(std::next(Structures.begin(), basicInsertionIdx), std::move(basicStructure));

    EmitEvent({ CtStructureTreeEventType::Add, combinedInsertionIdx });
    EmitEvent({ CtStructureTreeEventType::Add, basicInsertionIdx });
}

void CtStructureTree::CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                                                const CombinedStructureData& combinedStructureData) {
    BasicStructure basicStructure(basicStructureData);

    CombinedStructure combinedStructure;
    combinedStructure.SetData(combinedStructureData);

    CombineWithBasicStructure(std::move(basicStructure), std::move(combinedStructure));
}

void CtStructureTree::RefineWithBasicStructure(const BasicStructureData& newStructureData,
                                               const CombinedStructureData& combinedStructureData,
                                               uidx_t structureToRefineIdx) {
    if (!HasRoot())
        throw std::runtime_error("No root is present yet. Cannot refine.");

    if (!StructureIdxExists(structureToRefineIdx))
        throw std::runtime_error("Index of structure to be refined does not exist.");

    BasicStructure newBasicStructure(newStructureData);

    CombinedStructure combinedStructure;
    combinedStructure.SetData(combinedStructureData);

    auto& structureToRefine = std::get<BasicStructure>(GetStructureAt(structureToRefineIdx));
    idx_t parentIdx = GetParentIdxOf(structureToRefine);

    uidx_t combinedInsertionIdx = structureToRefineIdx;
    uidx_t basicInsertionIdx = combinedInsertionIdx + 1;
    uidx_t newRefinedIdx = combinedInsertionIdx + 2;

    IncrementParentAndChildIndices(combinedInsertionIdx);
    IncrementParentAndChildIndices(basicInsertionIdx);

    combinedStructure.AddStructureIndex(basicInsertionIdx);
    combinedStructure.AddStructureIndex(newRefinedIdx);

    combinedStructure.SetParentIdx(parentIdx);
    newBasicStructure.SetParentIdx(combinedInsertionIdx);
    structureToRefine.SetParentIdx(combinedInsertionIdx);

    Structures.emplace(std::next(Structures.begin(), combinedInsertionIdx), std::move(combinedStructure));
    Structures.emplace(std::next(Structures.begin(), basicInsertionIdx), std::move(newBasicStructure));

    EmitEvent({ CtStructureTreeEventType::Add, combinedInsertionIdx });
    EmitEvent({ CtStructureTreeEventType::Add, basicInsertionIdx });

    if (parentIdx == -1) {
        RootIdx = 0;
        return;
    }

    auto& parent = std::get<CombinedStructure>(Structures[parentIdx]);
    parent.ReplaceChild(newRefinedIdx, combinedInsertionIdx);
}

void CtStructureTree::RemoveBasicStructure(uidx_t removeIdx) {
    if (!StructureIdxExists(removeIdx))
        throw std::runtime_error("Given index of structure to remove does not exist. Cannot remove non-existing structure");

    auto& structure = std::get<BasicStructure>(GetStructureAt(removeIdx));

    if (RootIdx == removeIdx) {
        Structures.erase(Structures.begin());
        RootIdx = -1;
        EmitEvent({ CtStructureTreeEventType::Remove, removeIdx });
        return;
    }

    const idx_t parentIdx = GetParentIdxOf(structure);
    if (parentIdx < 0)
        throw std::runtime_error("Parent cannot be nullptr");

    auto& parent = std::get<CombinedStructure>(Structures[parentIdx]);

    parent.RemoveStructureIndex(removeIdx);
    Structures.erase(std::next(Structures.begin(), removeIdx));
    DecrementParentAndChildIndices(removeIdx);
    EmitEvent({ CtStructureTreeEventType::Remove, removeIdx });

    if (parent.StructureCount() == 1) {
        const idx_t grandParentIdx = GetParentIdxOf(parent);
        const uidx_t remainingIdx = parent.StructureIdxAt(0);

        if (grandParentIdx != -1) {
            auto& grandParent = std::get<CombinedStructure>(Structures[grandParentIdx]);
            grandParent.ReplaceChild(parentIdx, remainingIdx);
        }

        std::visit([=](auto& structure) { structure.SetParentIdx(grandParentIdx); },
                   Structures[remainingIdx]);
        Structures.erase(std::next(Structures.begin(), parentIdx));
        DecrementParentAndChildIndices(parentIdx);
        EmitEvent({ CtStructureTreeEventType::Remove, static_cast<uidx_t>(parentIdx) });

        if (grandParentIdx == -1)
            RootIdx = 0;
    }

    MTime.Modified();
}

struct EvaluateImplicitStructures {
    using ModelingResult = CtStructureTree::ModelingResult;

    auto operator() (const auto& basicStructure) const noexcept -> ModelingResult {
        return { basicStructure.FunctionValue(point), basicStructure.Tissue.CtNumber, basicStructure.Id };
    }

    auto operator() (const CombinedStructure& combinedStructure) const noexcept -> ModelingResult {
        const Point transformedPoint = combinedStructure.GetTransformedPoint(point);

        std::vector<ModelingResult> results;
        const std::vector<uidx_t>& childIndices = combinedStructure.GetChildIndices();
        results.reserve(childIndices.size());
        for (const auto childIdx : childIndices) {
            results.emplace_back(std::visit(EvaluateImplicitStructures{structures, transformedPoint},
                                            structures[childIdx]));
        }

        static auto compareModelingResults
                = [](const ModelingResult& a, const ModelingResult& b) {
            return a.FunctionValue < b.FunctionValue;
        };

        switch (combinedStructure.Operator) {
            case OperatorType::UNION:
                return *std::min_element(results.begin(), results.end(), compareModelingResults);

            case OperatorType::INTERSECTION:
                return *std::max_element(results.begin(), results.end(), compareModelingResults);

            case OperatorType::DIFFERENCE: {
                ModelingResult firstResult = results[0];

                firstResult.FunctionValue = std::reduce(std::next(results.begin()), results.end(),
                                                  firstResult.FunctionValue,
                                                  [](float resultFunctionValue,
                                                     const ModelingResult current) -> float {
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

auto CtStructureTree::FunctionValueAndRadiodensity(Point point) const -> ModelingResult {
    if (!HasRoot())
        throw std::runtime_error("Tree does not have a root. Cannot evaluate");

    return std::visit(EvaluateImplicitStructures{Structures, point}, GetRoot());
}

void CtStructureTree::SetData(uidx_t structureIdx, const QVariant& data) {
    if (!StructureIdxExists(structureIdx))
        throw std::runtime_error("Cannot set data. Given structure idx does not exist.");

    StructureVariant& structureVariant = Structures[structureIdx];

    if (data.canConvert<CombinedStructureData>()) {
        auto combinedData = data.value<CombinedStructureData>();
        std::get<CombinedStructure>(structureVariant).SetData(combinedData);
    } else {
        auto basicData = data.value<BasicStructureData>();
        std::get<BasicStructure>(structureVariant).SetData(basicData);
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

auto CtStructureTree::GetStructureAt(uidx_t idx) const -> const StructureVariant& {
    return Structures.at(idx);
}

auto CtStructureTree::GetStructureAt(uidx_t idx) -> StructureVariant& {
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

void CtStructureTree::AddTreeEventCallback(CtStructureTree::TreeEventCallback&& treeEventCallback) {
    TreeEventCallbacks.emplace_back(std::move(treeEventCallback));
}

auto CtStructureTree::GetRootIdx() const noexcept -> idx_t {
    return RootIdx;
}

auto CtStructureTree::StructureCount() const noexcept -> uidx_t {
    return Structures.size();
}

auto CtStructureTree::StructureIdxExists(idx_t idx) const noexcept -> bool {
    return idx >= 0 && idx < Structures.size();
}

auto CtStructureTree::EmitEvent(CtStructureTreeEvent event) noexcept -> void {
    MTime.Modified();

    for (const auto& callback: TreeEventCallbacks)
        callback(event);
}

template<TCtStructure TStructure>
auto CtStructureTree::FindIndexOf(const TStructure& ctStructure) const -> uidx_t {
    for (uidx_t i = 0; i < static_cast<uidx_t>(Structures.size()); i++) {
        if (!std::holds_alternative<TStructure>(Structures[i]))
            continue;

        if (std::get<TStructure>(Structures[i]) == ctStructure)
            return i;
    }

    throw std::runtime_error("Given structure not within stored structures");
}

auto CtStructureTree::GetParentIdxOf(const TCtStructure auto& ctStructure) const -> idx_t {
    const uidx_t childIdx = FindIndexOf(ctStructure);

    const idx_t parentIdx = std::visit([](TCtStructure auto const& structure) { return structure.GetParentIdx(); },
                                        Structures[childIdx]);

    return parentIdx;
}

auto CtStructureTree::IncrementParentAndChildIndices(uidx_t startIdx) -> void {
    if (RootIdx >= startIdx)
        RootIdx++;

    auto incrementParentIndicesGreaterThanOrEqualToStart
            = [=](auto& structure) { if (structure.GetParentIdx() >= startIdx) structure.IncrementParentIdx(); };

    for (auto& structureVariant : Structures) {
        std::visit(incrementParentIndicesGreaterThanOrEqualToStart, structureVariant);
        std::visit(Overload {
            [=](CombinedStructure& combinedStructure) { combinedStructure.UpdateChildIndicesGreaterThanOrEqualToBy(startIdx, +1); },
            [](BasicStructure&) {},
        }, structureVariant);
    }
}

auto CtStructureTree::DecrementParentAndChildIndices(uidx_t startIdx) -> void {
    if (RootIdx >= startIdx)
        RootIdx--;

    auto decrementParentIndicesGreaterThanOrEqualToStart
            = [=](auto& structure) { if (structure.GetParentIdx() >= startIdx) structure.DecrementParentIdx(); };

    for (auto& structureVariant : Structures) {
        std::visit(decrementParentIndicesGreaterThanOrEqualToStart, structureVariant);
        std::visit(Overload {
                [=](CombinedStructure& combinedStructure) {
                    combinedStructure.UpdateChildIndicesGreaterThanOrEqualToBy(startIdx, -1); },
                [](BasicStructure&) {},
        }, structureVariant);
    }
}

