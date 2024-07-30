#include "App.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>


auto main(int argc, char* argv[]) -> int {
#ifdef BUILD_TYPE_DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    auto logger = spdlog::rotating_logger_mt("app", "../data/logs/log.txt", 10ULL * (1<<10), 20, true);
    spdlog::set_default_logger(logger);
#ifdef BUILD_TYPE_DEBUG
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
#else
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::info);
#endif

    std::unique_ptr<App> const app { App::CreateInstance(argc, argv) };

    int const appExitCode = app->Run();

    spdlog::info("Application shut down with exit code {}", appExitCode);

    return appExitCode;
}
