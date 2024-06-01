#include "App.h"

#include <iostream>
#include <span>
#include <string>
#include <stdexcept>

auto main(int argc, char* argv[]) -> int {
    App* app = App::CreateInstance(argc, argv);

    int const appExitCode = app->Run();

    return appExitCode;
}
