/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef KARAOKE_PARAM_HELPER_H
#define KARAOKE_PARAM_HELPER_H

#include <map>
#include <string>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

// Single source of truth: KaraokeParamType -> legacy "Karaoke_<key>".
inline const std::map<int32_t, std::string> &GetKaraokeTypeToKeyMap()
{
    static const std::map<int32_t, std::string> typeToKey = {
        { KARAOKE_PARAM_VOLUME,         "Karaoke_volume" },
        { KARAOKE_PARAM_REVERB_MODE,    "Karaoke_reverb_mode" },
        { KARAOKE_PARAM_EQ_MODE,        "Karaoke_eq_mode" },
        { KARAOKE_PARAM_ENABLE,         "Karaoke_enable" },
        { KARAOKE_PARAM_CAPTURE_REVERB, "Karaoke_capture_reverb" },
    };
    return typeToKey;
}

// Reverb preset ordinal -> legacy HDI reverb string.
inline const std::map<int32_t, std::string> &GetKaraokeReverbMap()
{
    static const std::map<int32_t, std::string> reverbToString = {
        { REVERB_PRESET_ORIGINAL, "disable" },
        { REVERB_PRESET_KTV,      "ktv" },
        { REVERB_PRESET_THEATER,  "theatre" },
        { REVERB_PRESET_CONCERT,  "concert" },
    };
    return reverbToString;
}

// Equalizer preset ordinal -> legacy HDI eq string.
inline const std::map<int32_t, std::string> &GetKaraokeEqMap()
{
    static const std::map<int32_t, std::string> eqToString = {
        { EQUALIZER_PRESET_FLAT,   "disable" },
        { EQUALIZER_PRESET_FULL,   "full" },
        { EQUALIZER_PRESET_BRIGHT, "bright" },
    };
    return eqToString;
}

// Generic table lookup; sets valid=true on hit, valid=false and returns "" when absent.
// Returns a const reference into the (static) table to avoid a string copy on hit.
inline const std::string &KaraokeLookup(const std::map<int32_t, std::string> &table,
    int32_t key, bool &valid)
{
    auto it = table.find(key);
    if (it == table.end()) {
        valid = false;
        static const std::string empty;
        return empty;
    }
    valid = true;
    return it->second;
}

// Volume is the only plain integer value (0-100); reject out-of-range.
inline std::string KaraokeVolumeToString(int32_t value, bool &valid)
{
    if (value < 0 || value > 100) {
        valid = false;
        return "";
    }
    return std::to_string(value);
}

// Boolean-encoded values (ENABLE / CAPTURE_REVERB) accept only 0 or 1; any other
// value is rejected so the string dispatched to HDI stays consistent with the
// nearlink transition gate in AudioPolicyServer::SetKaraokeParameters.
inline std::string KaraokeBoolToString(int32_t value, bool &valid, const char *onTrue, const char *onFalse)
{
    if (value != 0 && value != 1) {
        valid = false;
        return "";
    }
    return value == 1 ? onTrue : onFalse;
}

// Encode a (paramType, value) pair into its value string. Sets valid=false on
// any invalid input (unknown type / out-of-range volume / unknown preset / non-boolean).
inline std::string KaraokeValueToString(int32_t paramType, int32_t value, bool &valid)
{
    switch (paramType) {
        case KARAOKE_PARAM_VOLUME: return KaraokeVolumeToString(value, valid);
        case KARAOKE_PARAM_REVERB_MODE: return KaraokeLookup(GetKaraokeReverbMap(), value, valid);
        case KARAOKE_PARAM_EQ_MODE: return KaraokeLookup(GetKaraokeEqMap(), value, valid);
        case KARAOKE_PARAM_ENABLE: return KaraokeBoolToString(value, valid, "enable", "disable");
        case KARAOKE_PARAM_CAPTURE_REVERB: return KaraokeBoolToString(value, valid, "true", "false");
        default: valid = false; return "";
    }
}

/**
 * @brief Build the legacy "key=value" audio parameter string for a Karaoke setting.
 *
 * Single source of truth for the Karaoke key/value mapping. The output is byte
 * identical to the strings the client used to assemble before the enum refactor.
 *
 * @param paramType One of KaraokeParamType.
 * @param value     Type-dependent integer (see KaraokeParamType docs).
 * @param valid     Set to false for unknown paramType, out-of-range volume, or
 *                  unknown preset ordinal; in that case returns "" and the
 *                  caller must NOT dispatch to HDI.
 * @return          "Karaoke_<key>=<value>" string, or "" when invalid.
 */
inline std::string BuildKaraokeParamString(int32_t paramType, int32_t value, bool &valid)
{
    valid = true;
    const std::string &key = KaraokeLookup(GetKaraokeTypeToKeyMap(), paramType, valid);
    if (!valid) {
        return "";
    }
    std::string valueStr = KaraokeValueToString(paramType, value, valid);
    return valid ? (key + "=" + valueStr) : "";
}

} // namespace AudioStandard
} // namespace OHOS

#endif // KARAOKE_PARAM_HELPER_H
