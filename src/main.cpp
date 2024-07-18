#include "App.h"


auto main(int argc, char* argv[]) -> int {
#ifdef BUILD_TYPE_DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    std::unique_ptr<App> app { App::CreateInstance(argc, argv) };

    int const appExitCode = app->Run();

    return appExitCode;
}
