/*
 * SPDX-FileCopyrightText: Copyright 2024 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_ENCODABLE_HELPER_H
#define FLUTTER_PLUGIN_ENCODABLE_HELPER_H

#include <flutter/encodable_value.h>

typedef flutter::EncodableValue EncodableValue;
typedef flutter::EncodableValue EncodableValue;
typedef flutter::EncodableMap EncodableMap;
typedef flutter::EncodableList EncodableList;

namespace Helper {

// ========== encodable_value ==========

template <typename T>
inline bool TypeIs(const EncodableValue & val) {
    return std::holds_alternative<T>(val);
}

template <typename T>
inline const T GetValue(const EncodableValue & val) {
    return std::get<T>(val);
}

inline EncodableMap GetMap(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    if (it != map.end() && TypeIs<EncodableMap>(it->second))
        return GetValue<EncodableMap>(it->second);
    return EncodableMap();
}

inline EncodableList GetList(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    if (it != map.end() && TypeIs<EncodableList>(it->second))
        return GetValue<EncodableList>(it->second);
    return EncodableList();
}

inline int GetInt(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    if (it != map.end() && TypeIs<int>(it->second))
        return GetValue<int>(it->second);
    return -1;
}

inline bool GetBool(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    if (it != map.end() && TypeIs<bool>(it->second))
        return GetValue<bool>(it->second);
    return false;
}

inline std::string GetString(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    if (it != map.end() && TypeIs<std::string>(it->second))
        return GetValue<std::string>(it->second);
    return std::string();
}

inline std::vector<int> GetVectorInt(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    std::vector<int> result{};
    if (it != map.end() && TypeIs<std::vector<EncodableValue>>(it->second))
        for (auto item : GetValue<std::vector<EncodableValue>>(it->second)) {
            result.push_back(GetValue<int>(item));
        }
    return result;
}

inline std::vector<double> GetVectorDouble(const EncodableMap& map, const std::string& key) {
    auto it = map.find(EncodableValue(key));
    std::vector<double> result{};
    if (it != map.end() && TypeIs<std::vector<EncodableValue>>(it->second))
        for (auto item : GetValue<std::vector<EncodableValue>>(it->second)) {
            result.push_back(GetValue<double>(item));
        }
    return result;
}

}  // namespace Helper

#endif /* FLUTTER_PLUGIN_ENCODABLE_HELPER_H */
