#include "Config.h"
#include "Server.h"

#include <csignal>
#include <iostream>

Server* g_server = nullptr;

void handleSignal(int) {
    if (g_server != nullptr) {
        g_server->stop();
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

    g_server = &server;
    std::signal(SIGINT, handleSignal);

    if (!server.start()) {
        return 1;
    }

    g_server = nullptr;

    return 0;
}
