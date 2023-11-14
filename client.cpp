#include <nlohmann/json.hpp>
#include "SocketShell.h"
#include "file_utils.h"

using json = nlohmann::json;

std::string __buffer;
void readFromServer(SocketShell clientSocket){
    while(true){
        std::string message;
        if(readString(clientSocket, message) == 0){break;}
        std::cout << CLEAR_LINE << message;
        if(!__buffer.empty()){
            std::cout << __buffer;
            std::cout.flush();
        }
    }
}
int main(int argc, char** argv){
    std::ifstream __file("config.json");
    if(!__file){
        std::cerr << "Cannot opening file" << std::endl;
        return -1;
    }
    json config = json::parse(__file);
    std::string __address = config["server_address"];
    const int __port = config["port"];
    try{
        SocketShell clientSocket(__address, __port);
        while(true){
            std::cout << "login: ";
            sendString(clientSocket, input());
            std::cout << "password: ";
            sendString(clientSocket, input("hidden"));
            readString(clientSocket, __buffer);
            if(__buffer == "accept"){break;}
            else{std::cout << RED + __buffer + RESET << std::endl;}
        }
        sendString(clientSocket, config["preload_msgs"]);
        std::thread readThread(readFromServer, clientSocket);
        readThread.detach();
        while(true){
            input(__buffer);
            if(__buffer == "!q"){break;}
            std::cout << UP;
            if(validation(__buffer)){sendString(clientSocket, __buffer);}
            else{std::cout << CLEAR_LINE << RED << "validation error" << RESET << ENDL;}
        }
        closeSocket(clientSocket);
    }catch(std::exception& e){
        logging(e.what(), config["logging_path"]);
    }
    return 0;
}