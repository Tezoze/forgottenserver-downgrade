// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

// #pragma message("Including pugicast.h at " __FILE__ ":" __LINE__)
#pragma once
#ifndef FS_PUGICAST_H
#define FS_PUGICAST_H

// Only include pugixml.hpp if it hasn't been included yet
#ifndef PUGIXML_VERSION
#include <pugixml.hpp>
#endif
#include <cstdint>
#include <string>
#include <type_traits>

// Create a namespace that provides XML parsing functions
namespace fs {
    
#ifndef FS_PUGICAST_FUNCTIONS_DEFINED
#define FS_PUGICAST_FUNCTIONS_DEFINED

// Helper functions - internal use only
namespace detail {
    inline int pugicast_convert(const pugi::xml_attribute& attr, int) {
        return attr.as_int();
    }

    inline unsigned int pugicast_convert(const pugi::xml_attribute& attr, unsigned int) {
        return attr.as_uint();
    }

    inline int16_t pugicast_convert(const pugi::xml_attribute& attr, int16_t) {
        return static_cast<int16_t>(attr.as_int());
    }

    inline uint16_t pugicast_convert(const pugi::xml_attribute& attr, uint16_t) {
        return static_cast<uint16_t>(attr.as_uint());
    }

    inline int32_t pugicast_convert(const pugi::xml_attribute& attr, int32_t) {
        return attr.as_int();
    }

    inline uint32_t pugicast_convert(const pugi::xml_attribute& attr, uint32_t) {
        return attr.as_uint();
    }

    inline int64_t pugicast_convert(const pugi::xml_attribute& attr, int64_t) {
        return attr.as_llong();
    }

    inline uint64_t pugicast_convert(const pugi::xml_attribute& attr, uint64_t) {
        return attr.as_ullong();
    }

    inline bool pugicast_convert(const pugi::xml_attribute& attr, bool) {
        return attr.as_bool();
    }

    inline float pugicast_convert(const pugi::xml_attribute& attr, float) {
        return attr.as_float();
    }

    inline double pugicast_convert(const pugi::xml_attribute& attr, double) {
        return attr.as_double();
    }

    inline const char* pugicast_convert(const pugi::xml_attribute& attr, const char*) {
        return attr.value();
    }

    inline std::string pugicast_convert(const pugi::xml_attribute& attr, std::string) {
        return std::string(attr.value());
    }
}

// Public API
template <typename T>
inline T xml_parse(const pugi::xml_attribute& attr) {
    return detail::pugicast_convert(attr, T());
}

#endif // FS_PUGICAST_FUNCTIONS_DEFINED

}

#endif // FS_PUGICAST_H
