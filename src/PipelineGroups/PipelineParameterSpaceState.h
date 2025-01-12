#pragma once

#include "../Utils/LinearAlgebraTypes.h"

#include <iterator>
#include <optional>
#include <string>
#include <variant>
#include <vector>


template<typename T>
class ObjectProperty;

template<typename T>
class ParameterSpan;

class PipelineParameterSpace;
class PipelineParameterSpanSet;
struct PipelineParameterSpan;


template<typename T>
class SpanState {
public:
    explicit SpanState(ParameterSpan<T>& parameterSpan);

    SpanState(SpanState const& other) = default;
    SpanState(SpanState&& other) = default;
    auto operator= (SpanState const& other) -> SpanState&;
    auto operator= (SpanState&& other) -> SpanState& = default;

    auto
    Apply() const -> void;

    [[nodiscard]] auto
    RefersTo(PipelineParameterSpan const& parameterSpan) const noexcept -> bool;

    [[nodiscard]] auto
    GetValue() const noexcept -> T;

    [[nodiscard]] auto
    operator== (SpanState const& other) const noexcept -> bool = default;

private:
    template<typename U> friend struct SpanStateSourceIterator;

    ParameterSpan<T>& Span;
    T Value;
};



template<typename T>
struct SpanStateSourceIterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = SpanState<T>;
    using difference_type = ptrdiff_t;

    explicit SpanStateSourceIterator(ParameterSpan<T>& parameterSpan);
    auto operator=(SpanStateSourceIterator const&) -> SpanStateSourceIterator& = default;
    ~SpanStateSourceIterator();

    auto
    operator*() const -> SpanState<T> const&;

    auto
    operator++() -> SpanStateSourceIterator&;

    auto
    operator++(int) -> void;

    [[nodiscard]] auto
    operator==(SpanStateSourceIterator const& other) const -> bool;

    [[nodiscard]] auto
    End() -> SpanStateSourceIterator;

private:
    explicit SpanStateSourceIterator(ParameterSpan<T>& parameterSpan, std::string&& /*dummy*/);

    ParameterSpan<T>* Span;
    SpanState<T> CurrentState;
    SpanState<T> InitialState;
    int TotalNumberOfStates;
    int StateIdx {0};
};



class ParameterSpanState {
    using SpanStateVariant = std::variant<SpanState<float>, SpanState<FloatPoint>>;

public:
    explicit ParameterSpanState(PipelineParameterSpan& parameterSpan);

    template<typename Arg>
    explicit ParameterSpanState(Arg&& arg)
            : State(std::forward<decltype(arg)>(arg)) {}

    auto
    Apply() const noexcept -> void;

    [[nodiscard]] auto
    RefersTo(PipelineParameterSpan const& parameterSpan) const noexcept -> bool;

    [[nodiscard]] auto
    GetValue() const noexcept -> std::variant<float, FloatPoint>;

private:
    friend struct ParameterSpanSetState;

    SpanStateVariant State;
};

struct ParameterSpanStateSourceIterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = ParameterSpanState;
    using difference_type = ptrdiff_t;

    using SourceIteratorVariant = std::variant<SpanStateSourceIterator<float>, SpanStateSourceIterator<FloatPoint>>;

    explicit ParameterSpanStateSourceIterator(PipelineParameterSpan& parameterSpan);

    auto
    operator*() const -> ParameterSpanState;

    auto
    operator++() -> ParameterSpanStateSourceIterator&;

    auto
    operator++(int) -> void;

    [[nodiscard]] auto
    operator==(ParameterSpanStateSourceIterator const& other) const -> bool = default;

    [[nodiscard]] auto
    End() -> ParameterSpanStateSourceIterator;

private:
    template<typename Arg>
    explicit ParameterSpanStateSourceIterator(Arg&& arg)
            : IteratorVariant(std::forward<decltype(arg)>(arg)) {}

    SourceIteratorVariant IteratorVariant;
};

static_assert(std::input_iterator<ParameterSpanStateSourceIterator>);


struct ParameterSpanSetState {
    using SpanStates = std::vector<ParameterSpanState>;

    explicit ParameterSpanSetState(PipelineParameterSpanSet& parameterSpanSet);

    auto
    Apply() const noexcept -> void;

    [[nodiscard]] auto
    FindSpanStateBySpan(PipelineParameterSpan const& parameterSpan) const noexcept
            -> std::optional<std::reference_wrapper<ParameterSpanState const>>;

private:
    PipelineParameterSpanSet& SpanSet;
    SpanStates States;
};



class PipelineParameterSpaceState {
    using SpanSetStates = std::vector<ParameterSpanSetState>;

public:
    explicit PipelineParameterSpaceState(PipelineParameterSpace& parameterSpace);

    auto
    Apply() const noexcept -> void;

    [[nodiscard]] auto
    FindSpanStateBySpan(PipelineParameterSpan const& parameterSpan) const -> ParameterSpanState const&;

private:
    friend class PipelineParameterSpaceStateModel;

    PipelineParameterSpace& ParameterSpace;
    SpanSetStates States;
};
