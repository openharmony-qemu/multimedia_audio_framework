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
    static const std::map<int32_t, std::string> typeToKey = {
        { KARAOKE_PARAM_VOLUME,         "Karaoke_volume" },
        { KARAOKE_PARAM_REVERB_MODE,    "Karaoke_reverb_mode" },
        { KARAOKE_PARAM_EQ_MODE,        "Karaoke_eq_mode" },
        { KARAOKE_PARAM_ENABLE,         "Karaoke_enable" },
        { KARAOKE_PARAM_CAPTURE_REVERB, "Karaoke_capture_reverb" },
    };
    static const std::map<int32_t, std::string> reverbToString = {
        { REVERB_PRESET_ORIGINAL, "disable" },
        { REVERB_PRESET_KTV,      "ktv" },
        { REVERB_PRESET_THEATER,  "theatre" },
        { REVERB_PRESET_CONCERT,  "concert" },
    };
    static const std::map<int32_t, std::string> eqToString = {
        { EQUALIZER_PRESET_FLAT,   "disable" },
        { EQUALIZER_PRESET_FULL,   "full" },
        { EQUALIZER_PRESET_BRIGHT, "bright" },
    };

    auto keyIt = typeToKey.find(paramType);
    if (keyIt == typeToKey.end()) {
        valid = false;
        return "";
    }
    std::string valueStr;
    switch (paramType) {
        case KARAOKE_PARAM_VOLUME:
            if (value < 0 || value > 100) {
                valid = false;
                return "";
            }
            valueStr = std::to_string(value);
            break;
        case KARAOKE_PARAM_REVERB_MODE: {
            auto it = reverbToString.find(value);
            if (it == reverbToString.end()) {
                valid = false;
                return "";
            }
            valueStr = it->second;
            break;
        }
        case KARAOKE_PARAM_EQ_MODE: {
            auto it = eqToString.find(value);
            if (it == eqToString.end()) {
                valid = false;
                return "";
            }
            valueStr = it->second;
            break;
        }
        case KARAOKE_PARAM_ENABLE:
            valueStr = (value != 0) ? "enable" : "disable";
            break;
        case KARAOKE_PARAM_CAPTURE_REVERB:
            valueStr = (value != 0) ? "true" : "false";
            break;
        default:
            valid = false;
            return "";
    }
    return keyIt->second + "=" + valueStr;
}

} // namespace AudioStandard
} // namespace OHOS

#endif // KARAOKE_PARAM_HELPER_H
