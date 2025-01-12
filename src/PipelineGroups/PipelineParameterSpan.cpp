#include "PipelineParameterSpan.h"

#include "../Utils/Overload.h"

#include <algorithm>
#include <format>

template<typename T>
[[nodiscard]] auto
ParameterSpan<T>::NumberDetails::GetNumberOfPipelines() const noexcept -> uint32_t {
    if (Step == 0)
        return 1;

    return (Max - Min) / Step + 1;
}

template<typename T>
auto ParameterSpan<T>::NumberDetails::operator==(NumberDetails const& other) const noexcept -> bool {
    return Min == other.Min && Max == other.Max && Step == other.Step;
}

template<typename T>
[[nodiscard]] auto
ParameterSpan<T>::NumberDetails::ToString() const noexcept -> std::string {
    return std::format("{{ [{}, {}] -> {} }}", Min, Max, Step);
}

template<>
[[nodiscard]] auto
ParameterSpan<FloatPoint>::NumberDetails::GetNumberOfPipelines() const noexcept -> uint32_t {
    if (std::ranges::all_of(Step, [](float step) { return step == 0.0; }))
        return 0;

    std::array<uint32_t, 3> numberOfPipelinesPerCoordinate {};
    for (int i = 0; i < 3; i++)
        numberOfPipelinesPerCoordinate[i] = Step[i] == 0.0
                                            ? std::numeric_limits<int>::max()
                                            : static_cast<int>((Max[i] - Min[i]) / Step[i]) + 1;

    return *std::ranges::min_element(numberOfPipelinesPerCoordinate);
}

template<>
[[nodiscard]] auto
ParameterSpan<FloatPoint>::NumberDetails::ToString() const noexcept -> std::string {
    return std::format("{{ [({}, {}, {}), ({}, {}, {})] -> ({}, {}, {}) }}",
                       Min.at(0), Min.at(1), Min.at(2),
                       Max.at(0), Max.at(1), Max.at(2),
                       Step.at(0), Step.at(1), Step.at(2));
}

template<typename T>
ParameterSpan<T>::ParameterSpan(ArtifactVariantPointer artifactPointer, ObjectProperty<T> objectProperty,
                                NumberDetails numbers, std::string name) :
        ArtifactPointer(artifactPointer),
        Name(name.empty()
             ? objectProperty.GetName() + " " + numbers.ToString()
             : std::move(name)),
        Property(std::move(objectProperty)),
        Numbers(std::move(numbers)) {}

template<typename T>
auto ParameterSpan<T>::operator==(ParameterSpan const& other) const noexcept -> bool {
    return Property == other.Property && Numbers == other.Numbers;
}

template class ParameterSpan<float>;
template class ParameterSpan<FloatPoint>;



auto PipelineParameterSpan::GetArtifact() const noexcept -> ArtifactVariantPointer {
    return std::visit([](auto const& span) { return span.GetArtifact(); }, SpanVariant);
}

auto PipelineParameterSpan::GetName() const noexcept -> std::string {
    return std::visit([](auto const& span) { return span.GetName(); }, SpanVariant);
}

auto PipelineParameterSpan::GetPropertyName() const noexcept -> std::string {
    return std::visit([](auto const& span) { return span.GetPropertyName(); }, SpanVariant);
}

auto PipelineParameterSpan::SetName(std::string const& name) noexcept -> void {
    std::visit([&](auto& span) { span.SetName(name); }, SpanVariant);
}

auto PipelineParameterSpan::GetNumberOfPipelines() const noexcept -> uint32_t {
    return std::visit([](auto const& span) { return span.GetNumberOfPipelines(); }, SpanVariant);
}

auto PipelineParameterSpan::operator==(PipelineParameterSpan const& other) const noexcept -> bool {
    return this == &other
           || SpanVariant == other.SpanVariant;
}

auto PipelineParameterSpan::States() -> std::vector<ParameterSpanState> {
    std::vector<ParameterSpanState> states;
    states.reserve(GetNumberOfPipelines());

    for (ParameterSpanStateSourceIterator it (*this); it != it.End(); ++it)
        states.emplace_back(*this);

    return states;
}

auto PipelineParameterSpan::GetRange() const noexcept -> Range {
    return std::visit(Overload {
        [](ParameterSpan<float> const& span) { return Range { span.GetNumbers().Min, span.GetNumbers().Max }; },
        [](ParameterSpan<FloatPoint> const& span) {
            auto const numbers = span.GetNumbers();
            auto const [minMin, minMax] = std::minmax_element(numbers.Min.cbegin(), numbers.Min.cend());
            auto const [maxMin, maxMax] = std::minmax_element(numbers.Max.cbegin(), numbers.Max.cend());

            return Range { std::min({ *minMin, *maxMin }),
                           std::max({ *minMax, *maxMax }) };
        }
    }, SpanVariant);
}
