#pragma once
#include "pch.h"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include "helperClass.h"
#include "fileObj.h"

void copy1(std::vector<fileObj>::reverse_iterator fileVecIt, const int size, helperClass::log* logFile);
void copy2(std::vector<fileObj>::reverse_iterator& fileVecIt, const int size, helperClass::log& logFile);
std::wstring getEOP(const std::wstring& path, const size_t& count);

int startBackup(const fs::path& sourceDir,std::wstring dest,helperClass::log& logFile) {
	if (dest.back() == L'\\')dest.pop_back();

	fs::recursive_directory_iterator sourceDirIt(sourceDir);
	std::unique_ptr<std::vector<fileObj>> fileVecPtr{ new std::vector<fileObj> };
	for (sourceDirIt; sourceDirIt != fs::end(sourceDirIt); sourceDirIt++) {
		if (fs::is_directory((*sourceDirIt).path())) {
			std::wstring tempDest{ dest };
			std::wstring tempSource{ (*sourceDirIt).path().wstring() };
			tempDest += L"\\";
			tempDest += getEOP(tempSource, sourceDir.wstring().size());
			fs::path destSubDir{ tempDest };
			if (fs::exists(destSubDir) && fs::is_directory(destSubDir));
			else {
				fs::create_directories(destSubDir);
				std::wstring tempWrite{ L"Directory Created: " };
				tempWrite += destSubDir;
				logFile.writeLog(tempWrite);
				std::wcout << tempWrite<<std::endl;
			}
		}
		else {
			std::wstring tempDest{ dest };
			std::wstring tempSource{ (*sourceDirIt).path().wstring() };
			tempDest += L"\\";
			tempDest += getEOP(tempSource, sourceDir.wstring().size());//source.size()
			fs::path destSubDir{ tempDest };
			(*fileVecPtr).emplace_back(
				(*sourceDirIt).path(), destSubDir,
				fs::file_size((*sourceDirIt).path())
			);
		}
	}
	std::wcout << L"\n\n";
	logFile.writeLog("\n");
	std::sort(fileVecPtr->begin(), fileVecPtr->end());
	std::vector<fileObj>::reverse_iterator fileVecIt{ fileVecPtr->rbegin() };
	int vecSize{ static_cast<int>(fileVecPtr->size()) };
	std::wcout << L"Source has " << vecSize << " files: checking and backing up when necessary."
		<< L"\nPlease be patient...";
	logFile.writeLog("Files created\\updated:");
	std::thread thread2{ copy1,fileVecIt, vecSize, &logFile };
	copy2(fileVecIt, vecSize, logFile);
	
	thread2.join();
	return 1;
}

std::wstring getEOP(const std::wstring& path, const size_t& count) {
	std::wstring pathPart2{};
	for (size_t c{ 0 }; c <= path.size(); c++) {
		if (c > (count-1))pathPart2 += path[c];
	}
	return pathPart2;
}

void copy1(std::vector<fileObj>::reverse_iterator fileVecIt, const int size, helperClass::log* logFile) {
	for (int c{ 1 }; c <= size; c++) {
		if (c % 2 != 0) {
			if (fs::copy_file(fileVecIt->sourcePath, fileVecIt->destPath, fs::copy_options::update_existing)) {
				std::wstring logStrW{ L"\"" };
				logStrW += fileVecIt->destPath.wstring();
				logStrW.pop_back();logStrW += L"\"";
				(*logFile).writeLog(logStrW);
			}
		}
		fileVecIt++;
	}
}

void copy2(std::vector<fileObj>::reverse_iterator& fileVecIt, const int size, helperClass::log& logFile) {
	fileVecIt++;
	for (int c{ 2 }; c <= size; c++) {
		if (c % 2 == 0) {
			if (fs::copy_file(fileVecIt->sourcePath, fileVecIt->destPath, fs::copy_options::update_existing)) {
				std::wstring logStrW{ L"\"" };
				logStrW += fileVecIt->destPath.wstring();
				logStrW.pop_back();logStrW += L"\"";
				logFile.writeLog(logStrW);
			}
		}
		fileVecIt++;
	}
}