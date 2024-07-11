#pragma once

#include "../Utils/IndexTypes.h"
#include "../Utils/LinearAlgebraTypes.h"

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
    auto operator= (ObjectProperty const& other) -> ObjectProperty& = default;

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

        return Name == other.Name && Range == other.Range;
    }

private:
    std::string Name;
    GetterFunction Getter;
    SetterFunction Setter;
    PropertyRange Range;
};

using FloatObjectProperty = ObjectProperty<float>;
using FloatPointObjectProperty = ObjectProperty<FloatPoint>;

class PipelineParameterProperty {
    using PipelineParameterPropertyVariant = std::variant<FloatObjectProperty, FloatPointObjectProperty>;

public:
    template<typename Args>
    PipelineParameterProperty(Args&& args) :
            PropertyVariant(std::forward<Args>(args)) {}

    [[nodiscard]] auto
    GetName() const noexcept -> std::string {
        return std::visit([](auto const& property) { return property.GetName(); }, PropertyVariant);
    }

    template<typename T>
    [[nodiscard]] auto
    IsType() const noexcept -> bool { return std::holds_alternative<ObjectProperty<T>>(PropertyVariant); }

    template<typename T>
    [[nodiscard]] auto
    Get() noexcept -> ObjectProperty<T>& { return std::get<ObjectProperty<T>>(PropertyVariant); }

    [[nodiscard]] auto
    Variant() noexcept -> PipelineParameterPropertyVariant& { return PropertyVariant; }

private:
    PipelineParameterPropertyVariant PropertyVariant;
};



class PipelineParameterProperties {
public:
    template<typename T>
    [[nodiscard]] auto
    GetPropertyByName(std::string name) -> ObjectProperty<T>& {
        auto it = std::find_if(Properties.begin(), Properties.end(),
                               [&name](auto& property) { return property.template IsType<T>()
                                                                 && property.GetName() == name; });

        if (it == Properties.end())
            throw std::runtime_error("No property with given name found");

        return (*it).template Get<T>();
    }

    [[nodiscard]] auto
    GetNames() -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(Properties.size());

        for (const auto& property : Properties)
            names.emplace_back(property.GetName());

        return names;
    }

    template<typename... Args>
    auto
    Add(Args&&... args) -> auto {
        Properties.emplace_back(std::forward<Args>(args)...);
    }

    [[nodiscard]] auto
    At(uidx_t idx) -> PipelineParameterProperty& { return Properties.at(idx); }

private:
    std::vector<PipelineParameterProperty> Properties;
};
