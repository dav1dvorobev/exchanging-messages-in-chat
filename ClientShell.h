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
    std::vector<ClientShell> clients;
}
bool cheak(const std::string& __login, const std::string& __password){
    for(auto client : global::clients){
        if(client.login == __login){
            throw std::runtime_error("already online");
        }
    }
    std::ifstream __file("users.json");
    json users = json::parse(__file);
    __file.close();
    std::string password;
    try{
        password = users[__login];
    }catch(...){throw std::runtime_error("wrong login");}
    return users[__login] == __password;
}
void preload(const SocketShell& __clientSocket){
    std::mutex historyMutex;
    std::lock_guard<std::mutex> lock(historyMutex);
    std::string preload_msgs;
    readString(__clientSocket, preload_msgs);
    size_t counter = stoi(preload_msgs);
    std::ifstream __history("chat_history.txt");
    if(!__history){return;}
    std::string* buf = new std::string[counter];
    size_t idx = 0;
    while(!__history.eof()){std::getline(__history, buf[idx++ % counter]);}
    for(size_t i = 0; i < counter; ++i){
        if(!buf[(idx + i) % counter].empty()){
            sendString(__clientSocket, buf[(idx + i) % counter] + ENDL);
        }
    }
    delete[] buf;
    __history.close();
}
void sendClients(const std::string& __buf){
    std::mutex sendMutex;
    std::lock_guard<std::mutex> lock(sendMutex);
    std::ofstream __history("chat_history.txt", std::ios_base::app);
    if(__history.tellp() != 0){
        __history << std::endl;
    }
    __history << __buf;
    std::cout << __buf << std::endl;
    for(auto client : global::clients){sendString(client, __buf + ENDL);}
    __history.close();
}