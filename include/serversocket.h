#ifndef SERVERSOCKET
#define SERVERSOCKET

#include <netinet/ip.h>
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

class ServerSocket {
    int _port;
    int _serverSocket;
    sockaddr_in _serverAddress;
public:
    ServerSocket(int __port = 8080) : _port(__port) {
        _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        int optval = 1;
        setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        _serverAddress.sin_family = AF_INET;
        _serverAddress.sin_addr.s_addr = INADDR_ANY;
        _serverAddress.sin_port = htons(_port);
        if (bind(_serverSocket, (sockaddr*)&_serverAddress, sizeof(_serverAddress)) == -1) {
            this->~ServerSocket();
            throw std::runtime_error("[ERROR] FAILED BIND");
        }
        if (listen(_serverSocket, 64) == -1) {
            this->~ServerSocket();
            throw std::runtime_error("[ERROR] FAILED LISTEN");
        }
    }
    int acceptConnection() const {
        int clientSocket = accept(_serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            this->~ServerSocket();
            throw std::runtime_error("[ERROR] FAILED ACCEPT");
        }
        return clientSocket;
    }
    int port() const { return _port; }
    ~ServerSocket() {
        if (_serverSocket != -1) {
            close(_serverSocket);
            _serverSocket = -1;
        }
    }
};

#endif // SERVERSOCKET