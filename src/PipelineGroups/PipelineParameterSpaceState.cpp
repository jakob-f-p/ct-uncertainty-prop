#include "PipelineParameterSpaceState.h"

#include "PipelineParameterSpace.h"
#include "PipelineParameterSpan.h"
#include "../Utils/Overload.h"

#include <stdexcept>


template<typename T>
SpanState<T>::SpanState(ParameterSpan<T>& parameterSpan) :
        Span(parameterSpan),
        Value(parameterSpan.Property.Get()) {}

template<typename T>
auto SpanState<T>::operator=(SpanState const& other) -> SpanState& {
    if (this == &other)
        return *this;

    Value = other.Value;
    Span.Property = other.Span.Property;

    return *this;
}

template<typename T>
auto SpanState<T>::Apply() const -> void {
//    if (Value < Span.Numbers.Min || Value > Span.Numbers.Max)
//        throw std::runtime_error("Value out of span range");

    Span.Property.Set(Value);
}

template<typename T>
auto SpanState<T>::RefersTo(PipelineParameterSpan const& parameterSpan) const noexcept -> bool {
    return parameterSpan == Span;
}

template<typename T>
auto SpanState<T>::GetValue() const noexcept -> T {
    return Value;
}

template class SpanState<float>;
template class SpanState<FloatPoint>;



template<typename T>
SpanStateSourceIterator<T>::SpanStateSourceIterator(ParameterSpan<T>& parameterSpan) :
        Span(&parameterSpan),
        CurrentState([&parameterSpan]() {
            SpanState<T> state { parameterSpan };
            state.Value = parameterSpan.Numbers.Min;
            return state;
        }()),
        InitialState(SpanState<T> { parameterSpan }),
        TotalNumberOfStates(parameterSpan.GetNumberOfPipelines()) {};

template<typename T>
SpanStateSourceIterator<T>::~SpanStateSourceIterator() {
    InitialState.Apply();
}

template<typename T>
auto SpanStateSourceIterator<T>::operator*() const -> SpanState<T> const& { CurrentState.Apply();
                                                                            return CurrentState; }
template<typename T>
auto SpanStateSourceIterator<T>::operator++() -> SpanStateSourceIterator& {
    CurrentState.Value += Span->Numbers.Step;

    StateIdx++;

    return *this;
}

template<typename T>
auto SpanStateSourceIterator<T>::operator++(int) -> void { ++*this; }

template<typename T>
auto SpanStateSourceIterator<T>::operator==(SpanStateSourceIterator const& other) const -> bool {
    return StateIdx == other.StateIdx;
}

template<typename T>
auto SpanStateSourceIterator<T>::End() -> SpanStateSourceIterator {
    return SpanStateSourceIterator { CurrentState.Span, "End" };
}

template<typename T>
SpanStateSourceIterator<T>::SpanStateSourceIterator(ParameterSpan<T>& parameterSpan, std::string&& /*dummy*/)  :
        Span(&parameterSpan),
        CurrentState(SpanState<T>(parameterSpan)),
        InitialState(CurrentState),
        TotalNumberOfStates(parameterSpan.GetNumberOfPipelines()),
        StateIdx(TotalNumberOfStates) {};

template<>
auto inline
SpanStateSourceIterator<FloatPoint>::operator++() -> SpanStateSourceIterator& {
    for (int i = 0; i < CurrentState.Value.size(); ++i)
        CurrentState.Value[i] += Span->Numbers.Step[i];

    StateIdx++;

    return *this;
}

template class SpanStateSourceIterator<float>;
template class SpanStateSourceIterator<FloatPoint>;

static_assert(std::input_iterator<SpanStateSourceIterator<float>>);
static_assert(std::input_iterator<SpanStateSourceIterator<FloatPoint>>);



ParameterSpanState::ParameterSpanState(PipelineParameterSpan& parameterSpan) :
        State([&parameterSpan]() {
            return std::visit(Overload {
                [](ParameterSpan<float>& span)      -> SpanStateVariant { return SpanState<float>(span); },
                [](ParameterSpan<FloatPoint>& span) -> SpanStateVariant { return SpanState<FloatPoint>(span); }
            }, parameterSpan.SpanVariant);
        }()) {}

auto ParameterSpanState::Apply() const noexcept -> void {
    std::visit([](auto const& spanState) { spanState.Apply(); }, State);
}

auto ParameterSpanState::RefersTo(PipelineParameterSpan const& parameterSpan) const noexcept -> bool {
    return std::visit([&parameterSpan](auto const& spanState) { return spanState.RefersTo(parameterSpan); }, State);
}

auto ParameterSpanState::GetValue() const noexcept -> std::variant<float, FloatPoint> {
    return std::visit([](auto const& spanState) -> std::variant<float, FloatPoint> { return spanState.GetValue(); },
                      State);
}


ParameterSpanStateSourceIterator::ParameterSpanStateSourceIterator(PipelineParameterSpan& parameterSpan)  :
        IteratorVariant(std::visit(Overload {
                [](ParameterSpan<float>& span)      -> SourceIteratorVariant {
                    return SpanStateSourceIterator<float>(span); },
                [](ParameterSpan<FloatPoint>& span) -> SourceIteratorVariant {
                    return SpanStateSourceIterator<FloatPoint>(span); }
        }, parameterSpan.SpanVariant)) {}

auto ParameterSpanStateSourceIterator::operator*() const -> ParameterSpanState {
    return std::visit([](auto& it) { return ParameterSpanState { *it }; }, IteratorVariant);
}

auto ParameterSpanStateSourceIterator::operator++() -> ParameterSpanStateSourceIterator& {
    std::visit([](auto& it) { ++it; }, IteratorVariant);

    return *this;
}

auto ParameterSpanStateSourceIterator::operator++(int) -> void { ++*this; }

auto ParameterSpanStateSourceIterator::End() -> ParameterSpanStateSourceIterator {
    return std::visit([](auto& it) { return ParameterSpanStateSourceIterator { it.End() }; }, IteratorVariant);
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

auto ParameterSpanSetState::FindSpanStateBySpan(PipelineParameterSpan const& parameterSpan) const noexcept
        -> std::optional<std::reference_wrapper<ParameterSpanState const>> {

    for (auto const& state : States) {
        if (state.RefersTo(parameterSpan))
            return state;
    }

    return std::nullopt;
}


PipelineParameterSpaceState::PipelineParameterSpaceState(PipelineParameterSpace& parameterSpace) :
        ParameterSpace(parameterSpace),
        States([&parameterSpace]() {
            SpanSetStates spanSetStates;
            spanSetStates.reserve(parameterSpace.GetNumberOfSpanSets());

            for (auto& spanSet : parameterSpace.ParameterSpanSets)
                spanSetStates.emplace_back(spanSet);

            return spanSetStates;
        }()) {}

auto PipelineParameterSpaceState::Apply() const noexcept -> void {
    for (auto const& state : States)
        state.Apply();
}

auto PipelineParameterSpaceState::FindSpanStateBySpan(PipelineParameterSpan const& parameterSpan) const
        -> ParameterSpanState const& {

    for (auto const& state : States) {
        std::optional<std::reference_wrapper<ParameterSpanState const>> res = state.FindSpanStateBySpan(parameterSpan);

        if (res)
            return *res;
    }

    throw std::runtime_error("span not found");
}
