#include <iostream>
#include <string>
#include <cstring>

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
        std::cout << "Running in console mode..." << std::endl;
        std::cout << "Device Config Monitor Service started." << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        // 模拟服务运行
        while (true) {
            // 实际服务逻辑会在这里
            // 简单示例：每5秒输出一次心跳
            std::cout << "Service is running..." << std::endl;
            // 在真实代码中，这里会使用sleep或定时器
            // 为了演示，我们只运行一次就退出
            break;
        }
    }
    else {
        std::cout << "Starting " << PROGRAM_NAME << " as Windows Service..." << std::endl;
        std::cout << "Use --console flag to run in console mode." << std::endl;
        // Windows服务逻辑将在这里
    }

    return 0;
}