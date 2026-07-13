#include "Config.h"
#include "Server.h"

#include <csignal>
#include <iostream>

static Server* serverInstance = nullptr;

void handleSignal(int signalNumber) {
    if (serverInstance != nullptr) {
        std::cout << "\nReceived signal "
                  << signalNumber
                  << ", shutting down server..."
                  << std::endl;

        serverInstance->stop();
    }
}

int main() {
    Config config;

    config.load("config/server.conf");

    std::cout << "Port: "
              << config.port()
              << '\n';

    std::cout << "Root directory: "
              << config.rootDir()
              << '\n';

    Server server(
        config.port(),
        config.rootDir()
    );

    serverInstance = &server;

    struct sigaction signalAction {};
    signalAction.sa_handler = handleSignal;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;

    sigaction(SIGINT, &signalAction, nullptr);
    sigaction(SIGTERM, &signalAction, nullptr);

    std::cout << "Starting LightHttpServer..."
              << std::endl;

    if (!server.start()) {
        std::cerr << "Failed to start server."
                  << std::endl;

        serverInstance = nullptr;
        return 1;
    }

    serverInstance = nullptr;
    return 0;
}