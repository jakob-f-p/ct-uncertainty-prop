include(FetchContent)

FetchContent_Declare (
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(TRACY_ENABLE OFF)
set(TRACY_ONLY_LOCALHOST ON)

FetchContent_MakeAvailable(tracy)

target_link_libraries(uncertainty_propagation PRIVATE TracyClient)
