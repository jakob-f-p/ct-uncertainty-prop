#pragma once

#include "PipelineParameterSpan.h"


template<typename T>
class ObjectProperty;

class PipelineParameterSpanSet;
class PipelineParameterSpace;

template<typename T>
class SpanState {
public:
    explicit SpanState(ParameterSpan<T>& parameterSpan) :
            Value(parameterSpan.Property.Get()),
            Property(parameterSpan.Property) {}


    auto
    Apply() const noexcept -> void { Property.Set(Value); }

private:
    T Value;
    ObjectProperty<T>& Property;
};


class ParameterSpanState {
    using SpanStateVariant = std::variant<SpanState<float>, SpanState<FloatPoint>>;

public:
    explicit ParameterSpanState(PipelineParameterSpan& parameterSpan);

    auto
    Apply() const noexcept -> void;

private:
    SpanStateVariant SpanState;
};


class ParameterSpanSetState {
    using SpanStates = std::vector<ParameterSpanState>;

public:
    explicit ParameterSpanSetState(PipelineParameterSpanSet& parameterSpanSet);

    auto
    Apply() const noexcept -> void;

private:
    PipelineParameterSpanSet& SpanSet;
    SpanStates States;
};


class ParameterSpaceState {
    using SpanSetStates = std::vector<ParameterSpanSetState>;

public:
    explicit ParameterSpaceState(PipelineParameterSpace& parameterSpace);

    auto
    Apply() const noexcept -> void;

private:
    PipelineParameterSpace& ParameterSpace;
    SpanSetStates States;
};
