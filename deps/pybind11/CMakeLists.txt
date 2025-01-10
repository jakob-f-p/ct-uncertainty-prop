include(FetchContent)

FetchContent_Declare (
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG stable
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(PYBIND11_FINDPYTHON ON)

FetchContent_MakeAvailable(pybind11)

get_target_property(_importedConfigurations Python::Python IMPORTED_CONFIGURATIONS)
list(LENGTH _importedConfigurations _importedConfigurationsLength)
list(FIND _importedConfigurations "RELEASE" _importedConfigurationsReleaseIdx)
list(FIND _importedConfigurations "DEBUG" _importedConfigurationsDebugIdx)
if (_importedConfigurationsLength EQUAL 2
        AND NOT _importedConfigurationsReleaseIdx EQUAL -1
        AND NOT _importedConfigurationsDebugIdx EQUAL -1)

    get_target_property(_importedLocationRelease Python::Python IMPORTED_LOCATION_RELEASE)
    get_target_property(_importedImpLibRelease Python::Python IMPORTED_IMPLIB_RELEASE)
    set_target_properties(Python::Python PROPERTIES IMPORTED_LOCATION_DEBUG ${_importedLocationRelease})
    set_target_properties(Python::Python PROPERTIES IMPORTED_IMPLIB_DEBUG ${_importedImpLibRelease})
endif ()

target_link_libraries(uncertainty_propagation PRIVATE pybind11::embed)
