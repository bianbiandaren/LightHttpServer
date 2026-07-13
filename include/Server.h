#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server {
public:
    Server(int port, const std::string& rootDir);

    bool start();

private:
    int port_;
    std::string rootDir_;
};

#endif