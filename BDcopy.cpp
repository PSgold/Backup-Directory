#include "Includes.h"

std::mutex coutMutex;
errorType::errorType(std::u8string file, std::u8string errorStr)
	:file{ file }, errorStr{ errorStr }{}


void errorVecWrapper::pushToVec(errorType& fsError) {
		std::lock_guard<std::mutex> lock(m);
		fsErrorVec.push_back(fsError);
}
std::vector<errorType>& errorVecWrapper::getErrorVec() {
    return fsErrorVec;
}

printLock::printLock(unsigned long long tFCount) : tFCount{ tFCount }, cFCount{ 0 },
    hConsole{ GetStdHandle(STD_OUTPUT_HANDLE) } {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);//populates csbi with console info
        cursorPosToRefresh = csbi.dwCursorPosition;
        cellToDel = (csbi.dwSize.X)*(csbi.dwSize.Y);
}
unsigned long long printLock::getFCount()const{return cFCount;}
unsigned long long printLock::getTotalFileCount()const{return tFCount;}
void printLock::addToFCount(unsigned int processed) { 
    std::lock_guard<std::mutex> lock(m);
    cFCount+=processed; 
}

int startBackup(const fs::path& sourceDir, std::wstring dest, helperClass::log* logFilePtr) {
	logFilePtr->writeLog(u8"\nstartBackup has been called...");
	
	//Gets file count in source directory
	fs::recursive_directory_iterator sourceFileC(sourceDir);
	unsigned int fileCount{0};
	for (sourceFileC; sourceFileC != fs::end(sourceFileC); sourceFileC++) {
		if (!(fs::is_directory((*sourceFileC).path()))) fileCount++;
		
	}

	//Loop through all directories and files recursively in source path
	std::vector<fileObj>fileVec{fileCount};//Vector to add fileObjs to and it helpers
	std::vector<fileObj>* fileVecPtr{&fileVec};//Pointer to fileObjs vector
	unsigned vecPointer{0};//fileVec current index
	fs::recursive_directory_iterator sourceDirIt(sourceDir);//iterartor to source obj that will iterate to all files and dirs in source recursively
	for (sourceDirIt; sourceDirIt != fs::end(sourceDirIt); sourceDirIt++) {
		//If item is a directory
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
			}
		}
		//If item is file
		else {
			std::wstring tempDest{ dest };
			std::wstring tempSource{ (*sourceDirIt).path().wstring() };
			tempDest += L"\\";
			tempDest += getEOP(tempSource, sourceDir.wstring().size());//source.size()
			fs::path destSubDir{ tempDest };
			if (fs::exists(destSubDir)) {
				fileObj temp{
					(*sourceDirIt).path(), destSubDir,
					fs::file_size((*sourceDirIt).path()),
					fs::last_write_time((*sourceDirIt).path()),
					fs::last_write_time(destSubDir)
				};
				(*fileVecPtr)[vecPointer] = temp; vecPointer++;
			}
			else {
				fileObj temp{
					(*sourceDirIt).path(), destSubDir,
					fs::file_size((*sourceDirIt).path()),
					fs::last_write_time((*sourceDirIt).path()),0
				};
				(*fileVecPtr)[vecPointer] = temp; ++vecPointer;
			}
		}
	}
	std::sort(fileVecPtr->begin(), fileVecPtr->end());
	std::vector<fileObj>::reverse_iterator fileVecIt{ fileVecPtr->rbegin() };
	std::wcout << L"Source has " << fileCount << " files: checking and backing up when necessary."
		<< L"\nPlease be patient and wait for it to print below \"Finished Backup\".\n\n";
	printLock print{ fileCount }; printLock* printPtr{ &print };
	errorVecWrapper EVW{}; errorVecWrapper* EVWPtr{ &EVW };
	logFilePtr->writeLog(u8"\nFiles created\\updated:");
	std::thread copyThread1{copy1,fileVecIt, fileCount, logFilePtr,EVWPtr,printPtr};
	std::thread copyThread2{copy2,fileVecIt, fileCount, logFilePtr, EVWPtr,printPtr};
	displayProgress(print);
	copyThread1.join();copyThread2.join();//progress.join();

	if (!((EVW.getErrorVec()).empty())) {
		logFilePtr->writeLog(u8"\n");
		logFilePtr->writeLog(u8"Error checking\\copying the follwing files:");
		std::vector<errorType>tempVec{ EVW.getErrorVec() };
		for (int c{ 0 }; c < EVW.getErrorVec().size(); c++) {
			std::u8string temp{ EVW.getErrorVec().at(c).file };
			temp += u8" - "; temp += EVW.getErrorVec().at(c).errorStr;
			logFilePtr->writeLog(temp);
		}
		std::wcout << L"(Error checking\\copying certain files. Please check log for more information)\n\n";
	}
	std::wcout<<L"Finished Backup!"<<std::endl;
	return 1;
}

std::wstring getEOP(const std::wstring& path, const size_t& count) {
	std::wstring pathPart2{};
	for (size_t c{ 0 }; c <= path.size(); c++) {
		if (c > (count - 1))pathPart2 += path[c];
	}
	return pathPart2;
}

void copy1(std::vector<fileObj>::reverse_iterator fileVecIt, const int size,
	helperClass::log* logFile, errorVecWrapper* EVW,printLock* printPtr
) {
	int sizeHalf;
	if (size % 2 == 0)sizeHalf = size / 2;
	else sizeHalf = (size / 2) + 1;

	for (int c{ 0 }; c < sizeHalf; c++) {
		//printPtr->addToFCount(); printPtr->print(fileVecIt->sourcePath.wstring(),0);
		try {
			if (fs::copy_file(fileVecIt->sourcePath, fileVecIt->destPath, fs::copy_options::update_existing)) {
				std::wstring logStrW{ L"\"" };
				logStrW += fileVecIt->destPath.wstring();
				logStrW.pop_back(); logStrW += L"\"";
				(*logFile).writeLog(logStrW);
				//printPtr->addToFCount();
			}
		}
		catch (std::filesystem::filesystem_error& fsError) {
			try {
				if (copyTry2(fileVecIt)) {
					std::wstring logStrW{ L"\"" };
					logStrW += fileVecIt->destPath.wstring();
					logStrW.pop_back(); logStrW += L"\"";
					logFile->writeLog(logStrW);
					//printPtr->addToFCount();
				}
			}
			catch (errorType& error) { EVW->pushToVec(error); }
		}
		if (c != sizeHalf - 1)fileVecIt += 2;
		printPtr->addToFCount();
	}
}

void copy2(std::vector<fileObj>::reverse_iterator fileVecIt, const int size,
	helperClass::log* logFile, errorVecWrapper* EVW,printLock* printPtr
) {
	int sizeHalf{ size / 2 };
	fileVecIt++;
	for (int c{ 0 }; c < sizeHalf; c++) {
		//print.addToFCount(); print.print(fileVecIt->sourcePath.wstring(),1);
		try {
			if (fs::copy_file(fileVecIt->sourcePath, fileVecIt->destPath, fs::copy_options::update_existing)) {
				std::wstring logStrW{ L"\"" };
				logStrW += fileVecIt->destPath.wstring();
				logStrW.pop_back(); logStrW += L"\"";
				logFile->writeLog(logStrW);
				//printPtr->addToFCount();
			}
		}
		catch (std::filesystem::filesystem_error& fsError) {
			try {
				if (copyTry2(fileVecIt)) {
					std::wstring logStrW{ L"\"" };
					logStrW += fileVecIt->destPath.wstring();
					logStrW.pop_back(); logStrW += L"\"";
					logFile->writeLog(logStrW);
					//printPtr->addToFCount();
				}
			}
			catch (errorType& error) { EVW->pushToVec(error); }
		}
		if (c != sizeHalf - 1)fileVecIt += 2;
		printPtr->addToFCount();
	}
}

bool copyTry2(std::vector<fileObj>::reverse_iterator& fileVecIt) {
	if (fileVecIt->destPathExists) {
		if (fileVecIt->lastWriteTimeSource > fileVecIt->lastWriteTimeDest) {
			std::ifstream source{ fileVecIt->sourcePath,std::ios::binary };
			std::ofstream dest{ fileVecIt->destPath,std::ios::binary | std::ios::trunc };

			if (!source.is_open()||!dest.is_open()) {
				source.close(); dest.close();
				errorType error{
					fileVecIt->sourcePath.u8string(),
					u8"fs::copy_file and fstream open both failed!"
				}; throw error;
			}
			dest << source.rdbuf();
			source.close(); dest.close();
			return 1;
		}
		else return 0;
	}
	else {
		std::ifstream source{ fileVecIt->sourcePath,std::ios::binary };
		std::ofstream dest{ fileVecIt->destPath,std::ios::binary };

		if (!source.is_open() || !dest.is_open()) {
			source.close(); dest.close();
			errorType error{
				fileVecIt->sourcePath.u8string(),
				u8"fs::copy_file and fstream open both failed!"
			}; throw error;
		}
		dest << source.rdbuf();
		source.close(); dest.close();
		return 1;
	}
}

void displayProgress(const printLock& print){
	const char startBracket{L'['}, endBracket{L']'}, fillChar{L'0'};
	const unsigned short numOfStep{4}, zerosFill{50};
	const unsigned short filesPerStep{static_cast<const unsigned short>(((print.getTotalFileCount())/numOfStep))};
	unsigned short percent{10}, zerosToFillStrOffset{0}, currentStep{1};unsigned short zerosToFill[]{5,10};
	std::wstring zerosToFillStr;zerosToFillStr.reserve(50);zerosToFillStr.insert(zerosToFillStrOffset,zerosToFill[0],fillChar);
	zerosToFillStrOffset+=zerosToFill[0];
	std::wcout<<HIDECURSOR<<SAVECURSOR<<"Progress: "<<percent<<L"%\n"<<startBracket<<std::setw(zerosFill)<<std::left
	<<zerosToFillStr<<endBracket<<RESTORECURSOR;
	std::chrono::milliseconds msToSleep{300};
	std::this_thread::sleep_for(msToSleep);
	while ((currentStep<=5)){
		switch(currentStep){
			case 1:{
				if (print.getFCount()>(filesPerStep*(numOfStep-3))){
					++currentStep;percent+=20;
					zerosToFillStr.insert(zerosToFillStrOffset,zerosToFill[1],fillChar);zerosToFillStrOffset+=zerosToFill[1];
					std::wcout<<RESTORECURSOR<<"Progress: "<<percent<<L"%\n"<<startBracket<<std::setw(zerosFill)
					<<std::left<<zerosToFillStr<<endBracket;std::wcout.flush();
					std::this_thread::sleep_for(msToSleep);
				}
				break;
			}
			case 2:{
				if (print.getFCount()>(filesPerStep*(numOfStep-2))){
					++currentStep;percent+=20;
					zerosToFillStr.insert(zerosToFillStrOffset,zerosToFill[1],fillChar);zerosToFillStrOffset+=zerosToFill[1];
					std::wcout<<RESTORECURSOR<<"Progress: "<<percent<<L"%\n"<<startBracket<<std::setw(zerosFill)
					<<std::left<<zerosToFillStr<<endBracket;std::wcout.flush();
					std::this_thread::sleep_for(msToSleep);
				}
				break;
			}
			case 3:{
				if (print.getFCount()>(filesPerStep*(numOfStep-1))){
					++currentStep;percent+=20;
					zerosToFillStr.insert(zerosToFillStrOffset,zerosToFill[1],fillChar);zerosToFillStrOffset+=zerosToFill[1];
					std::wcout<<RESTORECURSOR<<"Progress: "<<percent<<L"%\n"<<startBracket<<std::setw(zerosFill)
					<<std::left<<zerosToFillStr<<endBracket;std::wcout.flush();
					std::this_thread::sleep_for(msToSleep);
				}
				break;
			}
			case 4:{
				if (print.getFCount()>=(filesPerStep*(numOfStep))){
					++currentStep;percent+=20;
					zerosToFillStr.insert(zerosToFillStrOffset,zerosToFill[1],fillChar);zerosToFillStrOffset+=zerosToFill[1];
					std::wcout<<RESTORECURSOR<<"Progress: "<<percent<<L"%\n"<<startBracket<<std::setw(zerosFill)
					<<std::left<<zerosToFillStr<<endBracket;std::wcout.flush();
					std::this_thread::sleep_for(msToSleep);
				}
				break;
			}
			case 5: if(print.getFCount()==print.getTotalFileCount())++currentStep;break;
			default:break;
		}
	}
	percent += 10;
	zerosToFillStr.insert(zerosToFillStrOffset,zerosToFill[0],fillChar);
	std::wcout<<RESTORECURSOR<<"Progress: "<<percent<<L"%\n"<<startBracket<<std::setw(zerosFill)
	<<std::left<<zerosToFillStr<<endBracket<<L"\n\n"<<SHOWCURSOR;
}