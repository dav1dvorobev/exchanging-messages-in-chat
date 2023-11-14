#include "ServerSocket.h"
#include "ClientShell.h"
#include "file_utils.h"

#define PORT 8080
#define LOGGING_PATH "server.log"

void clientHandler(SocketShell clientSocket){
    std::string login;
    std::string password;
    while(true){
        readString(clientSocket, login);
        readString(clientSocket, password);
        try{
            if(cheak(login, password)){
                sendString(clientSocket, "accept");
                break;
            }
            else{sendString(clientSocket, "wrong password");}
        }catch(std::exception& e){sendString(clientSocket, e.what());}
    }
    preload(clientSocket);
    global::clients.push_back({login, clientSocket});
    size_t current = global::clients.size() - 1;
    sendClients(login + GREEN + " connected" + RESET);
    try{
        while(true){
            std::string message;
            if(readString(global::clients[current], message) == 0){
                closeSocket(global::clients[current]);
                global::clients.erase(global::clients.begin() + current);
                sendClients(login + RED + " disconnected" + RESET);
                break;
            }
            sendClients(login + ": " + message);
        }
    }catch(std::exception& e){
        logging(e.what(), LOGGING_PATH);
    }
}
int main(){
    try{
        ServerSocket serverSocket(PORT);
        std::cout << "server start in port " << serverSocket.port() << std::endl;
        logging("[INFO] SERVER START IN PORT " + std::to_string(PORT), LOGGING_PATH);
        while(true){
            SocketShell clientSocket = serverSocket.acceptConnection();
            std::thread clientThread(clientHandler, clientSocket);
            clientThread.detach();
        }
    } catch(std::exception& e){
        logging(e.what(), LOGGING_PATH);
    }
    return 0;
} 