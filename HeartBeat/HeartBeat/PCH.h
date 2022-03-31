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
#include <map>
#include <unordered_map>
#include <array>
#include <deque>
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
using std::unordered_map;
using std::map;
using std::array;
using std::deque;
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

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "DirectXTex")
#pragma comment(lib, "DirectXTK12")

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXTK12/SimpleMath.h>
#include <DirectXTex.h>
#include <DirectXTex.inl>
#include "d3dx12.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace DirectX::PackedVector;
using namespace DirectX::SimpleMath;

#define SERVER_PORT 9000