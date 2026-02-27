#include "Pch.h"
#include "System.h"
// spdlog
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
// STL
#include <vector>
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    auto system = std::make_unique<System>();

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks { console_sink, msvc_sink };
    auto logger = std::make_shared<spdlog::logger>("Sisyphus", sinks.begin(), sinks.end());

    // 로그 레벨 설정
    logger->set_level(spdlog::level::debug);
    // 글로벌 로거로 등록
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%T] [%^%l%$] %v");
    spdlog::info("spdlog Initialized with MSVC Sink!");
    
    if (system->Init()) {
        spdlog::info("Sisyphus Renderer Started!");
        system->Run();
    }
    return 0;
} // WinMain
