#pragma once

#pragma warning(disable:4819)
#pragma comment(lib, "fmt")
#pragma comment(lib, "spdlog")
#pragma comment(lib, "ws2_32")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <unordered_map>
#include <array>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <entt/entt.hpp>

#include "Log.h"

using std::string;
using std::wstring;
using std::vector;
using std::queue;
using std::list;
using std::unordered_map;
using std::array;
using std::shared_ptr;
using std::unique_ptr;

using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

namespace fs = std::filesystem;