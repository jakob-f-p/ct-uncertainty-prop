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
