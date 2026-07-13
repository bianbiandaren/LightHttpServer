#include "Server.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(
    int port,
    const std::string& rootDir
) : port_(port), rootDir_(rootDir), serverSocket_(-1), running_(false) {
}

Server::~Server() {
    stop();
}

void Server::stop() {
    running_ = false;

    if (serverSocket_ != -1) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
}

bool Server::start() {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenFd == -1) {
        std::cerr << "socket failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    int reuseAddress = 1;

    if (
        setsockopt(
            listenFd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)
        ) == -1
    ) {
        std::cerr << "setsockopt failed: "
                  << std::strerror(errno)
                  << '\n';

        close(listenFd);
        return false;
    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (
        bind(
            listenFd,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ) == -1
    ) {
        std::cerr << "bind failed: "
                  << std::strerror(errno)
                  << '\n';

        close(listenFd);
        return false;
    }

    if (listen(listenFd, 16) == -1) {
        std::cerr << "listen failed: "
                  << std::strerror(errno)
                  << '\n';

        close(listenFd);
        return false;
    }

    std::cout << "Server is listening on port "
              << port_
              << '\n';

    sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);

    int clientFd = accept(
        listenFd,
        reinterpret_cast<sockaddr*>(&clientAddress),
        &clientAddressLength
    );

    if (clientFd == -1) {
        std::cerr << "accept failed: "
                  << std::strerror(errno)
                  << '\n';

        close(listenFd);
        return false;
    }

    std::cout << "A client connected.\n";

    close(clientFd);
    close(listenFd);

    return true;
}
