#include "App.h"

int main(int argc, char *argv[]) {
    App* app = App::CreateInstance(argc, argv);

    return app->Run();
}
