#include "Config.h"
#include "Server.h"

#include <iostream>

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

    if (!server.start()) {
        return 1;
    }

    return 0;
}