#pragma once
#include "Includes.h"

namespace fs = std::filesystem;

struct fileObj{
	//attributes
	fs::path sourcePath;
	fs::path destPath;
	fs::file_time_type lastWriteTimeSource;
	fs::file_time_type lastWriteTimeDest;
	bool destPathExists;
	uintmax_t fileSize;

	//methods
	fileObj() = default;
	fileObj(fs::path,fs::path,std::uintmax_t, 
		fs::file_time_type, fs::file_time_type);
	fileObj(fs::path, fs::path, std::uintmax_t,
		fs::file_time_type, bool);	
	~fileObj();

	bool operator==(const fileObj&) const;
	bool operator<(const fileObj&) const;
};