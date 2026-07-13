#include "Server.h"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MimeType.h"
#include "Utils.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fstream>
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
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket_ == -1) {
        std::cerr << "socket failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    int reuseAddress = 1;

    if (
        setsockopt(
            serverSocket_,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)
        ) == -1
    ) {
        std::cerr << "setsockopt failed: "
                  << std::strerror(errno)
                  << '\n';

        stop();
        return false;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port_);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (
        bind(
            serverSocket_,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ) == -1
    ) {
        std::cerr << "bind failed: "
                  << std::strerror(errno)
                  << '\n';

        stop();
        return false;
    }

    if (listen(serverSocket_, 16) == -1) {
        std::cerr << "listen failed: "
                  << std::strerror(errno)
                  << '\n';

        stop();
        return false;
    }

    running_ = true;

    std::cout << "Server is listening on port "
              << port_
              << '\n';

    while (running_) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(
            serverSocket_,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressLength
        );

        if (clientSocket == -1) {
            if (!running_) {
                break;
            }

            std::cerr << "accept failed: "
                      << std::strerror(errno)
                      << '\n';
            continue;
        }

        handleClient(clientSocket);
        close(clientSocket);
    }

    stop();
    return true;
}

void Server::handleClient(int clientSocket) {
    char buffer[4096];
    const ssize_t received = recv(
        clientSocket,
        buffer,
        sizeof(buffer),
        0
    );

    if (received <= 0) {
        return;
    }

    const std::string rawRequest(buffer, received);
    const std::string response = handleRequest(rawRequest);
    sendAll(clientSocket, response);
}

std::string Server::handleRequest(const std::string& rawRequest) {
    HttpRequest request;

    if (!request.parse(rawRequest)) {
        return getErrorResponse(400);
    }

    if (request.method() != "GET") {
        return getErrorResponse(501);
    }

    std::string uri = request.uri();
    const std::size_t queryPos = uri.find('?');

    if (queryPos != std::string::npos) {
        uri = uri.substr(0, queryPos);
    }

    if (uri.empty() || uri[0] != '/') {
        return getErrorResponse(400);
    }

    if (uri == "/") {
        uri = "/index.html";
    }

    if (!Utils::isSafePath(uri)) {
        return getErrorResponse(403);
    }

    return serveFile(rootDir_ + uri);
}

std::string Server::serveFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        return getErrorResponse(404);
    }

    file.close();

    HttpResponse response;
    response.setStatus(200);
    response.setContentType(MimeType::get(filePath));
    response.setBody(Utils::readFile(filePath));

    return response.build();
}

std::string Server::getErrorResponse(int statusCode) {
    HttpResponse response;
    response.setStatus(statusCode);

    std::string body;
    std::string errorPage;

    if (statusCode == 403) {
        errorPage = rootDir_ + "/403.html";
    } else if (statusCode == 404) {
        errorPage = rootDir_ + "/404.html";
    }

    if (!errorPage.empty()) {
        std::ifstream file(errorPage, std::ios::binary);

        if (file.is_open()) {
            body = Utils::readFile(errorPage);
            response.setContentType("text/html; charset=utf-8");
        }
    }

    if (body.empty()) {
        response.setContentType("text/plain; charset=utf-8");

        if (statusCode == 400) {
            body = "400 Bad Request";
        } else if (statusCode == 403) {
            body = "403 Forbidden";
        } else if (statusCode == 404) {
            body = "404 Not Found";
        } else if (statusCode == 501) {
            body = "501 Not Implemented";
        } else {
            body = "Error";
        }
    }

    response.setBody(body);
    return response.build();
}

bool Server::sendAll(
    int clientSocket,
    const std::string& data
) {
    std::size_t sentTotal = 0;

    while (sentTotal < data.size()) {
        const ssize_t sent = send(
            clientSocket,
            data.data() + sentTotal,
            data.size() - sentTotal,
            0
        );

        if (sent <= 0) {
            return false;
        }

        sentTotal += static_cast<std::size_t>(sent);
    }

    return true;
}
