#pragma once

#include <spdlog/spdlog.h>

#define HB_LOG(...) spdlog::info(__VA_ARGS__)
#define HB_ASSERT(x, ...) {if(!(x)) { spdlog::info(__VA_ARGS__); __debugbreak();}}
