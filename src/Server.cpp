#include "Server.h"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MimeType.h"
#include "Utils.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
const int MAX_EVENTS = 64;
const std::size_t MAX_REQUEST_SIZE = 64 * 1024;
}

Server::Server(
    int port,
    const std::string& rootDir
) : port_(port),
    rootDir_(rootDir),
    serverSocket_(-1),
    epollFd_(-1),
    running_(false),
    stopFlag_(nullptr) {
}

Server::~Server() {
    stop();
}

void Server::setStopFlag(const volatile std::sig_atomic_t* stopFlag) {
    stopFlag_ = stopFlag;
}

bool Server::start() {
    if (!setupListenSocket()) {
        return false;
    }

    if (!setupEpoll()) {
        stop();
        return false;
    }

    running_ = true;

    std::cout << "Server is listening on port "
              << port_
              << '\n';

    epoll_event events[MAX_EVENTS];

    while (running_ && !shouldStop()) {
        const int count = epoll_wait(epollFd_, events, MAX_EVENTS, 1000);

        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }

            std::cerr << "epoll_wait failed: "
                      << std::strerror(errno)
                      << '\n';
            stop();
            return false;
        }

        for (int i = 0; i < count; ++i) {
            const int fd = events[i].data.fd;
            const uint32_t eventMask = events[i].events;

            if (fd == serverSocket_) {
                acceptClients();
                continue;
            }

            if (
                eventMask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)
            ) {
                closeClient(fd);
                continue;
            }

            if (eventMask & EPOLLIN) {
                handleRead(fd);
            }

            if (
                clients_.find(fd) != clients_.end() &&
                (eventMask & EPOLLOUT)
            ) {
                handleWrite(fd);
            }
        }
    }

    stop();
    return true;
}

void Server::stop() {
    running_ = false;

    for (auto& item : clients_) {
        close(item.first);
    }

    clients_.clear();

    if (epollFd_ != -1) {
        close(epollFd_);
        epollFd_ = -1;
    }

    if (serverSocket_ != -1) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
}

bool Server::setupListenSocket() {
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
        return false;
    }

    if (!setNonBlocking(serverSocket_)) {
        return false;
    }

    sockaddr_in serverAddress {};
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
        return false;
    }

    if (listen(serverSocket_, 128) == -1) {
        std::cerr << "listen failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    return true;
}

bool Server::setupEpoll() {
    epollFd_ = epoll_create1(0);

    if (epollFd_ == -1) {
        std::cerr << "epoll_create1 failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    return addToEpoll(serverSocket_, EPOLLIN);
}

bool Server::setNonBlocking(int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        std::cerr << "fcntl get failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl set failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    return true;
}

bool Server::addToEpoll(int fd, uint32_t events) {
    epoll_event event {};
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) == -1) {
        std::cerr << "epoll add failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    return true;
}

bool Server::modifyEpoll(int fd, uint32_t events) {
    epoll_event event {};
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) == -1) {
        std::cerr << "epoll modify failed: "
                  << std::strerror(errno)
                  << '\n';
        return false;
    }

    return true;
}

void Server::removeFromEpoll(int fd) {
    if (epollFd_ != -1) {
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
    }
}

void Server::acceptClients() {
    while (true) {
        sockaddr_in clientAddress {};
        socklen_t clientAddressLength = sizeof(clientAddress);

        const int clientFd = accept(
            serverSocket_,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressLength
        );

        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }

            if (errno == EINTR) {
                continue;
            }

            std::cerr << "accept failed: "
                      << std::strerror(errno)
                      << '\n';
            break;
        }

        if (!setNonBlocking(clientFd)) {
            close(clientFd);
            continue;
        }

        if (!addToEpoll(clientFd, EPOLLIN | EPOLLRDHUP)) {
            close(clientFd);
            continue;
        }

        clients_[clientFd] = ClientConnection {};
    }
}

void Server::handleRead(int clientFd) {
    auto it = clients_.find(clientFd);

    if (it == clients_.end()) {
        return;
    }

    char buffer[4096];

    while (true) {
        const ssize_t count = recv(clientFd, buffer, sizeof(buffer), 0);

        if (count > 0) {
            it->second.requestBuffer.append(
                buffer,
                static_cast<std::size_t>(count)
            );

            if (it->second.requestBuffer.size() > MAX_REQUEST_SIZE) {
                it->second.responseBuffer = getErrorResponse(400);
                it->second.sentBytes = 0;
                modifyEpoll(clientFd, EPOLLOUT | EPOLLRDHUP);
                return;
            }

            if (
                it->second.requestBuffer.find("\r\n\r\n") !=
                std::string::npos
            ) {
                it->second.responseBuffer =
                    handleRequest(it->second.requestBuffer);
                it->second.sentBytes = 0;
                modifyEpoll(clientFd, EPOLLOUT | EPOLLRDHUP);
                return;
            }

            continue;
        }

        if (count == 0) {
            closeClient(clientFd);
            return;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        if (errno == EINTR) {
            continue;
        }

        closeClient(clientFd);
        return;
    }
}

void Server::handleWrite(int clientFd) {
    auto it = clients_.find(clientFd);

    if (it == clients_.end()) {
        return;
    }

    std::string& response = it->second.responseBuffer;
    std::size_t& sentBytes = it->second.sentBytes;

    while (sentBytes < response.size()) {
        const ssize_t count = send(
            clientFd,
            response.data() + sentBytes,
            response.size() - sentBytes,
            0
        );

        if (count > 0) {
            sentBytes += static_cast<std::size_t>(count);
            continue;
        }

        if (count == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return;
        }

        if (count == -1 && errno == EINTR) {
            continue;
        }

        closeClient(clientFd);
        return;
    }

    closeClient(clientFd);
}

void Server::closeClient(int clientFd) {
    removeFromEpoll(clientFd);
    close(clientFd);
    clients_.erase(clientFd);
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
    struct stat fileStat {};

    if (stat(filePath.c_str(), &fileStat) == -1) {
        return getErrorResponse(404);
    }

    if (!S_ISREG(fileStat.st_mode)) {
        return getErrorResponse(403);
    }

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
        struct stat fileStat {};

        if (
            stat(errorPage.c_str(), &fileStat) == 0 &&
            S_ISREG(fileStat.st_mode)
        ) {
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

bool Server::shouldStop() const {
    return stopFlag_ != nullptr && *stopFlag_ != 0;
}
