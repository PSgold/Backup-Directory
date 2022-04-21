#pragma once

#define SAVECURSOR L"\x1b\x37"
#define RESTORECURSOR L"\x1b\x38"
#define HIDECURSOR L"\x1b[?25l"
#define SHOWCURSOR L"\x1b[?25h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <filesystem>
#include <memory>
#include <mutex>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstring>
#include "windows.h"
#include "io.h"
#include "fcntl.h"
#include "fileObj.h"
#include "helperClass.h"
#include "BDcopy.h"