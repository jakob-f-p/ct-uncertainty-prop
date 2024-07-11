include(FetchContent)

FetchContent_Declare (
        HighFive
        GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(HIGHFIVE_USE_BOOST OFF)

FetchContent_MakeAvailable(HighFive)

target_link_libraries(uncertainty_propagation PRIVATE HighFive::HighFive)
