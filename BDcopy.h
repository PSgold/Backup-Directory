#pragma once
#include "Includes.h"

struct errorType {
	std::u8string file;
	std::u8string errorStr;
	errorType(std::u8string file, std::u8string errorStr);
};

//class purpose is to wrap the vector of error objs so two threads 
//can safely access the vector 
class errorVecWrapper {
	std::mutex m;
	std::vector <errorType> fsErrorVec{};

	public:
	void pushToVec(errorType& fsError);
	std::vector<errorType>& getErrorVec();
};

class printLock {
	std::mutex m;
	unsigned long long cFCount;//current file count to be updated by both threads
	const unsigned long long tFCount;//total file count
	HANDLE hConsole;//Handle to console output
	COORD cursorPosToRefresh;//cursor position when obj is created
	int cellToDel;//number of console cells to clear from cursorPosToRefresh
	
	public:
	printLock(unsigned long long tFCount);
	void addToFCount(unsigned int processed=1);
	unsigned long long getFCount() const;
	unsigned long long getTotalFileCount() const;
};


//startBackup takes the original sourcePath as a path obj, 
//the original dest path as std::wstring and a pointer to the logfile obj
int startBackup(const fs::path& sourceDir, std::wstring dest, helperClass::log* logFilePtr);

//will be called by child thread to copy file to dest
//if file doesn't exist or if source file modified date is newer than dest file
void copy1(
	std::vector<fileObj>::reverse_iterator fileVecIt, const int size,
	helperClass::log* logFile, errorVecWrapper*, printLock*
);

//called by main thread. does the same as copy1
void copy2(
	std::vector<fileObj>::reverse_iterator fileVecIt, const int size,
	helperClass::log* logFile, errorVecWrapper*, printLock*
);

bool copyTry2(std::vector<fileObj>::reverse_iterator&);

//returns the part of source path that will be added to dest path
std::wstring getEOP(const std::wstring& path, const size_t& count);

void displayProgress(const printLock& print);