#include "App.h"


auto main(int argc, char* argv[]) -> int {
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    std::unique_ptr<App> app { App::CreateInstance(argc, argv) };

    int const appExitCode = app->Run();

    return appExitCode;
}
