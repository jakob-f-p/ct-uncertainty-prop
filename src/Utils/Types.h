#pragma once

#include <vtkType.h>

#include <array>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

using DoubleVector = std::array<double, 3>;
using FloatVector = std::array<float, 3>;

using DoublePoint = DoubleVector;
using FloatPoint = FloatVector;

[[nodiscard]] auto static
DoubleToFloatPoint(DoublePoint point) -> FloatPoint { return { static_cast<float>(point[0]),
                                                               static_cast<float>(point[1]),
                                                               static_cast<float>(point[2]) }; }

using uidx_t = uint16_t;

using StructureId = uint16_t;


struct idx_t : public std::optional<uidx_t> {
    using std::optional<uidx_t>::operator=;

    auto
    operator++() noexcept -> idx_t& { if (has_value()) value()++; return *this;  }

    auto
    operator++(int) noexcept -> idx_t { idx_t previous = *this; ++(*this); return previous;  }

    auto
    operator--() noexcept -> idx_t& { if (has_value()) value()--; return *this;  }

    auto
    operator--(int) noexcept -> idx_t { idx_t previous = *this; --(*this); return previous;  }

    [[nodiscard]] auto
    ToSigned() const noexcept -> int32_t { return has_value() ? value() : -1; }

    [[nodiscard]] auto
    ToUnsigned() const -> uidx_t { if (!has_value()) throw std::runtime_error("idx must be valid");
                                   return value(); }

    [[nodiscard]] auto static
    FromSigned(int32_t idx) noexcept -> idx_t { return idx >= 0 ? idx_t { idx } : idx_t { std::nullopt }; }

    [[nodiscard]] auto static
    SignedAsUnsignedIdx(int32_t idx) -> uidx_t { if (idx < 0) throw std::runtime_error("idx must be positive");
                                                 return static_cast<uidx_t>(idx); }
};

template<typename T>
concept IsNamed = requires(T obj) {
    { obj.GetName() } -> std::same_as<std::string>;
      obj.SetName("a");
    { obj.GetViewName() } -> std::same_as<std::string>;
};


template<typename T>
concept HasMTime = requires(T obj) {
    { obj.GetMTime() } -> std::same_as<vtkMTimeType>;
};


template<typename T>
struct DataType {
    using Type = std::decay_t<std::remove_pointer_t<T>>::Data;
};

template<class T>
using DataTypeT = typename DataType<T>::Type;


template<typename T>
struct WidgetType {
    using Type = std::decay_t<std::remove_pointer_t<T>>::Widget;
};

template<class T>
using WidgetTypeT = typename WidgetType<T>::Type;


template<typename ...T>
using DataVariant = std::variant<DataTypeT<T>...>;

template<typename ...T>
using WidgetPointerVariant = std::variant<WidgetTypeT<DataTypeT<T>>*...>;



template<typename TData>
struct GetArtifactType {
    using Type = std::decay_t<TData>::Artifact;
};

template<class T>
using ArtifactTypeT = typename GetArtifactType<T>::Type;


