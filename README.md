# Backup-Directory
This is a console program for Windows written in C++.
Visual C++ 2015 x64 or higher is required to run the program.
The program will use two threads doing the backup.

Backs up source directory to destination recursively.
If the directory already exists at the same point in the destination tree, it will not be overwritten
If a file already exists at the same point in the destination tree it will only be overwritten if the source file modified time is newer than the destination file modified time.
A log file is created in %temp% specifying if any new directories were created and if any files were created/updated at destination.

The source files have also been uploaded here.
