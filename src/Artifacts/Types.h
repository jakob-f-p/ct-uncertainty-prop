#pragma once

#include <concepts>
#include <variant>


template<typename TData>
struct GetArtifactType {
    using Type = std::decay_t<TData>::Artifact;
};

template<class T>
using ArtifactTypeT = typename GetArtifactType<T>::Type;


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
