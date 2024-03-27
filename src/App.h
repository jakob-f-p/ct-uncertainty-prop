#pragma once

#include <QApplication>

class CtDataCsgTree;
class PipelineList;
class MainWindow;

class App {
public:
    static App* CreateInstance(int argc, char* argv[]);
    static App* GetInstance();

    int Run();
    static int Quit();

    CtDataCsgTree& GetCtDataCsgTree() const;
    PipelineList& GetPipelines() const;

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
    CtDataCsgTree& CtDataTree;
    PipelineList& Pipelines;
    MainWindow* MainWin;
};