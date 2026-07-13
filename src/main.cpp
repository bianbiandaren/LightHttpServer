#include "Server.h"
#include "Config.h"

#include <iostream>
#include <signal.h>

static Server* serverInstance = nullptr;

void handleSignal(int signal) {
    if (serverInstance) {
        std::cout << "\nShutting down server..." << std::endl;
        serverInstance->stop();
    }
}

int main() {
    Config config;
    config.load("config/server.conf");

    Server server(config.port(), config.rootDir());
    serverInstance = &server;

#ifdef _WIN32
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
#else
    struct sigaction sa;
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif

    std::cout << "Starting LightHttpServer..." << std::endl;

    if (!server.start()) {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }

    return 0;
}