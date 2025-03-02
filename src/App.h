#pragma once

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <memory>

class CtDataSource;
class CtStructureTree;
class MainWindow;
class PipelineList;
class PipelineGroupList;
class PythonInterpreter;
class ThresholdFilter;

class QApplication;
class vtkImageAlgorithm;


class App {
public:
    App() = delete;
    App(const App&) = delete;
    void operator=(const App&) = delete;
    App(App&&) = delete;
    void operator=(App&&) = delete;
    ~App();

    [[nodiscard]] static
    auto CreateInstance(int argc, char* argv[]) -> App*;

    [[nodiscard]] static
    auto GetInstance() -> App&;

    auto Run() -> int;

    static auto Quit() -> int;

    [[nodiscard]] auto
    GetCtDataTree() const -> CtStructureTree&;

    enum struct CtDataSourceType : uint8_t { IMPLICIT, IMPORTED };

    [[nodiscard]] auto
    GetCtDataSource() const -> CtDataSource&;

    auto
    SetCtDataSource(CtDataSource& ctDataSource) -> void;

    [[nodiscard]] auto
    GetCtDataSourceType() const -> CtDataSourceType;

    [[nodiscard]] auto
    GetImageDimensions() const -> std::array<uint32_t, 3>;

    [[nodiscard]] auto
    GetPipelines() const -> PipelineList&;

    [[nodiscard]] auto
    GetThresholdFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetPipelineGroups() const -> PipelineGroupList&;

    [[nodiscard]] auto
    GetPythonInterpreter() const -> PythonInterpreter&;

protected:
    App(int argc, char* argv[]);

private:
    auto InitializeWithTestData() -> void;

    static App* Self;

    int Argc;
    char** Argv;

    std::unique_ptr<QApplication> QApp;
    std::unique_ptr<CtStructureTree> CtDataTree;
    vtkSmartPointer<CtDataSource> DataSource;
    vtkNew<ThresholdFilter> ThresholdFilterAlgorithm;
    std::unique_ptr<PipelineList> Pipelines;
    std::unique_ptr<PipelineGroupList> PipelineGroups;
    std::unique_ptr<PythonInterpreter> PyInterpreter;
    std::unique_ptr<MainWindow> MainWindow_;
};