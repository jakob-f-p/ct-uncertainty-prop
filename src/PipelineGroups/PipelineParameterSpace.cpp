#include "PipelineParameterSpace.h"

auto PipelineParameterSpanSet::AddParameterSpan(ParameterSpanVariant&& spanVariant) -> ParameterSpanVariant& {
    return ParameterSpans.emplace_back(std::move(spanVariant));
}

auto PipelineParameterSpanSet::RemoveParameterSpan(ParameterSpanVariant const& spanVariant) -> void {
    auto it = std::find_if(ParameterSpans.begin(), ParameterSpans.end(),
                 [&spanVariant](auto& variant) {
        return std::visit([&spanVariant](auto const& span) {
            using SpanType = std::decay_t<decltype(span)>;

            return std::holds_alternative<SpanType>(spanVariant)
                    && span == std::get<SpanType>(spanVariant);
        }, variant);
    });

    if (it == ParameterSpans.end())
        throw std::runtime_error("Cannot remove given parameter span because it does not exist.");

    ParameterSpans.erase(it);
}

auto PipelineParameterSpace::AddParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                              ParameterSpanVariant&& parameterSpan) -> ParameterSpanVariant& {
    bool const isNullptr = std::visit([](auto artifactP) { return artifactP == nullptr; },
                                      artifactVariantPointer);
    if (isNullptr)
        throw std::runtime_error("Given artifact pointer must not be nullptr");

    auto& parameterSpanSet = ArtifactParametersMap.at(artifactVariantPointer);
    return parameterSpanSet.AddParameterSpan(std::move(parameterSpan));
}

auto PipelineParameterSpace::RemoveParameterSpan(ArtifactVariantPointer artifactVariantPointer,
                                                 ParameterSpanVariant const& parameterSpan) -> void {
    auto& parameterSpanSet = ArtifactParametersMap.at(artifactVariantPointer);

    parameterSpanSet.RemoveParameterSpan(parameterSpan);
}

auto PipelineParameterSpace::AddParameterSpanSet(ArtifactVariantPointer artifactVariantPointer)
        -> PipelineParameterSpanSet& {
    if (ArtifactParametersMap.contains(artifactVariantPointer))
        throw std::runtime_error("Cannot add parameter span set for given variant pointer."
                                 "A span set already exists.");

    auto [ it, successful] = ArtifactParametersMap.emplace(artifactVariantPointer, PipelineParameterSpanSet());
    if (!successful)
        throw std::runtime_error("Could not add parameter span set");

    return it->second;
}

auto PipelineParameterSpace::RemoveParameterSpanSet(ArtifactVariantPointer artifactVariantPointer) -> void {
    if (!ArtifactParametersMap.contains(artifactVariantPointer))
        throw std::runtime_error("Cannot remove parameter span set for given variant pointer. No span set exists.");

    auto numberOfRemovedElements = ArtifactParametersMap.erase(artifactVariantPointer);
    if (numberOfRemovedElements == 0)
        throw std::runtime_error("Could not remove parameter span set");
}

auto PipelineParameterSpace::HasParameterSpanSet(ArtifactVariantPointer artifactVariantPointer) const noexcept -> bool {
    return ArtifactParametersMap.contains(artifactVariantPointer);
}
