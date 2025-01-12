#pragma once

#include <pybind11/embed.h>
#include <pybind11/stl/filesystem.h>

#include <filesystem>
#include <span>


class PythonInterpreter {
public:
    explicit PythonInterpreter(int argc = 0, const char* const* argv = nullptr) :
            Interpreter([argc, argv] {
                        PyConfig config;
                        PyConfig_InitPythonConfig(&config);

                        config.parse_argv = 0;
                        config.install_signal_handlers = 1;

                        AddPythonLibPaths(config);

                        return new pybind11::scoped_interpreter(&config, argc, argv, true);
            }()),
            NameModuleMap(new std::unordered_map<std::string, ImportedModule>()){

        std::string const setMultiprocessingPath = "import multiprocessing\n"
                                                   "import os.path\n"
                                                   "import sys\n"
                                                   "executable_path = os.path.join(sys.exec_prefix, 'python.exe')\n"
#ifdef BUILD_TYPE_DEBUG
                                                   "print('python_executable_path', executable_path)\n"
#endif
                                                   "multiprocessing.set_executable(executable_path)";
        pybind11::exec(setMultiprocessingPath);

        AddImportedModule("extract_features");
        AddImportedModule("pca");
        AddImportedModule("tsne");

        ThreadState = PyEval_SaveThread();
    }

    ~PythonInterpreter() {
        PyEval_RestoreThread(ThreadState);
        delete NameModuleMap;
        delete Interpreter;

        delete &pybind11::detail::get_local_internals();
    }

    template<typename ...T>
    auto
    ExecuteFunction(std::string const& moduleName, std::string const& functionName, T... args) -> pybind11::object {
        auto& module = NameModuleMap->at(moduleName);

        return module.ExecuteFunction(functionName, args...);
    }

    struct ImportedModule {
        explicit ImportedModule(std::string const& name) :
                Module({ pybind11::module_::import(name.c_str()) }) {}

        template<typename ...T>
        auto
        ExecuteFunction(std::string const& functionName, T... args) -> pybind11::object {
//            Module.reload();

            SetArgvPathToModulePath();

            return Module.attr(functionName.c_str())(args...);
        }

    private:
        auto
        SetArgvPathToModulePath() const -> void {
            std::string const addArgvFileName = "import sys\n"
                                                "sys.argv = ['" + GetModulePath().generic_string() + "']";

            pybind11::exec(addArgvFileName);
        }

        [[nodiscard]] auto
        GetModulePath() const -> std::filesystem::path {
            return Module.attr("__file__").cast<std::filesystem::path>();
        }

        std::string Name;
        pybind11::module_ Module;
    };

private:
    auto static
    AddPythonLibPaths(PyConfig& config) -> void {
        if (PyStatus const status = PyConfig_Read(&config); PyStatus_Exception(status) != 0)
            throw std::runtime_error("PyStatus exception");

        namespace fs = std::filesystem;

        fs::path const libPath = fs::path(config.base_prefix) /= "Lib";
        fs::path const dllsPath = fs::path(config.base_prefix) /= "DLLs";
        fs::path const sitePackagesPath = fs::path(libPath) /= "site-packages";

#ifdef PYTHON_MODULES_DIRECTORY
        fs::path const embeddedModulesDirectory = fs::path { PYTHON_MODULES_DIRECTORY,
                                                             fs::path::format::generic_format }.make_preferred();
#else
        throw std::runtime_exception("PYTHON_MODULES_DIRECTORY not defined");
#endif

        std::array<fs::path const, 4> const pathsToAdd { libPath,
                                                         dllsPath,
                                                         sitePackagesPath,
                                                         embeddedModulesDirectory };

        std::span const oldSearchPaths(config.module_search_paths.items, config.module_search_paths.length);

        for (auto const& path : pathsToAdd)
            PyWideStringList_Append(&config.module_search_paths, path.native().data());
    }

    auto
    AddImportedModule(std::string moduleName) const -> void {
        NameModuleMap->emplace(moduleName, moduleName);
    }


    pybind11::scoped_interpreter* Interpreter;
    std::unordered_map<std::string, ImportedModule>* NameModuleMap;
    PyThreadState* ThreadState = nullptr;
};
