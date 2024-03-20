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
    set(QT_BASE_SRC_DIR "${QT_INSTALL_PATH}/../Src/qtbase/src")
    if (EXISTS ${QT_BASE_SRC_DIR})
        message(NOTICE "Including QT sources from ${QT_BASE_SRC_DIR}")
        include_directories(${QT_BASE_SRC_DIR})
    endif ()
endif ()

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

find_package(VTK REQUIRED COMPONENTS ${VTK_COMPONENTS})
foreach (vtk_component IN LISTS VTK_COMPONENTS)
    list(APPEND VTK_MODULES "VTK::${vtk_component}")
endforeach ()

target_link_libraries(uncertainty_propagation PRIVATE ${VTK_MODULES})

vtk_module_autoinit(
        TARGETS uncertainty_propagation
        MODULES ${VTK_MODULES}
)
