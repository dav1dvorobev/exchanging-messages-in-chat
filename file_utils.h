#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <thread>
#include <string>
#include <vector>
#include <ctime>
#include <mutex>

#define BLINK "\x1B[5m"
#define CLEAR_LINE "\r\x1B[K"
#define MOVE_UP "\x1B[1A"
#define RESET "\x1B[0m"
#define ENDL  "\r\n"

std::mutex loggingMutex;
void logging(const std::string& __message, const std::string& __path) {
    loggingMutex.lock();
    std::ofstream LOGFILE(__path, std::ios_base::app);
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);
    if (LOGFILE.tellp() != 0) { LOGFILE << std::endl; }
    LOGFILE << '['
        << std::setfill('0') << std::setw(2) << localTime->tm_mday << '-'
        << std::setfill('0') << std::setw(2) << localTime->tm_mon + 1 << '-'
        << localTime->tm_year + 1900 << ' '
        << std::setfill('0') << std::setw(2) << localTime->tm_hour << ':'
        << std::setfill('0') << std::setw(2) << localTime->tm_min << ':'
        << std::setfill('0') << std::setw(2) << localTime->tm_sec << "] "
        << __message;
    LOGFILE.close();
    loggingMutex.unlock();
}
bool validation(const std::string& __buf) {
    if (__buf.size() > 64) { return false; }
    if (!(__buf.back() == '.' ||
        __buf.back() == '!' ||
        __buf.back() == '?')) {
        return false;
    }
    for (const char& iter : __buf) {
        if (static_cast<int>(iter) < 32) {
            return false;
        }
    }
    return true;
}
void input(std::string& __buf, const char* __mode = "default") {
    char input = '\0';
    __buf.clear();
    system("stty raw");
    while ((input = std::cin.get()) != '\r') {
        if (strcmp(__mode, "hidden") == 0) { std::cerr << "\b*"; }
        if (input == '\177') {
            std::cout << "\b\b  \b\b";
            if (!__buf.empty()) {
                std::cout << "\b \b";
                __buf.pop_back();
            }
        }
        else { __buf += input; }
    }
    system("stty cooked");
    while (__buf.back() == ' ') { __buf.pop_back(); }
    std::cout << "\b\b  \b\b\r\n";
}
std::string input(const char* __mode = "default") {
    std::string __buf;
    input(__buf, __mode);
    return __buf;
}
std::string setColor(const std::string& __buf, const std::vector<size_t>& __RGB = { 0, 0, 0 }) {
    size_t R = __RGB[0],
        G = __RGB[1],
        B = __RGB[2];
    std::string color = "\x1B[38;2;" + std::to_string(R) + ';'
        + std::to_string(G) + ';'
        + std::to_string(B) + 'm';
    return color + __buf + RESET;
}
std::string setColorRandom(const std::string& __buf) {
    std::srand(std::time(nullptr));
    size_t R = rand() % 256,
        G = rand() % 256,
        B = rand() % 256;
    return setColor(__buf, { R, G, B });
}

#endif // !FILE_UTILS_H