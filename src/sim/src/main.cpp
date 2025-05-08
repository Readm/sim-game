#include "server.h"
#include <iostream>
#include <string>

bool parseCommandLine(int argc, char* argv[], int& port) {
    port = 8080;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    int port;
    
    if (!parseCommandLine(argc, argv, port)) {
        return 1;
    }
    
    // 创建并启动服务器
    sim::Server server(port);
    if (!server.start()) {
        std::cerr << "无法启动服务器" << std::endl;
        return 1;
    }
    
    std::cout << "服务器已启动，按Enter键停止..." << std::endl;
    std::cin.get();
    
    // 停止服务器
    server.stop();
    return 0;
} 