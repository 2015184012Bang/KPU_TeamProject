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

namespace fs = std::filesystem;