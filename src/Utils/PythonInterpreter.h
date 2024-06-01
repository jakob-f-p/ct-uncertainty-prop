#include <pybind11/embed.h>

#include <filesystem>
#include <iostream>
#include <span>

class PythonInterpreter : public pybind11::scoped_interpreter {
public:
    struct ImportedModule {
        pybind11::module_ Module;
        std::filesystem::path Path;
    };

    explicit PythonInterpreter(int argc = 0, const char* const* argv = nullptr) :
            pybind11::scoped_interpreter(
                    [argc, argv]() {
                        PyConfig config;
                        PyConfig_InitPythonConfig(&config);

                        config.parse_argv = 0;
                        config.install_signal_handlers = 1;

                        AddPythonLibPath(config);

                        return pybind11::scoped_interpreter(&config, argc, argv, true);
                    }()),
                    ExtractFeaturesModule(std::nullopt) {

        namespace py = pybind11;

        std::string const setMultiprocessingExecutable = "import multiprocessing\n"
                                                         "import os.path\n"
                                                         "import sys\n"
                                                         "executable_path = os.path.join(sys.exec_prefix, 'python.exe')\n"
                                                         "print('executable_path', executable_path)\n"
                                                         "multiprocessing.set_executable(executable_path)";
        py::exec(setMultiprocessingExecutable);
    }

    [[nodiscard]] auto
    GetExtractFeaturesModule() noexcept -> std::optional<ImportedModule>& { return ExtractFeaturesModule; }

private:
    auto static
    AddPythonLibPath(PyConfig& config) -> void {
        PyStatus status;
        status = PyConfig_Read(&config);
        if (PyStatus_Exception(status) != 0)
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

        for (auto const& path : pathsToAdd) {
            auto searchIt = std::find(oldSearchPaths.begin(), oldSearchPaths.end(), path.native());
            PyWideStringList_Append(&config.module_search_paths, path.native().data());
        }

#ifdef BUILD_TYPE_DEBUG
        std::span const newSearchPaths(config.module_search_paths.items, config.module_search_paths.length);
        for (auto* const searchPath: newSearchPaths)
            std::wcout << searchPath << "\n";
#endif
    }

    std::optional<ImportedModule> ExtractFeaturesModule;
};
