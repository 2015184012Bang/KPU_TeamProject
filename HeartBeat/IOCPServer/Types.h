#pragma once

#include <mutex>

using LockGuard = std::lock_guard<std::mutex>;
using Mutex = std::mutex;