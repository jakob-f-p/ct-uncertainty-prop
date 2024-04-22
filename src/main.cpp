#include "App.h"

auto main(int argc, char *argv[]) -> int {
    App* app = App::CreateInstance(argc, argv);

    return app->Run();
}
