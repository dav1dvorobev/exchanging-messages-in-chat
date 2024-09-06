#ifndef CLIENTSHELL
#define CLIENTSHELL

#include <nlohmann/json.hpp>

#include "socketshell.h"
#include "utils.h"

using json = nlohmann::json;

struct ClientShell {
    std::string login;
    SocketShell clientSocket;
    ClientShell(std::string login, SocketShell clientSocket)
        : login(login), clientSocket(clientSocket) {}
    operator SocketShell() const { return clientSocket; }
};

namespace global {
std::mutex clientMutex;
std::mutex historyMutex;
std::vector<ClientShell> clients;
} // namespace global

std::string status(const std::string& __login, const std::string& __password) {
    for (auto client : global::clients) {
        if (client.login == __login) { return setColor("already online", {255, 0, 0}); }
    }
    std::ifstream file("../data/users.json");
    json users = json::parse(file);
    file.close();
    try {
        std::string password = users[__login];
        if (password != __password) { return setColor("wrong password", {255, 0, 0}); }
    } catch (...) { return setColor("wrong login", {255, 0, 0}); }
    return "accept";
}
void preload_history(const SocketShell& __clientSocket) {
    int counter = 0;
    recv(__clientSocket, &counter, sizeof(counter), 0);
    std::ifstream history("chat_history.txt");
    if (!history) { return; }
    std::string* buf = new std::string[counter];
    int idx = 0;
    while (!history.eof()) { std::getline(history, buf[idx++ % counter]); }
    for (int i = 0; i < counter; ++i) {
        if (!buf[(idx + i) % counter].empty()) {
            sendString(__clientSocket, buf[(idx + i) % counter] + ENDL);
        }
    }
    delete[] buf;
    history.close();
}
void sendClients(const std::string& __buf) {
    std::cout << __buf << std::endl;
    global::historyMutex.lock();
    std::ofstream history("chat_history.txt", std::ios_base::app);
    if (history.tellp() != 0) { history << std::endl; }
    history << __buf;
    history.close();
    global::historyMutex.unlock();
    global::clientMutex.lock();
    for (auto client : global::clients) { sendString(client, __buf + ENDL); }
    global::clientMutex.unlock();
}

#endif // CLIENTSHELL