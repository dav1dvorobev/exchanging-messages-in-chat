#include <nlohmann/json.hpp>
#include "SocketShell.h"
#include "file_utils.h"

using json = nlohmann::json;

std::string buf;

void readFromServer(SocketShell clientSocket);
bool handleAuthorization(SocketShell clientSocket, const int& preload_history);

int main(int argc, char** argv) {
    std::ifstream file("config.json");
    if (!file) {
        std::cerr << "Cannot opening file" << std::endl;
        return -1;
    }
    json config = json::parse(file);
    const std::string address = config["server_address"];
    const int port = config["port"];
    const int preload_history = config["preload_history"];
    try {
        SocketShell clientSocket(address, port);

        bool authorized = handleAuthorization(clientSocket, preload_history);
        if (!authorized) { return -1; }

        std::thread readThread(readFromServer, clientSocket);
        readThread.detach();
        while (true) {
            input(buf);
            if (buf == "!q") { break; }
            std::cout << MOVE_UP;
            if (validation(buf)) { sendString(clientSocket, buf); }
            else { std::cout << CLEAR_LINE << setColor("validation error", { 255, 0, 0 }) << ENDL; }
        }
        close(clientSocket);
    }
    catch (std::exception& e) {
        logging(e.what(), config["logging_path"]);
        throw;
    }
    return 0;
}

void readFromServer(SocketShell clientSocket) {
    while (true) {
        std::string message = readString(clientSocket);
        std::cout << CLEAR_LINE << message;
        if (!buf.empty()) {
            std::cout << buf;
            std::cout.flush();
        }
    }
}
bool handleAuthorization(SocketShell clientSocket, const int& preload_history) {
    // ASK IF USER IS ALREADY REGISTERED
    std::cout << "Are you a new user? [y/n]: ";
    std::string answer;
    std::getline(std::cin, answer);
    bool isNewUser = answer[0] == 'y' || answer[0] == 'Y';
    bool isOldUser = answer[0] == 'n' || answer[0] == 'N';
    while (!isNewUser && !isOldUser) {
        std::cout << "Wrong answer, try again: ";
        std::getline(std::cin, answer);
        isNewUser = answer[0] == 'y' || answer[0] == 'Y';
        isOldUser = answer[0] == 'n' || answer[0] == 'N';
    }
    // REGISTER NEW USER
    if (isNewUser) {
        // Get new user credentials
        std::string newUserLogin;
        std::string newUserPassword;
        std::string newUserPasswordRepeat;

        sendString(clientSocket, "new");

        // get new user login
        std::cout << "Your login: ";
        sendString(clientSocket, input());

        // get new user password
        bool passwordsMatch = false;
        do {
            std::cout << "Your password: ";
            newUserPassword = input("hidden");
            std::cout << "Repeat your password: ";
            newUserPasswordRepeat = input("hidden");

            passwordsMatch = newUserPassword == newUserPasswordRepeat;
            if (!passwordsMatch) {
                std::cout << "Passwords do not match, try again" << std::endl;
            }
        } while (!passwordsMatch);

        sendString(clientSocket, newUserPassword);

        buf = readString(clientSocket);
        bool isUserRegistered = buf == setColor("User successfully registered", { 0, 255, 0 });
        if (isUserRegistered) {
            send(clientSocket, &preload_history, sizeof(int), 0);
            return true;
        }
        else {
            std::cout << buf << std::endl;
            return false;
        }
    }

    // SIGN IN OLD USER
    while (true) {
        sendString(clientSocket, "old");
        std::cout << "login: ";
        sendString(clientSocket, input());
        std::cout << "password: ";
        sendString(clientSocket, input("hidden"));
        buf = readString(clientSocket);
        if (buf == "accept") {
            send(clientSocket, &preload_history, sizeof(int), 0);
            break;
        }
        else { std::cout << buf << std::endl; }
    }

    return true;
}