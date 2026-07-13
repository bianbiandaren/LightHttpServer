#ifndef SERVER_H
#define SERVER_H

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

class Server {
public:
    Server(int port, const std::string& rootDir);
    ~Server();

    void setStopFlag(const volatile std::sig_atomic_t* stopFlag);
    bool start();
    void stop();

private:
    struct ClientConnection {
        std::string requestBuffer;
        std::string responseBuffer;
        std::size_t sentBytes = 0;
    };

    bool setupListenSocket();
    bool setupEpoll();
    bool setNonBlocking(int fd);

    bool addToEpoll(int fd, uint32_t events);
    bool modifyEpoll(int fd, uint32_t events);
    void removeFromEpoll(int fd);

    void acceptClients();
    void handleRead(int clientFd);
    void handleWrite(int clientFd);
    void closeClient(int clientFd);

    std::string handleRequest(const std::string& rawRequest);
    std::string serveFile(const std::string& filePath);
    std::string getErrorResponse(int statusCode);

    bool shouldStop() const;

    int port_;
    std::string rootDir_;
    int serverSocket_;
    int epollFd_;
    bool running_;
    const volatile std::sig_atomic_t* stopFlag_;
    std::unordered_map<int, ClientConnection> clients_;
};

#endif
