#include "CtStructureTree.h"

#include "../Utils/Overload.h"

#include "BasicStructure.h"
#include "CombinedStructure.h"

auto CtStructureTree::GetMTime() const -> vtkMTimeType {
    vtkMTimeType const nodeMTime = HasRoot()
            ? std::visit([](auto const& root) { return root.GetMTime(); }, Structures[*RootIdx])
            : 0;

    return std::max(MTime.GetMTime(), nodeMTime);
}

void CtStructureTree::AddBasicStructure(BasicStructure&& basicStructure, CombinedStructure* parent) {
    if (!parent) {  // add as root
        if (HasRoot()) {
            throw std::runtime_error("Another root is already present. Cannot add implicit Structure.");
        }

        basicStructure.ParentIdx = std::nullopt;
        Structures.emplace_back(std::move(basicStructure));
        RootIdx = 0;
        EmitEvent({ CtStructureTreeEventType::ADD, 0 });

        MTime.Modified();
        return;
    }

    // add as child
    if (CtStructureExists(basicStructure))
        throw std::runtime_error("CT Structure already exists. Cannot add existing Structure.");

    uidx_t const parentIdx = FindIndexOf(*parent);
    uidx_t const insertionIdx = parentIdx + 1;

    IncrementParentAndChildIndices(insertionIdx);

    parent->AddStructureIndex(insertionIdx);

    basicStructure.ParentIdx = parentIdx;
    Structures.emplace(std::next(Structures.begin(), insertionIdx), std::move(basicStructure));

    EmitEvent({ CtStructureTreeEventType::ADD, insertionIdx });
}

auto CtStructureTree::AddBasicStructure(BasicStructureData const& basicStructureData,
                                        CombinedStructure* parent) -> void {
    BasicStructure basicStructure(basicStructureData);

    AddBasicStructure(std::move(basicStructure), parent);
}

void CtStructureTree::CombineWithBasicStructure(BasicStructure&& basicStructure, CombinedStructure&& combinedStructure) {
    if (!HasRoot())
        throw std::runtime_error("No root is present yet. Cannot combine.");

    if (CtStructureExists(basicStructure) || CtStructureExists(combinedStructure))
        throw std::runtime_error("CT Structure already exists. Cannot combine existing Structure.");

    uidx_t const combinedInsertionIdx = 0;
    uidx_t const basicInsertionIdx = 1;

    IncrementParentAndChildIndices(combinedInsertionIdx);
    IncrementParentAndChildIndices(basicInsertionIdx);

    combinedStructure.AddStructureIndex(basicInsertionIdx);
    combinedStructure.AddStructureIndex(*RootIdx);

    combinedStructure.ParentIdx = std::nullopt;
    uidx_t newRootIdx = 0;
    basicStructure.ParentIdx = newRootIdx;
    std::visit([=](auto& structure) { structure.ParentIdx = newRootIdx; }, Structures[0]);

    RootIdx = newRootIdx;

    Structures.emplace(std::next(Structures.begin(), combinedInsertionIdx),
                       std::move(combinedStructure));
    Structures.emplace(std::next(Structures.begin(), basicInsertionIdx),
                       std::move(basicStructure));

    EmitEvent({ CtStructureTreeEventType::ADD, combinedInsertionIdx });
    EmitEvent({ CtStructureTreeEventType::ADD, basicInsertionIdx });
}

void CtStructureTree::CombineWithBasicStructure(BasicStructureData const& basicStructureData,
                                                CombinedStructureData const& combinedStructureData) {
    BasicStructure basicStructure(basicStructureData);

    CombinedStructure combinedStructure;
    combinedStructureData.PopulateStructure(combinedStructure);

    CombineWithBasicStructure(std::move(basicStructure), std::move(combinedStructure));
}

auto CtStructureTree::RefineWithBasicStructure(BasicStructure&& basicStructure,
                                               CombinedStructure&& combinedStructure,
                                               uidx_t structureToRefineIdx) -> void {
    if (!HasRoot())
        throw std::runtime_error("No root is present yet. Cannot refine.");

    if (!StructureIdxExists(structureToRefineIdx))
        throw std::runtime_error("Index of structure to be refined does not exist.");

    auto& structureToRefine = std::get<BasicStructure>(GetStructureAt(structureToRefineIdx));
    idx_t const parentIdx = GetParentIdxOf(structureToRefine);

    uidx_t const combinedInsertionIdx = structureToRefineIdx;
    uidx_t const basicInsertionIdx = combinedInsertionIdx + 1;
    uidx_t const newRefinedIdx = combinedInsertionIdx + 2;

    IncrementParentAndChildIndices(combinedInsertionIdx);
    IncrementParentAndChildIndices(basicInsertionIdx);

    combinedStructure.AddStructureIndex(basicInsertionIdx);
    combinedStructure.AddStructureIndex(newRefinedIdx);

    combinedStructure.ParentIdx = parentIdx;
    basicStructure.ParentIdx = combinedInsertionIdx;
    structureToRefine.ParentIdx = combinedInsertionIdx;

    Structures.emplace(std::next(Structures.begin(), combinedInsertionIdx), std::move(combinedStructure));
    Structures.emplace(std::next(Structures.begin(), basicInsertionIdx), std::move(basicStructure));

    EmitEvent({ CtStructureTreeEventType::ADD, combinedInsertionIdx });
    EmitEvent({ CtStructureTreeEventType::ADD, basicInsertionIdx });

    if (!parentIdx) {
        RootIdx = 0;
        return;
    }

    auto& parent = std::get<CombinedStructure>(Structures[*parentIdx]);
    parent.ReplaceChild(newRefinedIdx, combinedInsertionIdx);
}

void CtStructureTree::RefineWithBasicStructure(BasicStructureData const& newStructureData,
                                               CombinedStructureData const& combinedStructureData,
                                               uidx_t structureToRefineIdx) {
    BasicStructure newBasicStructure(newStructureData);

    CombinedStructure combinedStructure;
    combinedStructureData.PopulateStructure(combinedStructure);

    RefineWithBasicStructure(std::move(newBasicStructure), std::move(combinedStructure), structureToRefineIdx);
}

void CtStructureTree::RemoveBasicStructure(uidx_t removeIdx) {
    if (!StructureIdxExists(removeIdx))
        throw std::runtime_error("Given index of structure to remove does not exist. "
                                 "Cannot remove non-existing structure");

    auto& structure = std::get<BasicStructure>(GetStructureAt(removeIdx));

    if (RootIdx == removeIdx) {
        Structures.erase(Structures.begin());
        RootIdx = std::nullopt;
        EmitEvent({ CtStructureTreeEventType::REMOVE, removeIdx });
        return;
    }

    idx_t const parentIdxOptional = GetParentIdxOf(structure);
    if (!parentIdxOptional)
        throw std::runtime_error("Parent cannot be nullptr");

    uidx_t const parentIdx = *parentIdxOptional;
    auto& parent = std::get<CombinedStructure>(Structures[parentIdx]);

    parent.RemoveStructureIndex(removeIdx);
    Structures.erase(std::next(Structures.begin(), removeIdx));
    DecrementParentAndChildIndices(removeIdx);
    EmitEvent({ CtStructureTreeEventType::REMOVE, removeIdx });

    if (parent.StructureCount() == 1) {
        const idx_t grandParentIdx = GetParentIdxOf(parent);
        const uidx_t remainingIdx = parent.StructureIdxAt(0);

        if (grandParentIdx) {
            auto& grandParent = std::get<CombinedStructure>(Structures[*grandParentIdx]);
            grandParent.ReplaceChild(parentIdx, remainingIdx);
        } else {
            RootIdx = remainingIdx;
        }

        std::visit([=](auto& structure) { structure.ParentIdx = grandParentIdx; },
                   Structures[remainingIdx]);
        Structures.erase(std::next(Structures.begin(), parentIdx));
        DecrementParentAndChildIndices(parentIdx);
        EmitEvent({ CtStructureTreeEventType::REMOVE, static_cast<uidx_t>(parentIdx) });
    }

    MTime.Modified();
}

struct EvaluateImplicitStructures {
    using ModelingResult = CtStructureTree::ModelingResult;

    auto operator() (const BasicStructure& basicStructure) const noexcept -> ModelingResult {
        return { basicStructure.FunctionValue(basicStructure.GetTransformedPoint(point)),
                 basicStructure.Tissue.Radiodensity,
                 basicStructure.Id };
    }

    auto operator() (const CombinedStructure& combinedStructure) const noexcept -> ModelingResult {
        const Point transformedPoint = combinedStructure.GetTransformedPoint(point);

        std::vector<ModelingResult> evals;
        const std::vector<uidx_t>& childIndices = combinedStructure.GetChildIndices();
        evals.reserve(childIndices.size());
        for (const auto childIdx : childIndices) {
            evals.emplace_back(std::visit(EvaluateImplicitStructures{ structures, transformedPoint},
                                          structures[childIdx]));
        }

        static constexpr auto compareModelingResults
                = [](ModelingResult const& a, ModelingResult const& b) {
            return a.FunctionValue < b.FunctionValue;
        };

        switch (combinedStructure.Operator) {
            case CombinedStructure::OperatorType::UNION:
                return *std::min_element(evals.begin(), evals.end(), compareModelingResults);

            case CombinedStructure::OperatorType::INTERSECTION:
                return *std::max_element(evals.begin(), evals.end(), compareModelingResults);

            case CombinedStructure::OperatorType::DIFFERENCE_: {
                ModelingResult result = std::reduce(std::next(evals.begin()), evals.end(),
                                                    evals[0],
                                                    [](ModelingResult const res, ModelingResult const eval) {
                    return ModelingResult { res.FunctionValue - eval.FunctionValue,
                                            eval.FunctionValue < 0.0F
                                                    ? res.Radiodensity - eval.Radiodensity
                                                    : res.Radiodensity,
                                            res.BasicCtStructureId };
                });

                if (result.Radiodensity == 0.0F)
                    result.BasicCtStructureId = -1;

                return result;
            }

            default: return {};
        }
    }

    const std::vector<CtStructureVariant>& structures;
    const Point& point;
};

auto CtStructureTree::FunctionValueAndRadiodensity(Point point,
                                                   CtStructureVariant const* structure) const -> ModelingResult {
    if (!HasRoot() || (structure && !std::visit([&](auto const& s) { return CtStructureExists(s); }, *structure)))
        throw std::runtime_error("TreeArtifacts does not contain structure. Cannot evaluate");

    if (structure)
        point = TransformPointUntilStructure(point, *structure);

    return std::visit(EvaluateImplicitStructures { Structures, point }, structure ? *structure : GetRoot());
}

struct EvaluateFunctionValue {
    auto operator() (BasicStructure const& basicStructure) const noexcept -> float {
        return basicStructure.FunctionValue(basicStructure.GetTransformedPoint(point));
    }

    auto operator() (CombinedStructure const& combinedStructure) const noexcept -> float {
        const Point transformedPoint = combinedStructure.GetTransformedPoint(point);

        std::vector<float> results;
        std::vector<uidx_t> const& childIndices = combinedStructure.GetChildIndices();
        results.reserve(childIndices.size());
        for (const auto childIdx : childIndices)
            results.emplace_back(std::visit(EvaluateFunctionValue{structures, transformedPoint}, structures[childIdx]));

        switch (combinedStructure.Operator) {
            case CombinedStructure::OperatorType::UNION:
                return *std::min_element(results.begin(), results.end());

            case CombinedStructure::OperatorType::INTERSECTION:
                return *std::max_element(results.begin(), results.end());

            case CombinedStructure::OperatorType::DIFFERENCE_: {
                return std::reduce(std::next(results.begin()), results.end(),
                                   results[0],
                                   std::minus{});
            }

            default: return {};
        }
    }

    const std::vector<CtStructureVariant>& structures;
    const Point& point;
};

auto CtStructureTree::FunctionValue(Point point, CtStructureVariant const& structure) const -> float {
    point = TransformPointUntilStructure(point, structure);

    return std::visit(EvaluateFunctionValue{Structures, point}, structure);
}

struct FindClosestPointOnXYPlane {
    auto operator() (BasicStructure const& basicStructure) const noexcept -> std::optional<DoublePoint> {
        auto point = basicStructure.ClosestPointOnXYPlane(basicStructure.GetTransformedPoint(Point_));
        return point
                ? std::optional<DoublePoint> { basicStructure.GetInverselyTransformedPoint(*point) }
                : std::nullopt;
    }

    auto operator() (CombinedStructure const& combinedStructure) const noexcept -> std::optional<DoublePoint> {
        const Point transformedPoint = combinedStructure.GetTransformedPoint(Point_);

        std::vector<DoublePoint> points;
        std::vector<uidx_t> const& childIndices = combinedStructure.GetChildIndices();
        points.reserve(childIndices.size());
        for (const auto childIdx : childIndices) {
            auto const& closestPoint = std::visit(FindClosestPointOnXYPlane { Structures, transformedPoint },
                                                  Structures[childIdx]);
            if (closestPoint)
                points.emplace_back(std::move(*closestPoint));
        }

        if (points.empty())
            return std::nullopt;

        std::transform(points.cbegin(), points.cend(),
                       points.begin(),
                       [&](DoublePoint const& point) { return combinedStructure.GetInverselyTransformedPoint(point); });

        static const auto compareDistances = [this](DoublePoint const& a, DoublePoint const& b) {
            return VectorDistance(a, Point_) < VectorDistance(b, Point_);
        };

        auto const it = [&]() {
            switch (combinedStructure.Operator) {
                case CombinedStructure::OperatorType::UNION:
                    return std::min_element(points.begin(), points.end(), compareDistances);

                case CombinedStructure::OperatorType::INTERSECTION:
                    return std::max_element(points.begin(), points.end(), compareDistances);

                case CombinedStructure::OperatorType::DIFFERENCE_:
                    qWarning("todo");
                    return std::min_element(points.begin(), points.end(), compareDistances);

                default:
                    return points.end();
            }
        }();

        return it == points.end()
                ? std::nullopt
                : std::optional<DoublePoint> { *it };
    }

    const std::vector<CtStructureVariant>& Structures;
    const Point& Point_;
};

auto CtStructureTree::ClosestPointOnXYPlane(Point const& point,
                                            CtStructureVariant const& structure) const -> std::optional<DoublePoint> {
    Point const& transformedPoint = TransformPointUntilStructure(point, structure);

    auto const& closestPoint = std::visit(FindClosestPointOnXYPlane { Structures, transformedPoint }, structure);
//    auto const& closestPoint = std::visit(FindClosestPointOnXYPlane { Structures, point }, structure);

    return closestPoint
            ? std::optional<DoublePoint> { TransformPointFromStructure(*closestPoint, structure) }
            : std::nullopt;
//    return closestPoint
//           ? std::optional<DoublePoint> { closestPoint }
//           : std::nullopt;
}

void CtStructureTree::SetData(uidx_t structureIdx, QVariant const& data) {
    if (!StructureIdxExists(structureIdx))
        throw std::runtime_error("Cannot set data. Given structure idx does not exist.");

    CtStructureVariant& structureVariant = Structures[structureIdx];

    if (data.canConvert<CombinedStructureData>()) {
        auto combinedData = data.value<CombinedStructureData>();
        combinedData.PopulateStructure(std::get<CombinedStructure>(structureVariant));
    } else {
        auto basicData = data.value<BasicStructureData>();
        basicData.PopulateStructure(std::get<BasicStructure>(structureVariant));
    }

    EmitEvent({ CtStructureTreeEventType::EDIT, structureIdx });
}

auto CtStructureTree::HasRoot() const noexcept -> bool {
    return static_cast<bool>(RootIdx);
}

auto CtStructureTree::GetRoot() const -> CtStructureVariant const& {
    if (!HasRoot())
        throw std::runtime_error("Cannot get root. No root is present.");

    return Structures[*RootIdx];
}

auto CtStructureTree::GetRoot() -> CtStructureVariant& {
    if (!HasRoot())
        throw std::runtime_error("Cannot get root. No root is present.");

    return Structures[*RootIdx];
}

auto CtStructureTree::GetStructureAt(uidx_t idx) const -> const CtStructureVariant& {
    return Structures.at(idx);
}

auto CtStructureTree::GetStructureAt(uidx_t idx) -> CtStructureVariant& {
    return Structures.at(idx);
}

auto CtStructureTree::CtStructureExists(auto const& ctStructure) const -> bool {
    using TStructure = std::decay_t<decltype(ctStructure)>;

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

struct AddBasicStructureIndices {
    auto operator() (BasicStructure const& basicStructure) const noexcept -> void {
        StructureIds.push_back(basicStructure.GetId());
    }

    auto operator() (CombinedStructure const& combinedStructure) const noexcept -> void {
        for (const auto childIdx : combinedStructure.ChildStructureIndices)
            std::visit(AddBasicStructureIndices { StructureIds, Structures }, Structures[childIdx]);
    }

    std::vector<StructureId>& StructureIds;
    std::vector<CtStructureVariant> const& Structures;
};

auto CtStructureTree::GetBasicStructureIdsOfStructureAt(uidx_t idx) const -> std::vector<StructureId> {
    return GetBasicStructureIds(Structures.at(idx));
}

auto
CtStructureTree::GetBasicStructureIds(CtStructureVariant const& structure) const -> std::vector<StructureId> {
    if (auto it = std::find(Structures.cbegin(), Structures.cend(), structure);
        it == Structures.cend())
        throw std::runtime_error("Structure does not exist");

    std::vector<StructureId> structureIds {};

    std::visit(AddBasicStructureIndices { structureIds, Structures }, structure);

    return structureIds;
}
struct MaxTissueValueAlgorithm {
    auto operator() (BasicStructure const& basicStructure) const noexcept -> float {
        return basicStructure.GetTissueValue();
    }

    auto operator() (CombinedStructure const& combinedStructure) const noexcept -> float {
        std::vector<float> tissueValues {};
        for (const auto childIdx : combinedStructure.ChildStructureIndices) {
            float const value = std::visit(MaxTissueValueAlgorithm { Structures }, Structures[childIdx]);
            tissueValues.emplace_back(value);
        }

        return *std::max_element(tissueValues.begin(), tissueValues.end());
    }

    std::vector<CtStructureVariant> const& Structures;
};

auto CtStructureTree::GetMaxTissueValue(CtStructureVariant const& structure) const -> float {
    return std::visit(MaxTissueValueAlgorithm { Structures }, structure);
}

auto CtStructureTree::StructureCount() const noexcept -> uidx_t {
    return Structures.size();
}

auto CtStructureTree::StructureIdxExists(uidx_t idx) const noexcept -> bool {
    return idx < Structures.size();
}

auto CtStructureTree::EmitEvent(CtStructureTreeEvent event) noexcept -> void {
    MTime.Modified();

    for (const auto& callback: TreeEventCallbacks)
        callback(event);
}

auto CtStructureTree::FindIndexOf(auto const& ctStructure) const -> uidx_t {
    using TStructure = std::decay_t<decltype(ctStructure)>;

    for (uidx_t i = 0; i < static_cast<uidx_t>(Structures.size()); i++) {
        if (!std::holds_alternative<TStructure>(Structures[i]))
            continue;

        if (std::get<TStructure>(Structures[i]) == ctStructure)
            return i;
    }

    throw std::runtime_error("Given structure not within stored structures");
}

auto CtStructureTree::GetParentIdxOf(const auto& ctStructure) const -> idx_t {
    const uidx_t childIdx = FindIndexOf(ctStructure);

    const idx_t parentIdx = std::visit([](auto const& structure) { return structure.ParentIdx; },
                                        Structures[childIdx]);

    return parentIdx;
}

auto CtStructureTree::IncrementParentAndChildIndices(uidx_t startIdx) -> void {
    if (RootIdx >= startIdx)
        RootIdx++;

    auto incrementParentIndicesGreaterThanOrEqualToStart
            = [=](auto& structure) { if (structure.ParentIdx >= startIdx) structure.ParentIdx++; };

    for (auto& structureVariant : Structures) {
        std::visit(incrementParentIndicesGreaterThanOrEqualToStart, structureVariant);
        std::visit(Overload {
            [=](CombinedStructure& combinedStructure) {
                combinedStructure.UpdateChildIndicesGreaterThanOrEqualToBy(startIdx, +1);
                },
            [](BasicStructure&) {},
        }, structureVariant);
    }
}

auto CtStructureTree::DecrementParentAndChildIndices(uidx_t startIdx) -> void {
    if (RootIdx >= startIdx)
        RootIdx--;

    auto decrementParentIndicesGreaterThanOrEqualToStart
            = [=](auto& structure) { if (structure.ParentIdx >= startIdx) structure.ParentIdx--; };

    for (auto& structureVariant : Structures) {
        std::visit(decrementParentIndicesGreaterThanOrEqualToStart, structureVariant);
        std::visit(Overload {
                [=](CombinedStructure& combinedStructure) {
                    combinedStructure.UpdateChildIndicesGreaterThanOrEqualToBy(startIdx, -1); },
                [](BasicStructure&) {},
        }, structureVariant);
    }
}

