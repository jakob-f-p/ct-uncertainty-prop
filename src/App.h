#pragma once

#include <QGuiApplication>

#include <vtkNew.h>

#include <memory>

class CtStructureTree;
class PipelineList;
class MainWindow;

class App {
public:
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

    App(const App&) = delete;
    void operator=(const App&) = delete;
    App() = delete;

protected:
    App(int argc, char* argv[]);
    ~App();

private:
    void InitializeWithTestData();

    static App* Self;

    int Argc;
    char** Argv;

    QApplication* QApp;
    std::unique_ptr<CtStructureTree> CtDataTree;
    std::unique_ptr<PipelineList> Pipelines;
    MainWindow* MainWin;
};