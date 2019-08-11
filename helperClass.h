#pragma once
#include "pch.h"
#include <fstream>
#include <string>
#include <mutex>

enum class flags { null, open, error };

namespace helperClass {
	
	class log {
	  public:
		log() = default;
		log(std::wstring filePath);
		~log();

		void setFilePath(std::wstring);
		void writeLog(std::string);
		void writeLog(std::wstring&);
		flags checkFlag();

	  private:
		std::wstring filePath;
		std::ofstream logFile;
		flags flag{ flags::null };
		std::mutex fileMutex;
	};

	log::log(std::wstring filePath) :filePath{ filePath } {
		logFile.open((*this).filePath);
		if (logFile.is_open()) { flag = flags::open; }
		else { flag = flags::error; }
	}

	log::~log() {
		logFile.close();
		if (std::filesystem::is_empty(filePath))std::filesystem::remove(filePath);
	}

	void log::setFilePath(std::wstring filePath) {
		(*this).filePath = filePath;
		logFile.open((*this).filePath);
		if (logFile.is_open()) { flag = flags::open; }
		else { flag = flags::error; }
	}

	void log::writeLog(std::string content) {
		std::lock_guard<std::mutex> lock(fileMutex);
		logFile << content << "\n";
	}

	void log::writeLog(std::wstring& contentW) {
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

	flags log::checkFlag() { return flag; }
}