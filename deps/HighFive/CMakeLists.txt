include(FetchContent)

FetchContent_Declare (
        HighFive
        GIT_REPOSITORY https://github.com/highfive-devs/highfive.git
        GIT_TAG main  # major version upgrade expected; should there be issues, set to latest 2.x version
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(HIGHFIVE_USE_BOOST OFF)

FetchContent_MakeAvailable(HighFive)

target_link_libraries(uncertainty_propagation PRIVATE HighFive::HighFive)
