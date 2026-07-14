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

void handleFlood(int connectionsPerThread) {
    for (int i = 0; i < connectionsPerThread; ++i) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            continue;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            close(sock);
        } else {
            close(sock);
        }

#ifdef _WIN32
        WSACleanup();
#endif
    }
}

int main(int argc, char* argv[]) {
    int numThreads = 10;
    int connectionsPerThread = 100;

    if (argc >= 2) {
        numThreads = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        connectionsPerThread = std::stoi(argv[2]);
    }

    std::cout << "Starting handle flood test..." << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    std::cout << "Connections per thread: " << connectionsPerThread << std::endl;
    std::cout << "Total connections: " << numThreads * connectionsPerThread << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(handleFlood, connectionsPerThread);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Test completed!" << std::endl;
    return 0;
}