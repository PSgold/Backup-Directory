#include "Includes.h"

helperClass::log::log(std::wstring filePath) :filePath{ filePath } {
    logFile.open((*this).filePath);
    if (logFile.is_open()) { flag = flags::open; }
    else { flag = flags::error; }
}

helperClass::log::~log() {
    logFile.close();
    if (std::filesystem::is_empty(filePath))std::filesystem::remove(filePath);
}

void helperClass::log::setFilePath(std::wstring filePath) {
    (*this).filePath = filePath;
    logFile.open((*this).filePath);
    if (logFile.is_open()) { flag = flags::open; }
    else { flag = flags::error; }
}

void helperClass::log::writeLog(std::u8string content) {
    std::lock_guard<std::mutex> lock(fileMutex);
    logFile << reinterpret_cast<const char*> (content.data())<<reinterpret_cast<const char*>(u8"\n");
}

void helperClass::log::writeLog(std::wstring& contentW) {
    std::lock_guard<std::mutex> lock(fileMutex);
    int buffersize{ WideCharToMultiByte(CP_UTF8,0,contentW.data(),-1,NULL,0,NULL, NULL)};
    std::string content(buffersize-1, 'x');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        contentW.data(),
        -1,
        content.data(),
        buffersize,
        NULL, NULL
    );
    logFile << content << "\n";
}

flags helperClass::log::checkFlag() { return flag; }