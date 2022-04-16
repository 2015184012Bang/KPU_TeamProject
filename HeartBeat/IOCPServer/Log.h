#pragma once

#pragma warning(push)
#pragma warning(disable:4819)
#include <spdlog/spdlog.h>
#pragma warning(pop)

#define LOG(...) spdlog::info(__VA_ARGS__)