#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")
#pragma comment(lib, "libtcmalloc_minimal")
#pragma comment(linker, "/include:__tcmalloc")
#pragma comment(lib, "spdlog")
#pragma comment(lib, "fmt")
#pragma comment(lib, "DirectXTK12")

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>
#include <DirectXTK12/SimpleMath.h>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <deque>
#include <queue>
#include <list>
#include <functional>
#include <thread>
#include <queue>
#include <chrono>

#pragma warning(push)
#pragma warning(disable:4819)
#include <spdlog/spdlog.h>
#pragma warning(pop)

#include "Lock.h"

#ifdef _DEBUG
#define LOG(...) spdlog::info(__VA_ARGS__)
#undef ASSERT
#define ASSERT(x, ...) {if(!(x)) { LOG(__VA_ARGS__); __debugbreak();}}
#else
#define LOG(...) 
#undef ASSERT
#define ASSERT(x, ...)
#endif

using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace std;
using namespace std::chrono;
