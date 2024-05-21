#pragma once

#include "ObjectProperty.h"
#include "../Utils/Types.h"

#include <functional>

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
    };

    ParameterSpan(ObjectProperty<T> objectProperty, NumberDetails numbers) :
            Property(std::move(objectProperty)),
            Numbers(std::move(numbers)) {};
    ParameterSpan(ParameterSpan&& other) noexcept = default;
    auto operator= (ParameterSpan&& other) noexcept -> ParameterSpan& = default;
    ParameterSpan(ParameterSpan const& other) = delete;
    auto operator= (ParameterSpan const& other) -> ParameterSpan& = delete;
    ~ParameterSpan() = default;

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
    operator== (ParameterSpan const& other) const noexcept -> bool {
        return Property == other.Property && Numbers == other.Numbers;
    };

private:
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
