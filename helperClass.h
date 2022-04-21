#pragma once
#include "Includes.h"

enum class flags { null, open, error };

namespace helperClass {
	
	class log {
	  public:
		log() = default;
		log(std::wstring filePath);
		~log();

		void setFilePath(std::wstring);
		void writeLog(std::u8string);
		void writeLog(std::wstring&);
		flags checkFlag();

	  private:
		std::wstring filePath;
		std::ofstream logFile;
		flags flag{ flags::null };
		std::mutex fileMutex;
	};
}