#pragma once

#pragma warning(disable:4819)
#pragma comment(lib, "fmt")
#pragma comment(lib, "spdlog")
#pragma comment(lib, "DirectXTex")
#pragma comment(lib, "DirectXTK12")

#include "Log.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <entt/entt.hpp>

using std::string;
using std::wstring;
using std::vector;
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