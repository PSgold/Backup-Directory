// BackupDirectory.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <mutex>
#include <ctime>
#include <cstring>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "helperClass.h"
#include "fileObj.h"
#include "BDcopy.h"

//struct obj used to keep status of source and dest path status
struct pathFlags {
	bool source{0};
	bool dest{0};
};
pathFlags pathFlag;
bool logFileState{ 0 };//0 closed, 1 open

//local functions
void displayPreamble();
wchar_t displayMenu();
void strToArrayW(const std::string&, wchar_t*);//copies std::string to wchar array
std::wstring getSourceDir();
std::wstring getDestDir();
void pause(char);
bool setTempPathStr(std::wstring&);

class logFileError{
public: void printError(){std::wcout << L"Failed to open log file. Aborting program."<<std::endl;}
};

int wmain(int argc, wchar_t* argv[]) {
	
	//set console input and output to unicode
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);

	//disable console quick edit mode so mouse click won't pause the process
	DWORD consoleMode;
	HANDLE inputHandle{ GetStdHandle(STD_INPUT_HANDLE) };
	GetConsoleMode(inputHandle, &consoleMode);
	SetConsoleMode(inputHandle, consoleMode & (~ENABLE_QUICK_EDIT_MODE));

	//set console fonst to "Courier New"
	CONSOLE_FONT_INFOEX cf{};
	cf.cbSize = sizeof(cf);
	HANDLE outputHandle{ GetStdHandle(STD_OUTPUT_HANDLE) };
	GetCurrentConsoleFontEx(outputHandle, 0, &cf);
	wcscpy_s(cf.FaceName, L"Courier New");
	SetCurrentConsoleFontEx(outputHandle, 0, &cf);
	


	//If it was run via terminal with 2 params source and dest ======================================================================================================
	if (argc > 1) {
		std::wstring fileName{};
		setTempPathStr(fileName);
		helperClass::log* logFilePtr{ new helperClass::log { fileName } };
		
		std::wstring logTxt{ L"Source path entered: \"" };
		logTxt += argv[1]; logTxt += L"\"";
		logFilePtr->writeLog(logTxt);
		std::wstring logTxt2{ L"Destination path entered: \"" };
		logTxt2 += argv[2]; logTxt2 += L"\"";
		logFilePtr->writeLog(logTxt2);
		
		int sourceSize = std::char_traits<wchar_t>::length (argv[1]);
		int destSize = std::char_traits<wchar_t>::length(argv[2]);
		try {
			for (int c{ 0 }; c <= sourceSize; c++) {
				if (argv[1][c] == '"') throw 798;
			}
			for (int c{ 0 }; c <= destSize; c++) {
				if (argv[2][c] == '"') throw 799;
			}
		}
		catch(int ex){
			delete logFilePtr;
			switch (ex){
			case 798:std::wcout << L"Error: source path cannot have single trailing backslash '\\'. "
				<< "Either use a double trailing backslash '\\\\' or delete the trailing backslash."; break;
			case 799:std::wcout << L"Error: destination path cannot have single trailing backslash '\\'. "
				<< "Either use a double trailing backslash '\\\\' or delete the trailing backslash."; break;
			}
			return 0;
		}
		fs::path source{ argv[1] };
		std::wstring dest{ argv[2] };
		logFilePtr->writeLog("\nstartBackup has been called...");
		startBackup(source, dest, logFilePtr);
		delete logFilePtr; return 0;
	}
	//If it was run via terminal with 2 params source and dest =======================================================================================================
	
	
	
	//Creating source and destination Directory Variables used in main loop
	std::filesystem::path source;
	std::filesystem::path destination;
	std::wstring sourceStr;
	std::wstring destStr;
	wchar_t choice;


	//displays to console introduction explaining what the program does
	displayPreamble(); pause('M');

	//create pointer to log file. This will allow us to open and close log files per our need
	//only one log file will be open at a time
	helperClass::log* logFilePtr{ nullptr };
	
	//Start Main loop
	while (true) {
		if (!logFileState) {
			std::wstring fileName{};
			if (!setTempPathStr(fileName)) {
				std::cout << "Failed to load temp path environment variable. Aborting Program."
					<< std::endl; pause('Q'); return 0;
			}
			else{
				//Creating log file obj with file path name and testing if its open for writing
				logFilePtr = new helperClass::log { fileName };
				try { if (logFilePtr->checkFlag() != flags::open) { throw logFileError{}; } }
				catch (logFileError& ex) { ex.printError(); delete logFilePtr; pause('Q'); return 0; }
				logFileState = 1;
			}
		}
		choice = displayMenu();
		if (choice == L'1') {
			sourceStr = getSourceDir();
			std::wstring logTxt{ L"Source path entered: \"" };
			logTxt += sourceStr; logTxt += L"\"";
			logFilePtr->writeLog(logTxt);
			source = sourceStr;
			try { 
				if (!(std::filesystem::exists(source))) throw 0;
				pathFlag.source = 1;
				logFilePtr->writeLog("Source path exists!");
				std::wcout << L"Source path has been set to \""<<sourceStr<<L"\"\n";
				pause('M');
			}
			catch (int& ex) {
				pathFlag.source = 0;
				std::wstring logTxt{ L"Source path \"" };
				logTxt += sourceStr; logTxt += L"\" does not exist";
				logFilePtr->writeLog(logTxt);
				std::wcout << L"Error: source directory does not exist! Source has been cleared\n";
				pause('M');
			}
			logFilePtr->writeLog("");
			system("cls");
		}
		else if (choice == L'2') {
			destStr = getDestDir();
			std::wstring logTxt{ L"Destination path entered: " };
			logTxt += destStr;
			logFilePtr->writeLog(logTxt);
			destination = destStr;
			try { 
				if (!(std::filesystem::exists(destination))) throw 0; 
				pathFlag.dest = 1;
				logFilePtr->writeLog("Destination path exists!");
				std::wcout << L"Destination path has been set to \"" << destStr << L"\"\n";
				pause('M');
			}
			catch (int& ex) {
				pathFlag.dest = 0;
				std::wstring logTxt{ L"Destination path \"" };
				logTxt += destStr; logTxt += L"\" does not exist";
				logFilePtr->writeLog(logTxt);
				std::wcout << L"Error: destination directory does not exist! Destination has been cleared\n";
				pause('M');
			}
			logFilePtr->writeLog("");
			system("cls");
		}
		else if (choice == L'3') {
			if (pathFlag.source == 1)std::wcout << L"Source path is set to \"" << sourceStr << L"\"\n\n";
			else std::wcout << L"Source path has not been set\n\n";
			pause('M');
		}
		else if (choice == '4') {
			if (pathFlag.dest == 1)std::wcout << L"Destination path is set to \"" << destStr << L"\"\n\n";
			else std::wcout << L"Destination path has not been set\n\n";
			pause('M');
		}
		else if (choice == L'5') {
			if(pathFlag.source == 1 && pathFlag.dest==1){
				pathFlag.source = 0; pathFlag.dest = 0;
				logFilePtr->writeLog("\nstartBackup has been called...");
				startBackup(source,destStr,logFilePtr);
				std::wcout << L"\n\nBackup has finished...\n\n";
				pause('Q'); delete logFilePtr;logFilePtr = nullptr; logFileState = 0;
				
			}
			else if (pathFlag.source == 0 && pathFlag.dest == 0) {
				logFilePtr->writeLog("startBackup was called but source and destination path have not been set");
				std::wcout << L"Source and destination path have not been set\n\n";
				pause('M');
			}
			else if (pathFlag.source==0){
				logFilePtr->writeLog("startBackup was called but source path has not been set");
				std::wcout << L"Source path has not been set\n\n";
				pause('M');
			}
			else if (pathFlag.dest == 0) {
				logFilePtr->writeLog("startBackup was called but destination path has not been set");
				std::wcout << L"Destination path has not been set\n\n";
				pause('M');
			}
		}
		else if (choice == L'6') { pause('Q'); delete logFilePtr; return 0; }
	}
	//End Main loop
	
	delete logFilePtr; return 0;//unnecessary
}

void displayPreamble() {
	std::wcout << std::setw(8) << L"Preamble" << std::endl;
	std::wcout << std::setw(8) << std::setfill(L'=') << L"" << std::endl;
	std::wcout << L"This program will copy the source directory tree and files"
		<< L" to the destination directory.\nIt will not remove any existing"
		<< L" destination directories, rather will only create new ones if they don't already exist.\n"
		<< L"If a file already exists in the the same place at the destination"
		<< L" it will only be truncated if the source file's modified time"
		<< L" is newer than the destination file's modified time." << std::endl << std::endl;
}

wchar_t displayMenu() {
	wchar_t choice;
	std::wstring temp;
	std::wistringstream tempStream{L"0"};
	int counter{ 0 };
	system("cls");
	while (tempStream.str() != L"1" && tempStream.str() != L"2" && 
		   tempStream.str() != L"3" && tempStream.str() != L"4" &&
		   tempStream.str() != L"5" && tempStream.str() != L"6") {
		if (counter > 0) { std::wcout <<L"'"<< temp <<L"'"<< L" is not a valid choice. Please choose again.\n\n"; }
		std::wcout << L"===========Menu===========" << std::endl<<std::endl;
		std::wcout << L"1. Enter source directory\n2. Enter destination directory"
			<<L"\n3. Check source directory\n4. Check destination directory"
			<<L"\n5. Start backup\n6. Quit\n\n";
		std::wcout << std::setw(27) << std::setfill(L'=') << L"" << std::endl;
		std::wcout << L"Please choose (1,2,3,4,5,6): ";
		std::getline(std::wcin, temp);
		tempStream.str(temp);
		system("cls");
		counter++;
	}
	tempStream >> choice; return choice;
}

std::wstring getSourceDir() {
	std::wstring tempSource;
	std::wcout << L"Please enter the full source directory path: ";
	std::getline(std::wcin, tempSource); return tempSource;
}

std::wstring getDestDir() {
	std::wstring tempSource;
	std::wcout << L"Please enter the full destination directory path: ";
	std::getline(std::wcin, tempSource); return tempSource;
}

//copies an std::string to a raw wchar_t array
void strToArrayW(const std::string& str, wchar_t* wstr) {
	for (int c{ 0 }; c <= str.size(); c++) {
		if(c!= str.size())wstr[c] = str[c];
		else wstr[c] = '\0';
	}
}

void pause(char pauseType) {
	std::wstring tmp;
	if(pauseType == 'Q')std::wcout << L"Press Enter to quit... ";
	else if (pauseType == 'M')std::wcout << L"Press Enter for menu... ";
	std::getline(std::wcin, tmp);
}

bool setTempPathStr(std::wstring& fileName) {
	//Get system time, create filepathname with systime in temp folder and create and open log file
	std::time_t sysTime;
	std::time(&sysTime);
	std::tm result;
	std::tm* resultPtr = &result;
	localtime_s(resultPtr, &sysTime);//puts sysTime into tm obj which holds time as calendar time
	//places the tm obj with specific format into string buff, stringBuffO;fails if you try to put it directly into wostringstream 
	std::ostringstream stringBuffO;
	stringBuffO << std::put_time(resultPtr, "%d.%m.%y_%H.%M.%S");
	std::string stringBuffS{ stringBuffO.str() };
	const size_t dateLength{ stringBuffS.size() };
	std::unique_ptr<wchar_t> strBuffW{ new wchar_t[dateLength + 1] };//Create wchar array to transfer stringBuffs content
	strToArrayW(stringBuffS, strBuffW.get());//transfers the time from stringBuffs to strBuffW
	std::array<wchar_t, 50> tempPathW{};//array wchar obj to place the value of GetTempPath - temp environment variable string
	wchar_t* tempPathWPtr = tempPathW.data();//pointer to the raw wchar array in the tempPath array obj
	DWORD testTempPath;//will hold the return value of GetTempPath to test for success
	testTempPath = GetTempPathW(50, tempPathWPtr);//gets env variable value "temp"

	//testing if GetTempPathW succeeded
	if (testTempPath > 50 || testTempPath == 0)return 0;

	//transferring the wchar array obj to wstring obj
	std::wostringstream storeTempPath;
	for (int c{ 0 }; c < tempPathW.size(); c++) {
		if (tempPathW[c] != '\0') {
			storeTempPath << tempPathW[c];
		}
	}

	//creating full file path and name
	fileName = storeTempPath.str();
	fileName += L"BackUpDirectoryLog_";
	for (int c{ 0 }; strBuffW.get()[c] != '\0'; c++) fileName += strBuffW.get()[c];
	fileName += L".txt";

	return 1;
}


//Another way of copying a file with C++ using your own buffer:
//inputfile.seekg(0, inputfile.end);
//int length = inputfile.tellg();
//inputfile.seekg(0, inputfile.beg);
//char* buffer = new char[length];
//inputfile.read(buffer, length);
//outputfile.write(buffer, length);
//delete[] buffer;


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
