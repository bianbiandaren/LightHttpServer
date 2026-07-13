#include "Config.h"
#include "Server.h"

#include <csignal>
#include <iostream>

volatile std::sig_atomic_t g_stopRequested = 0;

void handleSignal(int) {
    g_stopRequested = 1;
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

    server.setStopFlag(&g_stopRequested);
    std::signal(SIGINT, handleSignal);

    if (!server.start()) {
        return 1;
    }

    return 0;
}
