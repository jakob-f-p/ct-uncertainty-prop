include(FetchContent)

set(SPDLOG_USE_STD_FORMAT ON)
#set(SPDLOG_ENABLE_PCH ON)
#set(SPDLOG_BUILD_SHARED ON)

FetchContent_Declare (
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(spdlog)

target_link_libraries(uncertainty_propagation PRIVATE spdlog::spdlog)
