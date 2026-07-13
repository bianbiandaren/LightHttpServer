#include "Server.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MimeType.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <filesystem>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

Server::Server(int port, const std::string& rootDir)
    : port_(port), rootDir_(rootDir), serverSocket_(-1), running_(false) {}

Server::~Server() {
    stop();
}

bool Server::start() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }
#endif

    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return false;
    }

    int optval = 1;
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&optval), sizeof(optval));

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(serverSocket_, FIONBIO, &mode);
#else
    fcntl(serverSocket_, F_SETFL, O_NONBLOCK);
#endif

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(serverSocket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket." << std::endl;
        return false;
    }

    if (listen(serverSocket_, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket." << std::endl;
        return false;
    }

    std::cout << "Server listening on port " << port_ << std::endl;
    running_ = true;

    fd_set readfds;
    int maxFd = serverSocket_;

    while (running_) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket_, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(maxFd + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0 && running_) {
            std::cerr << "Select error." << std::endl;
            break;
        }

        if (FD_ISSET(serverSocket_, &readfds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            int clientSocket = accept(serverSocket_,
                reinterpret_cast<struct sockaddr*>(&clientAddr),
                &clientAddrLen);

            if (clientSocket >= 0) {
                std::cout << "New connection from " 
                    << inet_ntoa(clientAddr.sin_addr) << ":" 
                    << ntohs(clientAddr.sin_port) << std::endl;
                handleClient(clientSocket);
            }
        }
    }

    return true;
}

void Server::stop() {
    running_ = false;
    if (serverSocket_ >= 0) {
#ifdef _WIN32
        closesocket(serverSocket_);
        WSACleanup();
#else
        close(serverSocket_);
#endif
        serverSocket_ = -1;
    }
}

void Server::handleClient(int clientSocket) {
#ifdef _WIN32
    u_long mode = 0;
    ioctlsocket(clientSocket, FIONBIO, &mode);
#else
    int flags = fcntl(clientSocket, F_GETFL, 0);
    fcntl(clientSocket, F_SETFL, flags & ~O_NONBLOCK);
#endif

    char buffer[8192];
    std::string request;

    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            break;
        }

        request.append(buffer, bytesRead);

        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (!request.empty()) {
        std::string response = handleRequest(request);
        send(clientSocket, response.c_str(), response.size(), 0);
    }

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
}

std::string Server::handleRequest(const std::string& request) {
    HttpRequest httpRequest;
    if (!httpRequest.parse(request)) {
        return getErrorResponse(400);
    }

    std::string method = httpRequest.method();
    std::string uri = httpRequest.uri();

    if (method != "GET" && method != "HEAD") {
        return getErrorResponse(501);
    }

    if (uri.empty() || uri[0] != '/') {
        return getErrorResponse(400);
    }

    if (!Utils::isSafePath(uri)) {
        return getErrorResponse(403);
    }

    std::string filePath = rootDir_ + uri;

    if (filePath.back() == '/') {
        filePath += "index.html";
    }

    std::filesystem::path fsPath(filePath);
    if (!std::filesystem::exists(fsPath)) {
        return getErrorResponse(404);
    }

    if (std::filesystem::is_directory(fsPath)) {
        filePath += "/index.html";
        if (!std::filesystem::exists(filePath)) {
            return getErrorResponse(403);
        }
    }

    return serveFile(filePath);
}

std::string Server::serveFile(const std::string& filePath) {
    std::string content;
    if (!Utils::readFile(filePath, content)) {
        return getErrorResponse(404);
    }

    HttpResponse response;
    response.setStatus(200);
    response.setContentType(MimeType::get(filePath));
    response.setBody(content);

    return response.build();
}

std::string Server::getErrorResponse(int statusCode) {
    HttpResponse response;
    response.setStatus(statusCode);

    std::string errorPage = rootDir_ + "/" + std::to_string(statusCode) + ".html";
    std::string content;

    if (Utils::readFile(errorPage, content)) {
        response.setContentType("text/html; charset=utf-8");
        response.setBody(content);
    } else {
        response.setContentType("text/plain; charset=utf-8");
        response.setBody("Error " + std::to_string(statusCode));
    }

    return response.build();
}