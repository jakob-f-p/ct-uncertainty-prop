#include "App.h"

#include <Python.h>

#include <iostream>
#include <span>
#include <string>
#include <stdexcept>

auto InitializePythonInterpreter(char* argv[]) -> void;

auto main(int argc, char* argv[]) -> int {
    InitializePythonInterpreter(argv);

    App* app = App::CreateInstance(argc, argv);

    int const appExitCode = app->Run();

    if (Py_FinalizeEx() < 0)
        exit(120);

    return appExitCode;
}

auto InitializePythonInterpreter(char* argv[]) -> void {
    std::wstring const program = Py_DecodeLocale(argv[0], NULL);
    if (program.empty())
        throw std::runtime_error("Fatal error: cannot decode argv[0]\n");

    Py_SetProgramName(program.data());

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    PyStatus status;
    status = PyConfig_Read(&config);
    if (PyStatus_Exception(status) != 0)
        throw std::runtime_error("PyStatus exception");

    std::wstring const basePrefix = config.base_prefix;
    std::string const moduleDir = "\\Lib";
    std::wstring const modulePath = basePrefix + std::wstring(moduleDir.begin(), moduleDir.end());

    std::span const oldSearchPaths(config.module_search_paths.items, config.module_search_paths.length);
    if (std::find(oldSearchPaths.begin(), oldSearchPaths.end(), modulePath) == oldSearchPaths.end())
        PyWideStringList_Append(&config.module_search_paths, modulePath.data());

    std::span const newSearchPaths(config.module_search_paths.items, config.module_search_paths.length);
    for (auto* const searchPath: newSearchPaths)
        std::wcout << searchPath << "\n";

    status = Py_InitializeFromConfig(&config);

    Py_Initialize();
    PyRun_SimpleString("import sys\n"
                       "print('Python', sys.version, 'running')\n");
}
