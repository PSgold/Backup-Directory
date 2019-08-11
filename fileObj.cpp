#include "pch.h"
#include "fileObj.h"

fileObj::fileObj(fs::path path1,fs::path path2, std::uintmax_t size):
	sourcePath{ path1 }, destPath{ path2 }, fileSize{ size }{}

fileObj::~fileObj(){}


bool fileObj::operator==(const fileObj& file) const{
	return this->sourcePath == file.sourcePath;
}

bool fileObj::operator<(const fileObj& file) const{
	return (this->fileSize) < (file.fileSize);
}
