include(FetchContent)

FetchContent_Declare (
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG stable
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(PYBIND11_PYTHON_VERSION 3.9.13)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    if (_variableName MATCHES "PY" OR _variableName MATCHES "Py")
#        message(STATUS "${_variableName}=${${_variableName}}")
#    endif ()
#endforeach()

FetchContent_MakeAvailable(pybind11)

target_link_libraries(uncertainty_propagation PRIVATE pybind11::embed)
