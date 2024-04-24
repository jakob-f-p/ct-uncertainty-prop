#pragma once

#include <concepts>
#include <string>

template<typename T>
concept IsNamed = requires(T obj) {
    { obj.GetName() } -> std::same_as<std::string>;
      obj.SetName("a");
    { obj.GetViewName() } -> std::same_as<std::string>;
};


using vtkMTimeType = uint64_t;

template<typename T>
concept HasMTime = requires(T obj) {
    { obj.GetMTime() } -> std::same_as<vtkMTimeType>;
};


template<typename TArtifact>
struct ArtifactDataType {
    using Type = std::decay_t<TArtifact>::Data;
};

template<class T>
using ArtifactDataT = typename ArtifactDataType<T>::Type;


template<typename TArtifactWidget>
struct ArtifactWidgetData {
    using Type = std::decay_t<std::remove_pointer_t<TArtifactWidget>>::Data;
};

template<class T>
using ArtifactWidgetDataT = typename ArtifactWidgetData<T>::Type;


template<typename TData>
struct DataArtifactType {
    using Type = std::decay_t<TData>::Artifact;
};

template<class T>
using DataArtifactT = typename DataArtifactType<T>::Type;


