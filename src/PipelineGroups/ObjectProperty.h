#pragma once

#include "../Utils/Types.h"

#include <functional>
#include <string>
#include <variant>

namespace ObjectPropertyDetails {
    struct PropertyRange {
        float Min = -100.0;
        float Max = 100.0;
        float Step = 1.0;
        uint8_t Decimals = 1;

        [[nodiscard]] auto
        operator== (PropertyRange const& other) const noexcept -> bool = default;
    };
}

template<typename T>
class ObjectProperty {
    using GetterFunction = std::function<T()>;
    using SetterFunction = std::function<void(T)>;

public:
    using PropertyRange = ObjectPropertyDetails::PropertyRange;

    ObjectProperty(std::string name,
                   GetterFunction getter,
                   SetterFunction setter,
                   PropertyRange propertyRange) :
            Name(std::move(name)),
            Getter(std::move(getter)),
            Setter(std::move(setter)),
            Range(propertyRange) {};
    ObjectProperty(ObjectProperty&& other) noexcept = default;
    auto operator= (ObjectProperty&& other) noexcept -> ObjectProperty& = default;
    ObjectProperty(ObjectProperty const& other) = default;
    auto operator= (ObjectProperty const& other) -> ObjectProperty& = delete;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return Name; }

    [[nodiscard]] auto
    GetRange() const noexcept -> PropertyRange { return Range; }

    [[nodiscard]] auto
    Get() const noexcept -> T { return Getter(); }

    auto
    Set(T t) noexcept -> void { Setter(t); }

    [[nodiscard]] auto
    operator== (ObjectProperty const& other) const noexcept -> bool {
        if (this == &other)
            return true;

        return Name == other.Name
                && GetterAddress() == other.GetterAddress()
                && SetterAddress() == other.SetterAddress()
                && Range == other.Range;
    }

private:
    [[nodiscard]] auto
    GetterAddress() const -> size_t {
        using GetterFunctionType = T ();
        GetterFunctionType* const* getterPointer = Getter.template target<GetterFunctionType*>();
        return (size_t) *getterPointer;
    }

    [[nodiscard]] auto
    SetterAddress() const -> size_t {
        using SetterFunctionType = void (T);
        SetterFunctionType* const* setterPointer = Setter.template target<SetterFunctionType*>();
        return (size_t) *setterPointer;
    }

    std::string Name;
    GetterFunction Getter;
    SetterFunction Setter;
    PropertyRange Range;
};

using FloatObjectProperty = ObjectProperty<float>;
using FloatPointObjectProperty = ObjectProperty<FloatPoint>;
using PipelineParameterPropertyVariant = std::variant<FloatObjectProperty, FloatPointObjectProperty>;

using PipelineParameterProperties = std::vector<PipelineParameterPropertyVariant>;
