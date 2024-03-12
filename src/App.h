#pragma once

#include "Modeling/CtDataCsgTree.h"

#include <QApplication>

class App {
public:
    static App* CreateInstance(int argc, char* argv[]);
    static App* GetInstance();

    int Run();
    static int Quit();

    App(const App&) = delete;
    void operator=(const App&) = delete;
    App() = delete;

    ~App();

protected:
    App(int argc, char* argv[]);

private:
    void InitializeWithTestData();

    CtDataCsgTree* CtDataTree;
    int Argc;
    char** Argv;
    QApplication* QApp;

    static App* Self;
};