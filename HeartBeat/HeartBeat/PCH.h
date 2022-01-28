#pragma once

#pragma warning(disable:4819)
#pragma comment(lib, "fmt")
#pragma comment(lib, "spdlog")

#include "Log.h"

#include <string>
#include <array>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <list>
#include <memory>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <entt/entt.hpp>

using std::string;
using std::wstring;
using std::vector;
using std::list;
using std::queue;
using std::unordered_map;
using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::stringstream;

using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

namespace fs = std::filesystem;

#include "Timing.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "SocketUtil.h"
#include "MemoryBitStream.h"
#include "NetworkManager.h"