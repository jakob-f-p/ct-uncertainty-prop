include(FetchContent)

FetchContent_Declare (
        HighFive
        GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
        GIT_TAG master
#        GIT_TAG v2.9.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(HIGHFIVE_USE_BOOST OFF)

FetchContent_MakeAvailable(HighFive)

#function(print_all_targets DIR)
#    get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
#    foreach(TGT IN LISTS TGTS)
#        message(STATUS "Target: ${TGT}")
#    endforeach()
#
#    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
#    foreach(SUBDIR IN LISTS SUBDIRS)
#        print_all_targets("${SUBDIR}")
#    endforeach()
#endfunction()
#
#print_all_targets(.)
#
#if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
#    get_target_property(HIGH_FIVE_SOURCE_DIR HighFive::HighFive SOURCE_DIR)
#    set(HIGH_FIVE_INCLUDE_DIR "${HIGH_FIVE_SOURCE_DIR}/include")
#    message(NOTICE "Including ${HIGH_FIVE_INCLUDE_DIR}")
#    target_include_directories(uncertainty_propagation PRIVATE HIGH_FIVE_INCLUDE_DIR)
#endif ()

target_link_libraries(uncertainty_propagation PRIVATE HighFive::HighFive)
