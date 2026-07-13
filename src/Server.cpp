#include "Server.h"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MimeType.h"
#include "Utils.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port, const std::string& rootDir)
    : port_(port),
      rootDir_(rootDir),
      serverSocket_(-1),
      running_(false) {
}

Server::~Server() {
    stop();
}

bool Server::start() {
    // 1. 创建 IPv4 TCP Socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket_ == -1) {
        std::cerr << "socket failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    // 2. 允许服务器关闭后快速重新绑定端口
    int reuseAddress = 1;

    if (setsockopt(
            serverSocket_,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)
        ) == -1) {
        std::cerr << "setsockopt failed: "
                  << std::strerror(errno)
                  << '\n';

        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    // 3. 把监听 Socket 设置为非阻塞
    const int oldFlags = fcntl(serverSocket_, F_GETFL, 0);

    if (oldFlags == -1 ||
        fcntl(serverSocket_, F_SETFL, oldFlags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl failed: "
                  << std::strerror(errno)
                  << '\n';

        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    // 4. 配置服务器地址
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port_);

    // 5. 绑定端口
    if (bind(
            serverSocket_,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ) == -1) {
        std::cerr << "bind failed: "
                  << std::strerror(errno)
                  << '\n';

        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    // 6. 开始监听
    if (listen(serverSocket_, SOMAXCONN) == -1) {
        std::cerr << "listen failed: "
                  << std::strerror(errno)
                  << '\n';

        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    std::cout << "LightHttpServer is running at http://127.0.0.1:"
              << port_
              << '\n';

    running_ = true;

    // 当前先使用 select 监听连接。
    // 后面会按项目要求改成 epoll。
    while (running_) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket_, &readSet);

        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        const int activity = select(
            serverSocket_ + 1,
            &readSet,
            nullptr,
            nullptr,
            &timeout
        );

        if (activity == -1) {
            if (errno == EINTR) {
                continue;
            }

            std::cerr << "select failed: "
                      << std::strerror(errno)
                      << '\n';
            break;
        }

        // 超时，没有新连接
        if (activity == 0) {
            continue;
        }

        if (!FD_ISSET(serverSocket_, &readSet)) {
            continue;
        }

        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);

        const int clientSocket = accept(
            serverSocket_,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressLength
        );

        if (clientSocket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }

            std::cerr << "accept failed: "
                      << std::strerror(errno)
                      << '\n';
            continue;
        }

        char clientIp[INET_ADDRSTRLEN]{};

        inet_ntop(
            AF_INET,
            &clientAddress.sin_addr,
            clientIp,
            sizeof(clientIp)
        );

        std::cout << "Client connected: "
                  << clientIp
                  << ':'
                  << ntohs(clientAddress.sin_port)
                  << '\n';

        handleClient(clientSocket);
        close(clientSocket);
    }

    stop();
    return true;
}

void Server::stop() {
    running_ = false;

    if (serverSocket_ != -1) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
}

void Server::handleClient(int clientSocket) {
    char buffer[8192]{};
    std::string rawRequest;

    while (rawRequest.find("\r\n\r\n") == std::string::npos) {
        const ssize_t receivedBytes = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (receivedBytes > 0) {
            rawRequest.append(
                buffer,
                static_cast<std::size_t>(receivedBytes)
            );

            // 防止恶意客户端发送过大的请求头
            if (rawRequest.size() > 65536) {
                const std::string response = getErrorResponse(400);
                sendAll(clientSocket, response);
                return;
            }

            continue;
        }

        if (receivedBytes == 0) {
            return;
        }

        if (errno == EINTR) {
            continue;
        }

        std::cerr << "recv failed: "
                  << std::strerror(errno)
                  << '\n';
        return;
    }

    std::cout << "---------- HTTP Request ----------\n"
              << rawRequest
              << "----------------------------------\n";

    const std::string response = handleRequest(rawRequest);

    if (!sendAll(clientSocket, response)) {
        std::cerr << "Failed to send complete response.\n";
    }
}

std::string Server::handleRequest(const std::string& rawRequest) {
    HttpRequest request;

    if (!request.parse(rawRequest)) {
        return getErrorResponse(400);
    }

    const std::string method = request.method();
    std::string uri = request.uri();

    // 本项目只实现 GET。
    // 其他方法按要求返回 501。
    if (method != "GET") {
        return getErrorResponse(501);
    }

    if (uri.empty() || uri.front() != '/') {
        return getErrorResponse(400);
    }

    // 去除查询参数，例如 /index.html?id=1
    const std::size_t queryPosition = uri.find('?');

    if (queryPosition != std::string::npos) {
        uri = uri.substr(0, queryPosition);
    }

    // 简单防止目录遍历攻击
    if (!Utils::isSafePath(uri)) {
        return getErrorResponse(400);
    }

    // 访问 / 时返回首页
    if (uri == "/") {
        uri = "/index.html";
    }

    std::filesystem::path rootPath =
        std::filesystem::weakly_canonical(rootDir_);

    std::filesystem::path requestedPath =
        std::filesystem::weakly_canonical(
            std::filesystem::path(rootDir_) /
            uri.substr(1)
        );

    // 保证请求的文件仍然位于网站根目录中
    const std::string rootString = rootPath.string();
    const std::string requestedString = requestedPath.string();

    if (requestedString.compare(
            0,
            rootString.size(),
            rootString
        ) != 0) {
        return getErrorResponse(400);
    }

    if (!std::filesystem::exists(requestedPath) ||
        !std::filesystem::is_regular_file(requestedPath)) {
        return getErrorResponse(404);
    }

    return serveFile(requestedPath.string());
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

    const std::string errorPage =
        rootDir_ + "/" + std::to_string(statusCode) + ".html";

    std::string content;

    if (Utils::readFile(errorPage, content)) {
        response.setContentType("text/html; charset=utf-8");
        response.setBody(content);
    } else {
        response.setContentType("text/plain; charset=utf-8");

        switch (statusCode) {
            case 400:
                response.setBody("400 Bad Request");
                break;

            case 404:
                response.setBody("404 Not Found");
                break;

            case 501:
                response.setBody("501 Not Implemented");
                break;

            default:
                response.setBody(
                    "HTTP Error " + std::to_string(statusCode)
                );
                break;
        }
    }

    return response.build();
}

bool Server::sendAll(
    int clientSocket,
    const std::string& data
) {
    std::size_t sentBytes = 0;

    while (sentBytes < data.size()) {
        const ssize_t result = send(
            clientSocket,
            data.data() + sentBytes,
            data.size() - sentBytes,
            0
        );

        if (result > 0) {
            sentBytes += static_cast<std::size_t>(result);
            continue;
        }

        if (result == -1 && errno == EINTR) {
            continue;
        }

        return false;
    }

    return true;
}