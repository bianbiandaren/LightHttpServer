#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server {
public:
    Server(int port, const std::string& rootDir);
    ~Server();

    bool start();
    void stop();

private:
    void handleClient(int clientSocket);

    std::string handleRequest(
        const std::string& rawRequest
    );

    std::string serveFile(
        const std::string& filePath
    );

    std::string getErrorResponse(
        int statusCode
    );

    bool sendAll(
        int clientSocket,
        const std::string& data
    );

    int port_;
    std::string rootDir_;
    int serverSocket_;
    bool running_;
};

#endif