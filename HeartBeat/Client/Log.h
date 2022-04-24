#pragma once

#pragma warning(push)
#pragma warning(disable:4819)
#include <spdlog/spdlog.h>
#pragma warning(pop)

#ifdef _DEBUG
#define HB_LOG(...) spdlog::info(__VA_ARGS__)
#define HB_ASSERT(x, ...) {if((!x)) { spdlog::info(__VA_ARGS__); __debugbreak();}}
#else
#define HB_LOG(...)
#define HB_ASSERT(x, ...)
#endif // _DEBUG