#pragma once
#include "pch.h"
#include <filesystem>

namespace fs = std::filesystem;

struct fileObj{

	fs::path sourcePath;
	fs::path destPath;
	//fs::file_time_type lastWriteTime;
	uintmax_t fileSize;

	fileObj(fs::path,fs::path,std::uintmax_t);
	~fileObj();


	bool operator==(const fileObj&) const;
	bool operator<(const fileObj&) const;
};

