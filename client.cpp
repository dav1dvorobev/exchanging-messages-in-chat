#include <nlohmann/json.hpp>
#include "SocketShell.h"
#include "file_utils.h"

using json = nlohmann::json;

std::string buf;
void readFromServer(SocketShell clientSocket){
    while(true){
        std::string message = readString(clientSocket);
        std::cout << CLEAR_LINE << message;
        if(!buf.empty()){
            std::cout << buf;
            std::cout.flush();
        }
    }
}
int main(int argc, char** argv){
    std::ifstream file("config.json");
    if(!file){
        std::cerr << "Cannot opening file" << std::endl;
        return -1;
    }
    json config = json::parse(file);
    const std::string address = config["server_address"];
    const int port = config["port"];
    const int preload_history = config["preload_history"];
    try{
        SocketShell clientSocket(address, port);
        while(true){
            std::cout << "login: ";
            sendString(clientSocket, input());
            std::cout << "password: ";
            sendString(clientSocket, input("hidden"));
            buf = readString(clientSocket);
            if(buf == "accept"){
                send(clientSocket, &preload_history, sizeof(int), 0);
                break;
            }
            else{std::cout << buf << std::endl;}
        }
        std::thread readThread(readFromServer, clientSocket);
        readThread.detach();
        while(true){
            input(buf);
            if(buf == "!q"){break;}
            std::cout << UP;
            if(validation(buf)){sendString(clientSocket, buf);}
            else{std::cout << CLEAR_LINE << RED << "validation error" << RESET << ENDL;}
        }
        close(clientSocket);
    }catch(std::exception& e){
        logging(e.what(), config["logging_path"]);
        throw;
    }
    return 0;
}