// BackupDirectory.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Includes.h"

//struct obj used to keep status of source and dest path status
struct pathFlags {
	bool source{0};
	bool dest{0};
};
pathFlags pathFlag;
bool logFileState{ 0 };//0 closed, 1 open

//local functions
bool enableVTS(HANDLE& outputHandle);//enable virtual terminal sequences.
bool disableConsoleQuickEditMode(HANDLE& inputHandle);
void printHelp();
bool checkTwoArgs(wchar_t* argument);
void setPaths(
	std::wstring& source, std::wstring& dest,
	wchar_t* argvS, wchar_t* argvD,wchar_t* argument
);
helperClass::log* openLog();
void strToArrayW(const std::string&, wchar_t*);//copies std::string to wchar array
bool copyWcharArray(wchar_t* source, wchar_t* target, unsigned short size);
bool setTempPathStr(std::wstring&);

class logFileError{
public: void printError(){std::wcout << L"Failed to open log file. Aborting program."<<std::endl;}
};

int wmain(int argc, wchar_t* argv[]) {
	
	//set console input and output to unicode
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	//get standard input and output handle
	HANDLE inputHandle{ GetStdHandle(STD_INPUT_HANDLE) }, outputHandle{GetStdHandle(STD_OUTPUT_HANDLE)};
	if (outputHandle == INVALID_HANDLE_VALUE)return 1;
	if (inputHandle == INVALID_HANDLE_VALUE)return 1;
	//disable console quick edit mod	e so mouse click won't pause the process
	if(!disableConsoleQuickEditMode(inputHandle)){
		std::wcout<<L"Failed to disable console quick edit mode. Aborting!"<<std::endl;
		return 1;
	}
	//enable virtual terminal sequences.
	if(!enableVTS(outputHandle)){
		std::wcout<<L"Failed to enable console virtual terminal sequences. Aborting!"<<std::endl;
		return 1;
	}
	

	//////////////////////////Debug//////////////////////////
	/* std::wstring sourceStr(MAX_PATH,L'\0');
	std::wstring destStr(MAX_PATH,L'\0');
	wchar_t* argument{GetCommandLineW()};
	std::wcout<<L"argc count: "<<argc<<L'\n'<<argument<<L"\n\n"<<argv[1]<<std::endl;
	if(checkTwoArgs(argument)){
		setPaths(sourceStr,destStr,argv[1],argv[2],argument);
		std::wcout<<L"\nArguments are legal:\n"<<sourceStr<<L" ; "<<destStr<<std::endl;
	}
	else {
		std::wcout<<L"Arguments are illegal, aborting."<<std::endl;
	}
	return 0; */
	/////////////////////////Debug/////////////////////////////


	std::unique_ptr<helperClass::log>logFilePtr{openLog()};
	try {
		if(argc == 3) {
			//will hold source and destination dirs
			std::wstring sourceStr(MAX_PATH,L'\0');
			std::wstring destStr(MAX_PATH,L'\0');
			//gets the arguments passed as long Raw wstring
			wchar_t* argument{GetCommandLineW()};
			//sets the paths of sourceStr and destStr
			setPaths(sourceStr,destStr,argv[1],argv[2],argument);
			//Remove trailing backslash
			if(sourceStr.back() == L'\\')sourceStr.pop_back();
			if (destStr.back() == L'\\')destStr.pop_back();
			//construct filesystem path obj for source path and verify dir exists 
			fs::path source{sourceStr};
			fs::path dest{destStr};
			if(!(fs::exists(source)))throw 22;
			else if(!fs::exists(destStr))throw 23;
			//Open logFile and write source and dest paths to log
			std::wstring logTxt{ L"Source path entered: \"" };
			logTxt += sourceStr; logTxt += L"\"";
			logFilePtr->writeLog(logTxt);
			std::wstring logTxt2{ L"Destination path entered: \"" };
			logTxt2 += destStr; logTxt2 += L"\"";
			logFilePtr->writeLog(logTxt2);
			//call startbackup
			startBackup(source,destStr,logFilePtr.get());
		}
		else{
			wchar_t* argument{GetCommandLineW()};
			if(checkTwoArgs(argument)){
				//will hold source and destination dirs
				std::wstring sourceStr(MAX_PATH,L'\0');
				std::wstring destStr(MAX_PATH,L'\0');
				//sets the paths of sourceStr and destStr
				setPaths(sourceStr,destStr,argv[1],argv[2],argument);
				//Remove trailing backslash
				if(sourceStr.back() == L'\\')sourceStr.pop_back();
				if (destStr.back() == L'\\')destStr.pop_back();
				//construct filesystem path obj for source path and verify dir exists 
				fs::path source{sourceStr};
				fs::path dest{destStr};
				if(!(fs::exists(source)))throw 22;
				else if(!fs::exists(destStr))throw 23;
				//Open logFile and write source and dest paths to log
				std::wstring logTxt{ L"Source path entered: \"" };
				logTxt += sourceStr; logTxt += L"\"";
				logFilePtr->writeLog(logTxt);
				std::wstring logTxt2{ L"Destination path entered: \"" };
				logTxt2 += destStr; logTxt2 += L"\"";
				logFilePtr->writeLog(logTxt2);
				//call startbackup
				startBackup(source,destStr,logFilePtr.get());
			}
			else printHelp();
		}
	}
	catch (int ex) {
		switch (ex) {
			case 22:std::wcout << L"Error: source path does not exist!" << std::endl; break;
			case 23:std::wcout << L"Error: dest path does not exist!" << std::endl; break;
			case 82:std::wcout << L"Error: Failed to get temp directory path"<< std::endl; break;
		}
		return 1;
	}
	catch(fs::filesystem_error& error){
		std::u8string errorStr{reinterpret_cast<char8_t>(error.what())};
		logFilePtr->writeLog(errorStr);
	}
	catch(...){logFilePtr->writeLog(u8"General error thrown!");std::wcout<<L"\nGeneral Exception thrown. Aborting!"<<std::endl;}
	return 0;
}

bool disableConsoleQuickEditMode(HANDLE& inputHandle){
	//disable console quick edit mode so mouse click won't pause the process
	DWORD consoleMode;
	if(!GetConsoleMode(inputHandle, &consoleMode))return 0;
	if(!SetConsoleMode(inputHandle, consoleMode & (~ENABLE_QUICK_EDIT_MODE)))return 0;
	return 1;
}

bool enableVTS(HANDLE& outputHandle){
	// Set output mode to handle virtual terminal sequences
	DWORD dwOriginalOutMode;
    if (!GetConsoleMode(outputHandle, &dwOriginalOutMode))return 0;

    DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
    if (!SetConsoleMode(outputHandle, dwOutMode)){
        // we failed to set both modes, try to step down mode gracefully.
        dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
        if (!SetConsoleMode(outputHandle, dwOutMode)){
            // Failed to set any VT mode, can't do anything here.
            return 0;
        }
    }
	return 1;
}


void printHelp() {
	char help[]{
R"*(BackupDirectory Help:
This program will copy the source directory tree and files 
to the destination directory. It will not remove any existing 
destination directories, rather will only create new ones
if they don't already exist. If a file already exists in the 
same place at the destination it will only be truncated if 
the source file's modified time is newer than that of the 
destination file.

Command syntax:
BackupDirectory.exe source destination)*"
	};
	std::wcout << help << '\n' << std::endl;
}

bool checkTwoArgs(wchar_t* argument){
	unsigned short step{0};
	bool twoArgs{0};
	for(unsigned short c{0};argument[c]!=L'\0';++c){
		if(argument[c]==L' '&&argument[c+1]==L'"'&&step==0){++step;++c;}
		else if(argument[c]==L'"'&&step==1)++step;
		else if(argument[c]==L' '&&step==2)++step;
		else if(argument[c]!=L' '&&step==3){twoArgs=1;break;}
	}
	return twoArgs;
}

void setPaths(std::wstring& source, std::wstring& dest,wchar_t* argvS, wchar_t* argvD,wchar_t* argument){
	unsigned short step{0};
	for(unsigned short c{0};argument[c]!=L'\0';++c){
		if(argument[c]==L'"' && step == 0){++step;continue;}
		else if(argument[c]==L'"' && step == 1){
			++step;
			++c;
			for(;argument[c]==L' ';++c){};
			if (argument[c]!=L'"'){source = argvS;++step;continue;}
			else{
				++c;
				wchar_t* sourceStart{argument+c};
				unsigned short sourceSize{1};
				while(argument[c]!= L'"'){++sourceSize;++c;if(sourceSize>260)break;};
				copyWcharArray(sourceStart,source.data(),(sourceSize-1));
			}
		}
		else if(argument[c]!=L' '&& step==2){
			if(argument[c]!=L'"'){
				wchar_t* destStart{argument+c};
				unsigned short destSize{1};
				while(argument[c]!= L'\0'){++destSize;++c;if(destSize>260)break;};
				copyWcharArray(destStart,dest.data(),(destSize-1));
				break;
			}
			else{
				++c;
				wchar_t* destStart{argument+c};
				unsigned short destSize{1};
				while(argument[c]!= L'"'){++destSize;++c;if(destSize>260)break;};
				copyWcharArray(destStart,dest.data(),(destSize-1));
				break;
			}
		}
		else if(argument[c]== L' '&& step==3){	
			for(;argument[c]==L' ';++c){}
			if(argument[c]!=L'"'){dest = argvD;break;}
			else{
				++c;
				wchar_t* destStart{argument+c};
				unsigned short destSize{1};
				while(argument[c]!= L'"'){++destSize;++c;if(destSize>260)break;};
				copyWcharArray(destStart,dest.data(),(destSize-1));
				break;
			}
		}
	}
	unsigned short sourceAlloc{(static_cast<unsigned short>((source.size())-1))}, destAlloc{(static_cast<unsigned short>((dest.size())-1))};
	while(source[sourceAlloc]==L'\0'){source.pop_back();--sourceAlloc;}
	while(dest[destAlloc]==L'\0'){dest.pop_back();--destAlloc;}
}

helperClass::log* openLog(){
	std::wstring fileName{};
	if(!setTempPathStr(fileName))throw 82;
	helperClass::log* logFilePtr{ new helperClass::log { fileName }};
	return logFilePtr;
}

//copies an std::string to a raw wchar_t array
void strToArrayW(const std::string& str, wchar_t* wstr) {
	for (int c{ 0 }; c <= str.size(); c++) {
		if(c!= str.size())wstr[c] = str[c];
		else wstr[c] = '\0';
	}
}

bool copyWcharArray(wchar_t* source, wchar_t* target, unsigned short size){
	if(size>MAX_PATH)return 0;
	for(unsigned short c{0};c<size;++c){
		target[c] = source[c];
	}
	return 1;
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