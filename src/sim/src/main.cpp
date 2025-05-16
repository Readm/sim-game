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
    
    // Create and start server
    sim::Server server(port);
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server started, press Enter to stop..." << std::endl;
    std::cin.get();
    
    // Stop server
    server.stop();
    return 0;
} 