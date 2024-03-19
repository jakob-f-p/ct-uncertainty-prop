cmake_minimum_required(VERSION 3.27)

project(uncertainty_propagation)

set(CMAKE_CXX_STANDARD 17)

#option(TRACY_ENABLE 0)
#option(TRACY_ONLY_LOCALHOST 1)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tracy)