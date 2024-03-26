#pragma once

#include "MainWindow.h"
#include "Artifacts/PipelineList.h"
#include "Modeling/CtDataCsgTree.h"

#include <QApplication>

class App {
public:
    static App* CreateInstance(int argc, char* argv[]);
    static App* GetInstance();

    int Run();
    static int Quit();

    MainWindow* GetMainWindow() const;
    CtDataCsgTree* GetCtDataCsgTree() const;
    PipelineList* GetPipelineList() const;

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

    MainWindow* MainWin;
    CtDataCsgTree* CtDataTree;
    PipelineList* Pipelines;
};