#pragma once

#include "../Utils/Types.h"

#include <functional>
#include <string>


template<typename T>
requires std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>
class ObjectProperty {
    using GetterFunction = std::function<T()>;
    using SetterFunction = std::function<void(T)>;

public:
    ObjectProperty(std::string name, GetterFunction getter, SetterFunction setter) :
            Name(std::move(name)),
            Getter(std::move(getter)),
            Setter(std::move(setter)) {};
    ObjectProperty(ObjectProperty&& other) noexcept = default;
    auto operator= (ObjectProperty&& other) noexcept -> ObjectProperty& = default;
    ObjectProperty(ObjectProperty const& other) = delete;
    auto operator= (ObjectProperty const& other) -> ObjectProperty& = delete;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return Name; }

    [[nodiscard]] auto
    Get() const noexcept -> T { return Getter(); }

    auto
    Set(T t) noexcept -> void { Setter(t); }

    [[nodiscard]] auto
    operator== (ObjectProperty const& /*other*/) const noexcept -> bool { return false; }

private:
    std::string Name;
    GetterFunction Getter;
    SetterFunction Setter;
};

using FloatObjectProperty = ObjectProperty<float>;
using FloatPointObjectProperty = ObjectProperty<FloatPoint>;
using PipelineParameterPropertyVariant = std::variant<FloatObjectProperty, FloatPointObjectProperty>;

using PipelineParameterProperties = std::vector<PipelineParameterPropertyVariant>;
