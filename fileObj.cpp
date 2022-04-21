#include "Includes.h"

fileObj::fileObj(fs::path path1,fs::path path2, std::uintmax_t size,
			fs::file_time_type path1Time,fs::file_time_type path2Time
) :sourcePath{ path1 }, destPath{ path2 }, fileSize{ size }, 
lastWriteTimeSource{ path1Time }, lastWriteTimeDest{path2Time},
destPathExists{1}{}

fileObj::fileObj(fs::path path1, fs::path path2, std::uintmax_t size,
	fs::file_time_type path1Time, bool destExists
) : sourcePath{ path1 }, destPath{ path2 }, fileSize{ size },
lastWriteTimeSource{ path1Time }, destPathExists{destExists}{}


fileObj::~fileObj(){}


bool fileObj::operator==(const fileObj& file) const{
	return this->sourcePath == file.sourcePath;
}

bool fileObj::operator<(const fileObj& file) const{
	return (this->fileSize) < (file.fileSize);
}
