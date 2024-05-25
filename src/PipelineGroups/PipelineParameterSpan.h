#pragma once

#include "ObjectProperty.h"
#include "../Utils/Types.h"

#include <algorithm>
#include <format>
#include <functional>
#include <utility>

class Pipeline;

template<typename T>
class ParameterSpan {
public:
    struct NumberDetails {
        T Min;
        T Max;
        T Step;

        [[nodiscard]] auto
        operator== (NumberDetails const& other) const noexcept -> bool {
            return Min == other.Min && Max == other.Max && Step == other.Step;
        };

        [[nodiscard]] auto
        ToString() const noexcept -> std::string {
            return std::format("{{ [{}, {}] -> {} }}", Min, Max, Step);
        }
    };

    ParameterSpan(ArtifactVariantPointer artifactPointer,
                  ObjectProperty<T> objectProperty,
                  NumberDetails numbers,
                  std::string name = "") :
            ArtifactPointer(artifactPointer),
            Name(name.empty()
                    ? objectProperty.GetName() + " " + numbers.ToString()
                    : std::move(name)),
            Property(std::move(objectProperty)),
            Numbers(std::move(numbers)) {};
    ParameterSpan(ParameterSpan&& other) noexcept = default;
    auto operator= (ParameterSpan&& other) noexcept -> ParameterSpan& = default;
    ParameterSpan(ParameterSpan const& other) = delete;
    auto operator= (ParameterSpan const& other) -> ParameterSpan& = delete;
    ~ParameterSpan() = default;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return Name; }

    auto
    SetName(std::string name) noexcept -> void { Name = std::move(name); }

    [[nodiscard]] auto
    GetPropertyName() const noexcept -> std::string { return Property.GetName(); }

    [[nodiscard]] auto
    CanAdvance() const noexcept -> bool {
        double const current = Property.Get();

        return current >= Numbers.Min && current <= (Numbers.Max - Numbers.Step);
    }

    auto
    Advance() noexcept -> void {
        double const current = Property.Get();
        double const next = current + Numbers.Step;

        Property.Set(next);
    }

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint32_t {
        if (Numbers.Step == 0)
            return 1;

        return ((Numbers.Max - Numbers.Min) / Numbers.Step) + 1;
    }

    [[nodiscard]] auto
    GetArtifact() const noexcept -> ArtifactVariantPointer { return ArtifactPointer; }

    [[nodiscard]] auto
    GetNumbers() const noexcept -> NumberDetails { return Numbers; }

    [[nodiscard]] auto
    operator== (ParameterSpan const& other) const noexcept -> bool {
        return Property == other.Property && Numbers == other.Numbers;
    };

private:
    ArtifactVariantPointer ArtifactPointer;
    std::string Name;
    ObjectProperty<T> Property;
    NumberDetails Numbers;
};


template<>
[[nodiscard]] auto inline
ParameterSpan<FloatPoint>::CanAdvance() const noexcept -> bool {
    FloatPoint const current = Property.Get();
    FloatPoint maxMinusStep = {};
    for (int i = 0; i < maxMinusStep.size(); ++i)
        maxMinusStep[i] = Numbers.Max[i] - Numbers.Step[i];

    bool notTooSmall = true;
    for (int i = 0; i < maxMinusStep.size(); ++i)
        if (current[i] < Numbers.Min[i])
            notTooSmall = false;

    bool notTooLarge = true;
    for (int i = 0; i < maxMinusStep.size(); ++i)
        if (current[i] > maxMinusStep[i])
            notTooLarge = false;

    return notTooSmall && notTooLarge;
}


template<>
auto inline
ParameterSpan<FloatPoint>::Advance() noexcept -> void {
    FloatPoint const current = Property.Get();
    FloatPoint next = {};
    for (int i = 0; i < next.size(); ++i)
        next[i] = Numbers.Max[i] + Numbers.Step[i];

    Property.Set(next);
}

template<>
[[nodiscard]] auto inline
ParameterSpan<FloatPoint>::GetNumberOfPipelines() const noexcept -> uint32_t {
    if (std::any_of(Numbers.Step.begin(), Numbers.Step.end(), [](float step) { return step == 0.0; }))
        return 0;

    std::array<uint32_t, 3> numberOfPipelinesPerCoordinate {};
    for (int i = 0; i < 3; i++)
        numberOfPipelinesPerCoordinate[i] = ((Numbers.Max[i] - Numbers.Min[i]) / Numbers.Step[i]) + 1;

    return *std::min_element(numberOfPipelinesPerCoordinate.begin(), numberOfPipelinesPerCoordinate.end());
}

template<>
[[nodiscard]] auto inline
ParameterSpan<FloatPoint>::NumberDetails::ToString() const noexcept -> std::string {
    return std::format("{{ [({}, {}, {}), ({}, {}, {})] -> ({}, {}, {}) }}",
                       Min.at(0), Min.at(1), Min.at(2),
                       Max.at(0), Max.at(1), Max.at(2),
                       Step.at(0), Step.at(1), Step.at(2));
}


struct PipelineParameterSpan {

    template<typename... Args>
    PipelineParameterSpan(Args&&... args) : SpanVariant(std::forward<Args>(args)...) {};

    [[nodiscard]] auto
    GetArtifact() const noexcept -> ArtifactVariantPointer {
        return std::visit([](auto const& span) { return span.GetArtifact(); }, SpanVariant);
    }

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return std::visit([](auto const& span) { return span.GetName(); },
                                                                SpanVariant); }

    [[nodiscard]] auto
    GetPropertyName() const noexcept -> std::string {
        return std::visit([](auto const& span) { return span.GetPropertyName(); }, SpanVariant);
    }

    auto
    SetName(const std::string& name) noexcept -> void { std::visit([&](auto& span) { span.SetName(name); },
                                                                   SpanVariant); }

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint32_t {
        return std::visit([](auto const& span) { return span.GetNumberOfPipelines(); },
                          SpanVariant);
    }

    [[nodiscard]] auto
    operator== (PipelineParameterSpan const& other) const noexcept -> bool {
        return this == &other
               || SpanVariant == other.SpanVariant;
    }

private:
    friend class ObjectPropertyGroup;

    using ParameterSpanVariant = std::variant<ParameterSpan<float>, ParameterSpan<FloatPoint>>;
    ParameterSpanVariant SpanVariant;
};
