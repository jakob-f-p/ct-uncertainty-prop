set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


### add source files as executables ###
file(GLOB_RECURSE UNCERTAINTY_PROPAGATION_SRC CONFIGURE_DEPENDS "*.h" "*.cpp" "*.ui")

add_executable(uncertainty_propagation ${UNCERTAINTY_PROPAGATION_SRC})


### add Qt6 ###
set(QT_COMPONENTS
        Core
        Gui
        Widgets
)
find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

foreach (qt_component IN LISTS QT_COMPONENTS)
    list(APPEND QT_MODULES "Qt::${qt_component}")
endforeach ()
target_link_libraries(uncertainty_propagation PRIVATE ${QT_MODULES})

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${QT6_INSTALL_PREFIX}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

set(QT_SRC_DIR "${QT_INSTALL_PATH}/../Src")
if (EXISTS ${QT_SRC_DIR})
    set(QT_BASE_SRC_DIR "${QT_INSTALL_PATH}/../qtbase_Src")
    if (NOT EXISTS ${QT_BASE_SRC_DIR})
        set(QT_BASE_SRC_DIR "${QT_INSTALL_PATH}/../Src/qtbase/src")
    endif ()

    if (EXISTS ${QT_BASE_SRC_DIR})
        target_include_directories(Qt::Core INTERFACE "${QT_BASE_SRC_DIR}/corelib")
        target_include_directories(Qt::Widgets INTERFACE "${QT_BASE_SRC_DIR}/widgets")
        target_include_directories(Qt::Gui INTERFACE "${QT_BASE_SRC_DIR}/gui")
        message(NOTICE "Including QT sources from ${QT_BASE_SRC_DIR}/<target_dir> to <target>")
    endif ()
endif ()

file(GLOB RESOURCES_FOLDER CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/Resources/*.png")
qt_add_resources(uncertainty_propagation images
        PREFIX "/"
        BASE "Resources"
        FILES ${RESOURCES_FOLDER} "Resources/ArrowLeftDisabled.png"
)


### add VTK ###
set(VTK_COMPONENTS
        CommonCore
        FiltersCore
        GUISupportQt
        InteractionStyle
        RenderingAnnotation
        RenderingCore
        RenderingQt
        RenderingVolume
        RenderingVolumeOpenGL2
)

if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_IGNORE_PATH "C:/Program Files (x86)/VTK/build")
endif ()
find_package(VTK REQUIRED COMPONENTS ${VTK_COMPONENTS} PATHS "C:/Program Files (x86)/VTK/release-build")
message(STATUS "VTK_DIR=${VTK_DIR}")

foreach (vtk_component IN LISTS VTK_COMPONENTS)
    list(APPEND VTK_MODULES "VTK::${vtk_component}")
endforeach ()

target_link_libraries(uncertainty_propagation PRIVATE ${VTK_MODULES})

vtk_module_autoinit(
        TARGETS uncertainty_propagation
        MODULES ${VTK_MODULES}
)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(uncertainty_propagation PUBLIC BUILD_TYPE_DEBUG=TRUE)
endif ()


### add Python ###
find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    if (_variableName MATCHES "PY" OR _variableName MATCHES "Py")
#        message(STATUS "${_variableName}=${${_variableName}}")
#    endif ()
#endforeach()

target_link_libraries(uncertainty_propagation PRIVATE Python::Python Python::Module)
