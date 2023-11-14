#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <thread>
#include <string>
#include <ctime>
#include <mutex>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLINK "\033[5m"
#define CLEAR_LINE "\r\33[K"
#define UP "\033[1A"
#define RESET "\033[0m"
#define ENDL "\r\n"

void logging(const std::string& __message, const std::string& __path){
    std::mutex loggingMutex;
    std::lock_guard<std::mutex> lock(loggingMutex);
    std::ofstream LOGFILE(__path, std::ios_base::app);
    std::time_t currentTime = std::time(nullptr);
    std::tm *localTime = std::localtime(&currentTime);
    if(LOGFILE.tellp() != 0){LOGFILE << std::endl;}
    LOGFILE << '['
            << std::setfill('0') << std::setw(2) << localTime->tm_mday << '-'
            << std::setfill('0') << std::setw(2) << localTime->tm_mon + 1 << '-'
            << localTime->tm_year + 1900 << ' '
            << std::setfill('0') << std::setw(2) << localTime->tm_hour << ':'
            << std::setfill('0') << std::setw(2) << localTime->tm_min << ':'
            << std::setfill('0') << std::setw(2) << localTime->tm_sec << "] "
    << __message;
    LOGFILE.close();
}
bool validation(const std::string& __buf){
    if(__buf.size() > 64){return false;}
    if(!(__buf.back() == '.' || 
       __buf.back() == '!' ||
       __buf.back() == '?')){return false;}
    for(const char& iter : __buf){
        if(static_cast<int>(iter) < 32){
            return false;
        }
    }
    return true;
}
void input(std::string& __buf, const char* __mode = "default"){
    char input = '\0';
    __buf.clear();
    system("stty raw");
    while((input = std::cin.get()) != '\r'){
        if(strcmp(__mode, "hidden") == 0){std::cerr << "\b*";}
        if(input == '\177'){
            std::cout << "\b\b  \b\b";
            if(!__buf.empty()){
                std::cout << "\b \b";
                __buf.pop_back();
            }
        }else{__buf += input;}
    }
    system("stty cooked");
    while(__buf.back() == ' '){__buf.pop_back();}
    std::cout << "\b\b  \b\b\r\n";
}
std::string input(const char* __mode = "default"){  
    std::string __buf;
    input(__buf, __mode);
    return __buf;
}