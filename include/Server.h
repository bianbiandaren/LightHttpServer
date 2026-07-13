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
    std::string handleRequest(const std::string& request);
    std::string serveFile(const std::string& filePath);
    std::string getErrorResponse(int statusCode);

    int port_;
    std::string rootDir_;
    int serverSocket_;
    bool running_;
};

#endif