#pragma once

#include <vtkNew.h>

class CtStructureTree;
class PipelineList;
class MainWindow;

class QApplication;

class App {
public:
    static App* CreateInstance(int argc, char* argv[]);
    static App* GetInstance();

    int Run();
    static int Quit();

    [[nodiscard]] CtStructureTree& GetCtDataTree() const;
    [[nodiscard]] PipelineList& GetPipelines() const;

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

    QApplication& QApp;
    vtkNew<CtStructureTree> CtDataTree;
    vtkNew<PipelineList> Pipelines;
    MainWindow* MainWin;
};