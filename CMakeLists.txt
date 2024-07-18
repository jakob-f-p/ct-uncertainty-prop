cmake_minimum_required(VERSION 3.27)

project(uncertainty_propagation VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
add_subdirectory(deps/tests)
add_subdirectory(deps/tracy)
add_subdirectory(deps/pybind11)
add_subdirectory(deps/nlohmann_json)
add_subdirectory(deps/HighFive)
