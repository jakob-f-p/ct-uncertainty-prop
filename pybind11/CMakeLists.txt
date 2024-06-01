include(FetchContent)

FetchContent_Declare (
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG stable
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(pybind11)

target_link_libraries(uncertainty_propagation PRIVATE pybind11::embed)
