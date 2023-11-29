#include "ServerSocket.h"
#include "ClientShell.h"
#include "file_utils.h"

#define PORT 8080
#define LOGGING_PATH "server.log"

void clientHandler(SocketShell clientSocket) {
    std::string new_or_old;
    std::string login;
    std::string password;
    while (true) {
        try {
            new_or_old = readString(clientSocket);
            login = readString(clientSocket);
            password = readString(clientSocket);
            logging("[INFO] HANDLING NEW CONNECTION", LOGGING_PATH);
            logging("[INFO] USER INFO: [" + new_or_old + "]" + "[" + login + "]" + "[" + password + "]", LOGGING_PATH);
        }
        catch (...) {
            close(clientSocket);
            return;
        }
        if (new_or_old == "new") {
            // Add new user to database
            std::string register_status = registerUser(login, password);
            if (register_status == "User successfully registered") {
                sendString(clientSocket, setColor(register_status, { 0, 255, 0 }));
                global::clientMutex.lock();
                preload_history(clientSocket);
                global::clients.push_back({ login, clientSocket });
                global::clientMutex.unlock();
                break;
            }
            else {
                sendString(clientSocket, setColor(register_status, { 255, 0, 0 }));
            }
        }
        else {
            std::string cur_status = status(login, password);
            sendString(clientSocket, cur_status);
            if (cur_status == "accept") {
                global::clientMutex.lock();
                preload_history(clientSocket);
                global::clients.push_back({ login, clientSocket });
                global::clientMutex.unlock();
                break;
            }
        }
    }

    // by this moment user is registered and added to database
    login = getLoginWithColor(login, global::colors);

    sendClients(login + setColor(" connected", { 0, 255, 0 }));
    try {
        while (true) {
            std::string message = readString(clientSocket);
            sendClients(login + ": " + message);
        }
    }
    catch (...) {
        global::clientMutex.lock();
        close(clientSocket);
        // global::clients.erase(global::clients.begin() + idx);

        auto it = std::find_if(global::clients.begin(), global::clients.end(), [&](const ClientShell& client) {
            return client.clientSocket == clientSocket; // Directly compare each client's socket to the socket we're looking for
            });
        if (it != global::clients.end()) {
            global::clients.erase(it);
        }

        global::clientMutex.unlock();
        sendClients(login + setColor(" disconnected", { 255, 0, 0 }));
    }
}
int main() {
    try {
        ServerSocket serverSocket(PORT);
        std::cout << "server start in port " << serverSocket.port() << std::endl;
        logging("[INFO] SERVER START IN PORT " + std::to_string(PORT), LOGGING_PATH);
        logging("[INFO] INITIALIZING COLORS...", LOGGING_PATH);
        initAssignedColorsWithDBFile();
        logging("[INFO] ...COLORS INITIALIZED", LOGGING_PATH);
        while (true) {
            SocketShell clientSocket = serverSocket.acceptConnection();
            std::thread clientThread(clientHandler, clientSocket);
            clientThread.detach();
        }
    }
    catch (std::exception& e) {
        logging(e.what(), LOGGING_PATH);
        throw;
    }
    return 0;
}