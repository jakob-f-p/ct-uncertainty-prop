#include "ParameterSpaceState.h"

#include "../Utils/Overload.h"
#include "PipelineParameterSpace.h"

ParameterSpanState::ParameterSpanState(PipelineParameterSpan& parameterSpan) :
        SpanState([&parameterSpan]() {
            return std::visit(Overload {
                [](ParameterSpan<float>& span)      -> SpanStateVariant { return ::SpanState<float>(span); },
                [](ParameterSpan<FloatPoint>& span) -> SpanStateVariant { return ::SpanState<FloatPoint>(span); }
            }, parameterSpan.SpanVariant);
        }()) {}

auto ParameterSpanState::Apply() const noexcept -> void {
    std::visit([](auto const& spanState) { spanState.Apply(); }, SpanState);
}


ParameterSpanSetState::ParameterSpanSetState(PipelineParameterSpanSet& parameterSpanSet) :
        SpanSet(parameterSpanSet),
        States([&parameterSpanSet]() {
            SpanStates spanStates;
            spanStates.reserve(parameterSpanSet.GetSize());

            for (auto& span : parameterSpanSet.ParameterSpans)
                spanStates.emplace_back(span);

            return spanStates;
        }()) {}

auto ParameterSpanSetState::Apply() const noexcept -> void {
    for (auto const& state : States)
        state.Apply();
}


ParameterSpaceState::ParameterSpaceState(PipelineParameterSpace& parameterSpace) :
        ParameterSpace(parameterSpace),
        States([&parameterSpace]() {
            SpanSetStates spanSetStates;
            spanSetStates.reserve(parameterSpace.GetNumberOfSpanSets());

            for (auto& spanSet : parameterSpace.ParameterSpanSets)
                spanSetStates.emplace_back(spanSet);

            return spanSetStates;
        }()) {}

auto ParameterSpaceState::Apply() const noexcept -> void {
    for (auto const& state : States)
        state.Apply();
}
