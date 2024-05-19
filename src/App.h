#pragma once

#include <memory>

class QApplication;

class CtDataSource;
class CtStructureTree;
class PipelineList;
class MainWindow;

class App {
public:
    App() = delete;
    App(const App&) = delete;
    void operator=(const App&) = delete;
    App(App&&) = delete;
    void operator=(App&&) = delete;

    [[nodiscard]] static
    auto CreateInstance(int argc, char* argv[]) -> App*;

    [[nodiscard]] static
    auto GetInstance() -> App*;

    auto Run() -> int;

    static auto Quit() -> int;

    [[nodiscard]] auto
    GetCtDataTree() const -> CtStructureTree&;

    [[nodiscard]] auto
    GetPipelines() const -> PipelineList&;

protected:
    App(int argc, char* argv[]);
    ~App();

private:
    auto InitializeWithTestData() -> void;

    static App* Self;

    int Argc;
    char** Argv;

    std::unique_ptr<QApplication> QApp;
    std::unique_ptr<MainWindow> MainWin;
    std::unique_ptr<CtStructureTree> CtDataTree;
    std::unique_ptr<PipelineList> Pipelines;
};