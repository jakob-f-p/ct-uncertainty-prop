#include "PipelineParameterSpace.h"

#include "PipelineParameterSpan.h"

#include <numeric>
#include <ranges>


PipelineParameterSpanSet::PipelineParameterSpanSet(ArtifactVariantPointer artifactVariantPointer) :
        ArtifactPointer(std::move(artifactVariantPointer)) {}

auto PipelineParameterSpanSet::AddParameterSpan(PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan& {
    return ParameterSpans.emplace_back(std::move(parameterSpan));
}

auto PipelineParameterSpanSet::RemoveParameterSpan(PipelineParameterSpan const& parameterSpan) -> void {
    auto it = std::find(ParameterSpans.begin(), ParameterSpans.end(), parameterSpan);

    if (it == ParameterSpans.end())
        throw std::runtime_error("Cannot remove given parameter span because it does not exist.");

    ParameterSpans.erase(it);
}

auto PipelineParameterSpanSet::GetName() const noexcept -> std::string {
    return ArtifactPointer.GetName();
}

auto PipelineParameterSpanSet::GetArtifactPointer() const noexcept -> ArtifactVariantPointer {
    return ArtifactPointer;
}

auto PipelineParameterSpanSet::GetSize() const noexcept -> uint16_t {
    return ParameterSpans.size();
}

auto PipelineParameterSpanSet::Get(uint16_t idx) -> PipelineParameterSpan& {
    return ParameterSpans.at(idx);
}

auto PipelineParameterSpanSet::GetIdx(PipelineParameterSpan const& parameterSpan) const -> uint16_t {
    auto it = std::find(ParameterSpans.cbegin(), ParameterSpans.cend(), parameterSpan);

    if (it == ParameterSpans.cend())
        throw std::runtime_error("Given parameter span not found");

    return std::distance(ParameterSpans.cbegin(), it);
}

auto PipelineParameterSpanSet::GetNumberOfPipelines() const noexcept -> uint16_t {
    return std::transform_reduce(ParameterSpans.cbegin(), ParameterSpans.cend(), 1, std::multiplies{},
                                 [](auto const& span) { return span.GetNumberOfPipelines(); });
}

auto PipelineParameterSpanSet::SpanStatesProduct() -> std::vector<ParameterSpanSetState> {
    std::vector<ParameterSpanSetState> spanSetStates;
    spanSetStates.reserve(GetNumberOfPipelines());

    if (ParameterSpans.empty())
        return spanSetStates;

    std::vector<size_t> indices (ParameterSpans.size(), 0);

    // TODO: remove
    return spanSetStates;
//    while (true) {
//        std::string current;
//        for (size_t i = 0; i < ParameterSpans.size(); ++i) {
//            current += ParameterSpans[i][indices[i]];
//        }
//        result.push_back(current);
//
//        // Increment the indices from the last set to the first
//        size_t setIndex = sets.size();
//        while (setIndex > 0) {
//            --setIndex;
//            if (indices[setIndex] + 1 < sets[setIndex].size()) {
//                // Move to the next element in the current set
//                ++indices[setIndex];
//                break;
//            } else {
//                // Reset the current set and move to the next one
//                indices[setIndex] = 0;
//            }
//        }
//
//        // If we've reset the first set, we're done
//        if (setIndex == 0 && indices[0] == 0) {
//            break;
//        }
//    }
//
//    return result;
//    for (auto& span : ParameterSpans) {
//    }
}

auto PipelineParameterSpanSet::operator==(const PipelineParameterSpanSet& other) const noexcept -> bool {
    return this == &other;
}


auto PipelineParameterSpace::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                              PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan& {
    if (artifactVariantPointer.IsNullptr())
        throw std::runtime_error("Given artifact pointer must not be nullptr");

    auto& parameterSpanSet = GetSetForArtifactPointer(artifactVariantPointer);
    return AddParameterSpan(parameterSpanSet, std::move(parameterSpan));
}

auto PipelineParameterSpace::AddParameterSpan(PipelineParameterSpanSet& spanSet,
                                              PipelineParameterSpan&& parameterSpan) -> PipelineParameterSpan& {
    if (std::find(ParameterSpanSets.cbegin(), ParameterSpanSets.cend(), spanSet) == ParameterSpanSets.cend())
        throw std::runtime_error("Span set does not exist in parameter space");

    return spanSet.AddParameterSpan(std::move(parameterSpan));
}

auto PipelineParameterSpace::RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                                 PipelineParameterSpan const& parameterSpan) -> void {
    RemoveParameterSpan(GetSetForArtifactPointer(artifactVariantPointer), parameterSpan);
}

auto PipelineParameterSpace::RemoveParameterSpan(PipelineParameterSpanSet& spanSet,
                                                 PipelineParameterSpan const& parameterSpan) -> void {
    spanSet.RemoveParameterSpan(parameterSpan);

    if (spanSet.GetSize() == 0) {
        auto it = std::find(ParameterSpanSets.cbegin(), ParameterSpanSets.cend(), spanSet);
        ParameterSpanSets.erase(it);
    }
}

auto PipelineParameterSpace::GetNumberOfSpans() const noexcept -> uint16_t {
    auto numbersOfSpans = ParameterSpanSets
                          | std::views::transform([](auto& set) { return set.GetSize(); });

    return std::reduce(numbersOfSpans.begin(), numbersOfSpans.end(), 0, std::plus{});
}

auto PipelineParameterSpace::GetNumberOfSpanSets() const noexcept -> uint16_t {
    return ParameterSpanSets.size();
}

auto PipelineParameterSpace::GetNumberOfPipelines() const noexcept -> uint16_t {
    return std::transform_reduce(ParameterSpanSets.cbegin(), ParameterSpanSets.cend(), 1, std::multiplies{},
                                 [](auto const& spanSet) { return spanSet.GetNumberOfPipelines(); });
}

auto PipelineParameterSpace::GetSpanSet(uint16_t idx) -> PipelineParameterSpanSet& {
    if (idx >= ParameterSpanSets.size())
        throw std::runtime_error("Span set index out of range");

    for (auto& spanSet : ParameterSpanSets) {
        if (idx == 0)
            return spanSet;

        idx--;
    }

    throw std::runtime_error("Span set not found");
}

auto PipelineParameterSpace::GetSpanSet(PipelineParameterSpan const& parameterSpan) -> PipelineParameterSpanSet& {
    for (auto& spanSet : ParameterSpanSets) {
        for (int i = 0; i < spanSet.GetSize(); i++) {
            auto& span = spanSet.Get(i);

            if (span == parameterSpan)
                return spanSet;
        }
    }

    throw std::runtime_error("Span set not found");
}

auto PipelineParameterSpace::GetSpanSetIdx(PipelineParameterSpanSet const& spanSet) const -> uint16_t {
    uidx_t idx = 0;

    for (auto const& set : ParameterSpanSets) {
        if (spanSet == set)
            return idx;

        idx++;
    }

    throw std::runtime_error("Span set not found");
}

auto PipelineParameterSpace::GetSpanSetName(PipelineParameterSpanSet const& spanSet) const -> std::string {
    auto it = std::find(ParameterSpanSets.cbegin(), ParameterSpanSets.cend(), spanSet);

    if (it == ParameterSpanSets.cend())
        throw std::runtime_error("Span set not found");

    return it->GetName();
}

auto PipelineParameterSpace::GenerateSpaceStates() -> std::vector<PipelineParameterSpaceState> {
    std::vector<PipelineParameterSpaceState> spaceStates;
    spaceStates.reserve(GetNumberOfPipelines());

    std::vector<PipelineParameterSpan*> spans;
    spans.reserve(GetNumberOfSpans());
    for (auto& spanSet : ParameterSpanSets)
        for (auto& span : spanSet.ParameterSpans)
            spans.push_back(&span);

    GenerateSpaceStatesRecursive(spans, spaceStates, 0);

    return spaceStates;
}

auto PipelineParameterSpace::ContainsSetForArtifactPointer(ArtifactVariantPointer artifactVariantPointer) const noexcept
        -> bool {

    auto it = std::find_if(ParameterSpanSets.cbegin(), ParameterSpanSets.cend(),
                           [&](auto const& set) { return set.ArtifactPointer == artifactVariantPointer; });

    return it != ParameterSpanSets.cend();
}

auto PipelineParameterSpace::GetSetForArtifactPointer(ArtifactVariantPointer artifactVariantPointer)
        -> PipelineParameterSpanSet& {

    if (!ContainsSetForArtifactPointer(artifactVariantPointer))
        ParameterSpanSets.emplace_back(artifactVariantPointer);

    auto it = std::find_if(ParameterSpanSets.begin(), ParameterSpanSets.end(),
                           [&](auto const& set) { return set.ArtifactPointer == artifactVariantPointer; });

    if (it == ParameterSpanSets.end())
        throw std::runtime_error("No set for given artifact pointer exists");

    return *it;
}

auto PipelineParameterSpace::GenerateSpaceStatesRecursive(std::vector<PipelineParameterSpan*> const& spans,
                                                          std::vector<PipelineParameterSpaceState>& states,
                                                          int depth) -> void {
    if (depth == spans.size()) {
        states.emplace_back(*this);
        return;
    }

    ParameterSpanStateSourceIterator it (*spans.at(depth));
    for (; it != it.End(); ++it)
        GenerateSpaceStatesRecursive(spans, states, depth + 1);
}
