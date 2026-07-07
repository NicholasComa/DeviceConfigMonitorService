/**/#include <iostream>
#include <string>
#include <cstring>

#include "config.h"

const std::string PROGRAM_NAME = "DeviceConfigMonitorService";
const std::string VERSION = "1.0.0";

void printBanner() {
    std::cout << "========================================" << std::endl;
    std::cout << "  " << PROGRAM_NAME << std::endl;
    std::cout << "  Version: " << VERSION << std::endl;
    std::cout << "========================================" << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [--console]" << std::endl;
    std::cout << "  --console    Run in console mode" << std::endl;
    std::cout << "  --help       Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    bool consoleMode = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--console") == 0) {
            consoleMode = true;
        }
        else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }

    if (consoleMode) {
        printBanner();

        // 加载配置文件（不存在则生成默认配置，格式错误不崩溃）
        AppConfig config = LoadConfig();

        // 打印配置摘要
        PrintConfigSummary(config);

        std::cout << "Running in console mode..." << std::endl;
        std::cout << "Device Config Monitor Service started." << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        // 使用配置中的心跳间隔运行服务
        while (true) {
            std::cout << "[" << config.ServiceName << "] "
                      << "Heartbeat (interval=" << config.HeartbeatIntervalSeconds
                      << "s), DeviceId=" << config.DeviceId << std::endl;
            // 实际服务中使用 Sleep 等待
            // Sleep(config.HeartbeatIntervalSeconds * 1000);
            break; // 演示用，只运行一次
        }
    }
    else {
        std::cout << "Starting " << PROGRAM_NAME << " as Windows Service..." << std::endl;
        std::cout << "Use --console flag to run in console mode." << std::endl;
        // Windows服务逻辑将在这里
    }

    return 0;
}

/*#include "service_main.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    // Check for console mode
    if (argc >= 2)
    {
        std::string arg = argv[1];
        if (arg == "--console")
        {
            return ServiceMain::GetInstance().RunAsConsole() ? 0 : 1;
        }
    }

    // Run as Windows Service
    return ServiceMain::GetInstance().RunAsService() ? 0 : 1;
}*/