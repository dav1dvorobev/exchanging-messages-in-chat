#pragma once
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

#define MESSAGE_SIZE 1024

class SocketShell{
    int _clientSocket;
public:
    SocketShell(int __clientSocket = -1):_clientSocket(__clientSocket){} 
    SocketShell(std::string __address, int __port){
        _clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddres;
        serverAddres.sin_family = AF_INET;
        serverAddres.sin_port = htons(__port); 
        serverAddres.sin_addr.s_addr = inet_addr(__address.c_str());
        if(connect(_clientSocket, (sockaddr*)&serverAddres, sizeof(serverAddres)) == -1){
            close(_clientSocket);
            throw std::runtime_error("[ERROR] FAILED CONNECT");
        }
    }
    operator int() const{return _clientSocket;}
};
void closeSocket(SocketShell __fd){
    if(__fd != -1){ 
        close(__fd);
    }
}
int readString(const SocketShell& __fd, std::string& __buf){
    char __input[MESSAGE_SIZE] = "\0";
    int bytesRead = read(__fd, __input, MESSAGE_SIZE);
    if(bytesRead == -1){
        throw std::runtime_error("[ERROR] FAILED READING");
    }
    __buf = __input;
    return bytesRead;
}
int sendString(const SocketShell& __fd, const std::string& __buf){
    int bytesSend = write(__fd, __buf.c_str(), __buf.size());
    if(bytesSend == -1){
        throw std::runtime_error("[ERROR] FAILED SENDING");
    }
    return bytesSend;
}