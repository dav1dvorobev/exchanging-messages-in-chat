#pragma once
#include <nlohmann/json.hpp>
#include "SocketShell.h"
#include "file_utils.h"
#include <vector>

using json = nlohmann::json;

struct ClientShell{
    std::string login;
    SocketShell clientSocket;
    ClientShell(std::string login, SocketShell clientSocket):login(login), clientSocket(clientSocket){}
    operator SocketShell() const{return clientSocket;}
};

namespace global{
    std::mutex clientMutex;
    std::mutex historyMutex;
    std::vector<ClientShell> clients;
}
std::string status(const std::string& __login, const std::string& __password){
    for(auto client : global::clients){
        if(client.login == __login){
            return RED + std::string("already online") + RESET;
        }
    }
    std::ifstream file("users.json");
    json users = json::parse(file);
    file.close();
    try{
        std::string password = users[__login];
        if(password != __password){
            return RED + std::string("wrong password") + RESET;
        } 
    }catch(...){return RED + std::string("wrong login") + RESET;}
    return "accept";
}
void preload_history(const SocketShell& __clientSocket){
    int counter = 0;
    recv(__clientSocket, &counter, sizeof(counter), 0);
    std::ifstream history("chat_history.txt");
    if(!history){return;}
    std::string* buf = new std::string[counter];
    int idx = 0;
    while(!history.eof()){std::getline(history, buf[idx++ % counter]);}
    for(int i = 0; i < counter; ++i){
        if(!buf[(idx + i) % counter].empty()){
            sendString(__clientSocket, buf[(idx + i) % counter] + ENDL);
        }
    }
    delete[] buf;
    history.close();
}
void sendClients(const std::string& __buf){
    std::lock_guard<std::mutex> history_lock(global::historyMutex);{
        std::ofstream history("chat_history.txt", std::ios_base::app);
        if(history.tellp() != 0){
            history << std::endl;
        }
        history << __buf;
        history.close();
    }
    std::lock_guard<std::mutex> send_lock(global::clientMutex);
    for(auto client : global::clients){
        sendString(client, __buf + ENDL);
    }
}