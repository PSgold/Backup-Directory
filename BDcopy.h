#pragma once
#include "pch.h"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include "helperClass.h"
#include "fileObj.h"

//class purpose is to wrap the vector of error objs so two threads 
//can safely access the vector 
class errorVecWrapper {
	std::mutex m;
	std::vector <fs::filesystem_error> fsErrorVec{};
	
public:
	void pushToVec(fs::filesystem_error& fsError) {
		std::lock_guard<std::mutex> lock(m);
		fsErrorVec.push_back(fsError);
	}
	std::vector<fs::filesystem_error>& getErrorVec() {
		return fsErrorVec;
	}
};

//will be called by child thread to copy file to dest
//if file doesn't exist or if source file modified date is newer than dest file
void copy1(
	std::vector<fileObj>::reverse_iterator fileVecIt, const int size, 
	helperClass::log* logFile, errorVecWrapper*
);

//called by main thread. does the same as copy1
void copy2(
	std::vector<fileObj>::reverse_iterator& fileVecIt, const int size, 
	helperClass::log* logFile, errorVecWrapper&
);

//returns the part of source path that will be added to dest path
std::wstring getEOP(const std::wstring& path, const size_t& count);

//startBackup takes the original sourcePath as a path obj, 
//the original dest path as std::wstring and a pointer to the logfile obj
int startBackup(const fs::path& sourceDir,std::wstring dest,helperClass::log* logFilePtr) {
	if (dest.back() == L'\\')dest.pop_back();//get rid of end slash in dest path if exists. not necessary with source as it was passed as path obj.

	fs::recursive_directory_iterator sourceDirIt(sourceDir);//iterartor to source obj that will iterate to all files and dirs in source recursively
	std::unique_ptr<std::vector<fileObj>> fileVecPtr{ new std::vector<fileObj> };//creates vector to hold file objs on heap
	errorVecWrapper EVW{};errorVecWrapper* EVWPtr{ &EVW };
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
				logFilePtr->writeLog(tempWrite);
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
	std::sort(fileVecPtr->begin(), fileVecPtr->end());
	std::vector<fileObj>::reverse_iterator fileVecIt{ fileVecPtr->rbegin() };
	int vecSize{ static_cast<int>(fileVecPtr->size()) };
	std::wcout << L"Source has " << vecSize << " files: checking and backing up when necessary."
		<< L"\nPlease be patient...";
	logFilePtr->writeLog("Files created\\updated:");
	std::thread thread2{ copy1,fileVecIt, vecSize, logFilePtr,EVWPtr };
	copy2(fileVecIt, vecSize, logFilePtr,EVW);
	thread2.join();

	if(!((EVW.getErrorVec()).empty())){
		logFilePtr->writeLog("\n");
		logFilePtr->writeLog("Error checking\\copying the follwing files:");
		std::vector<fs::filesystem_error>tempVec{ EVW.getErrorVec() };
		for (int c{ 0 }; c < tempVec.size(); c++) {
			std::string tempError{ tempVec.at(c).what() };
			logFilePtr->writeLog(tempError);
			logFilePtr->writeLog("\n");
		}
		std::wcout << L"\n\n(Error checking\\copying certain files. Please check log for more information)";
	}
	return 1;
}

std::wstring getEOP(const std::wstring& path, const size_t& count) {
	std::wstring pathPart2{};
	for (size_t c{ 0 }; c <= path.size(); c++) {
		if (c > (count-1))pathPart2 += path[c];
	}
	return pathPart2;
}

void copy1(std::vector<fileObj>::reverse_iterator fileVecIt, const int size, 
	helperClass::log* logFile, errorVecWrapper* EVW
) {
	for (int c{ 1 }; c <= size; c++) {
		if (c % 2 != 0) {
			try {
				if (fs::copy_file(fileVecIt->sourcePath, fileVecIt->destPath, fs::copy_options::update_existing)) {
					std::wstring logStrW{ L"\"" };
					logStrW += fileVecIt->destPath.wstring();
					logStrW.pop_back(); logStrW += L"\"";
					(*logFile).writeLog(logStrW);
				}
			}
			catch (std::filesystem::filesystem_error& fsError) {
				EVW->pushToVec(fsError);
			}
		}
		fileVecIt++;
	}
}

void copy2(std::vector<fileObj>::reverse_iterator& fileVecIt, const int size,
	helperClass::log* logFile, errorVecWrapper& EVW
) {
	fileVecIt++;
	for (int c{ 2 }; c <= size; c++) {
		if (c % 2 == 0) {
			try {
				if (fs::copy_file(fileVecIt->sourcePath, fileVecIt->destPath, fs::copy_options::update_existing)) {
					std::wstring logStrW{ L"\"" };
					logStrW += fileVecIt->destPath.wstring();
					logStrW.pop_back(); logStrW += L"\"";
					logFile->writeLog(logStrW);
				}
			}
			catch (std::filesystem::filesystem_error& fsError) {
				EVW.pushToVec(fsError);
			}
		}
		fileVecIt++;
	}
}