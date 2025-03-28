// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_OTPCH_H
#define FS_OTPCH_H

// Ensure Winsock2.h is included before any other headers that might include windows.h
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX      // Prevent windows.h from defining min/max macros
#endif

// Include Winsock2.h directly and properly
#include <Winsock2.h>
#include <Windows.h>
// Undefine any problematic Windows macros
#undef min
#undef max
#endif

// Define our own min/max functions to avoid any possible conflicts with macros
#if !defined(MY_MIN)
#define MY_MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#if !defined(MY_MAX)
#define MY_MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

// Definitions should be global.
#include "definitions.h"

// System headers required in headers should be included here.
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lockfree/stack.hpp>
#include <boost/variant.hpp>
#include <cassert>
#include <concepts>
#include <condition_variable>
#include <cryptopp/rsa.h>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <forward_list>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#ifdef _WIN32
#include "lua.hpp"
#elif defined(__linux__)
#endif

// Include pugixml.hpp before pugicast.h
#include <pugixml.hpp>
#include "pugicast.h"

#include <map>
#include <memory>
#include <mutex>
#include <mysql/mysql.h>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>

#endif // FS_OTPCH_H
