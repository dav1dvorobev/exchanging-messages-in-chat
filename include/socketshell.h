#ifndef SOCKETSHELL
#define SOCKETSHELL

#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

#define MESSAGE_SIZE 1024

class SocketShell {
    int _clientSocket;
public:
    SocketShell(int __clientSocket = -1) : _clientSocket(__clientSocket) {}
    SocketShell(std::string __address, int __port) {
        _clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddres;
        serverAddres.sin_family = AF_INET;
        serverAddres.sin_port = htons(__port);
        serverAddres.sin_addr.s_addr = inet_addr(__address.c_str());
        if (connect(_clientSocket, (sockaddr*)&serverAddres,
                    sizeof(serverAddres)) == -1) {
            close(_clientSocket);
            throw std::runtime_error("[ERROR] FAILED CONNECT");
        }
    }
    operator int() const { return _clientSocket; }
};
std::string readString(const SocketShell& __fd) {
    char input[MESSAGE_SIZE] = "\0";
    int bytesRead = recv(__fd, input, MESSAGE_SIZE, 0);
    if (bytesRead <= 0) { throw std::runtime_error("[ERROR] FAILED READ"); }
    return std::string(input);
}
void sendString(const SocketShell& __fd, const std::string& __buf) {
    int bytesSend = send(__fd, __buf.c_str(), __buf.size(), 0);
    if (bytesSend == -1) { throw std::runtime_error("[ERROR] FAILED SEND"); }
}

#endif // SOCKETSHELL