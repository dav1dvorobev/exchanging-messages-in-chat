#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

#include <nlohmann/json.hpp>
#include "SocketShell.h"
#include "file_utils.h"

using json = nlohmann::json;

struct ClientShell {
    std::string login;
    SocketShell clientSocket;
    ClientShell(std::string login, SocketShell clientSocket) :login(login), clientSocket(clientSocket) {}
    operator SocketShell() const { return clientSocket; }
};

namespace global {
    std::mutex clientMutex;
    std::mutex historyMutex;
    std::vector<ClientShell> clients;
    std::map<std::string, std::vector<size_t>> colors = {
        {"red", {255, 0, 0}},
        {"orange", {255, 165, 0}},
        {"yellow", {255, 255, 0}},
        {"green", {0, 255, 0}},
        {"bright pink", {255, 20, 147}},
        {"cyan", {0, 255, 255}},
        {"magenta", {255, 0, 255}}
    };
    std::vector<bool> assigned_colors = { false, false, false, false, false, false, false };
}
void initAssignedColorsWithDBFile() {
    std::ifstream file("users.json");
    if (!file) {
        std::cerr << "Cannot open database file users.json" << std::endl;
        return;
    }
    json users = json::parse(file);
    file.close();
    auto colors_it = global::colors.begin();
    for (auto user : users) {
        std::string color = user["color"];
        for (auto color_it = global::colors.begin(); color_it != global::colors.end(); ++color_it) {
            if (color_it->first == color) {
                global::assigned_colors[std::distance(global::colors.begin(), color_it)] = true;
                break;
            }
        }
    }
}
std::string status(const std::string& __login, const std::string& __password) {
    for (auto client : global::clients) {
        if (client.login == __login) {
            return setColor("already online", { 255, 0, 0 });
        }
    }
    std::ifstream file("users.json");
    json users = json::parse(file);
    file.close();
    try {
        std::string password = users[__login]["password"];
        if (password != __password) {
            return setColor("wrong password", { 255, 0, 0 });
        }
    }
    catch (...) { return setColor("wrong login", { 255, 0, 0 }); }
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
    if (history.tellp() != 0) {
        history << std::endl;
    }
    history << __buf;
    history.close();
    global::historyMutex.unlock();
    global::clientMutex.lock();
    for (auto client : global::clients) {
        sendString(client, __buf + ENDL);
    }
    global::clientMutex.unlock();
}
std::string registerUser(const std::string& __login, const std::string& __password) {
    // ADD USER TO DATABASE
    // read database
    std::ifstream file("users.json");
    if (!file) {
        std::cerr << "Cannot open database file users.json" << std::endl;
        return "Cannot open database file users.json";
    }
    json users = json::parse(file);
    file.close();

    // check if user already exists
    if (users.contains(__login)) {
        return "User already exists";
    }

    // check if all colors are used
    size_t color_counter = 0;
    for (auto color : global::assigned_colors) {
        if (color) {
            ++color_counter;
        }
    }
    if (color_counter == global::assigned_colors.size()) {
        return "All colors are used";
    }
    // get random color index
    size_t colors_num = global::assigned_colors.size();
    std::srand(std::time(nullptr));
    // check if color is already used
    size_t color_idx = std::rand() % colors_num;
    while (global::assigned_colors[color_idx]) {
        color_idx = std::rand() % colors_num;
    }

    // get corresponding color
    size_t idx = 0;
    for (auto color : global::colors) {
        if (idx == color_idx) {
            users[__login] = {
                {"password", __password},
                {"color", color.first}
            };
            break;
        }
        ++idx;
    }

    // update colors
    global::assigned_colors[color_idx] = true;

    // write to file
    std::ofstream file_out("users.json");
    if (!file_out) {
        std::cerr << "Cannot open database file users.json" << std::endl;
        return "Cannot open database file users.json";
    }
    file_out << users.dump(4); // 4 spaces

    return "User successfully registered";
}

#endif // !CLIENT_SHELL_H