#pragma once

#include "Artifacts/PipelineList.h"
#include "Modeling/CtDataCsgTree.h"

#include <QApplication>

class App {
public:
    static App* CreateInstance(int argc, char* argv[]);
    static App* GetInstance();

    int Run();
    static int Quit();

    CtDataCsgTree* GetCtDataCsgTree() const;
    PipelineList* GetPipelineList() const;

    App(const App&) = delete;
    void operator=(const App&) = delete;
    App() = delete;

    ~App();

protected:
    App(int argc, char* argv[]);

private:
    void InitializeWithTestData();

    static App* Self;

    int Argc;
    char** Argv;
    QApplication* QApp;

    CtDataCsgTree* CtDataTree;
    PipelineList* Pipelines;
};