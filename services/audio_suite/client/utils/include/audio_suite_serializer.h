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

#ifndef AUDIO_SUITE_SERIALIZER_H
#define AUDIO_SUITE_SERIALIZER_H

#include <string>
#include <sstream>
#include <vector>
#include "audio_suite_base.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr size_t POSITION_PARAMS_NUM = 3;
constexpr size_t ROTATION_PARAMS_NUM = 5;
constexpr size_t EXTENSION_PARAMS_NUM = 2;
constexpr size_t TEMPO_PITCH_PARAMS_NUM = 2;
constexpr size_t PURE_VOICE_CHANGE_MIN_PARAMS_NUM = 2;
constexpr size_t PURE_VOICE_CHANGE_PITCH_PARAM_INDEX = 2;

constexpr size_t PARAM_INDEX_0 = 0;
constexpr size_t PARAM_INDEX_1 = 1;
constexpr size_t PARAM_INDEX_2 = 2;
constexpr size_t PARAM_INDEX_3 = 3;
constexpr size_t PARAM_INDEX_4 = 4;

template<typename T>
struct Serializer {
    static std::string Serialize(const T& param);
    static bool Deserialize(const std::string& str, T& param);
};

std::vector<std::string> SplitString(const std::string& str, char delimiter);

bool StringConverter(const std::string& str, int32_t& value);
bool StringConverterFloat(const std::string& str, float& value);

template<>
struct Serializer<AudioSpaceRenderPositionParams> {
    static std::string Serialize(const AudioSpaceRenderPositionParams& param);
    static bool Deserialize(const std::string& str, AudioSpaceRenderPositionParams& param);
};

template<>
struct Serializer<AudioSpaceRenderRotationParams> {
    static std::string Serialize(const AudioSpaceRenderRotationParams& param);
    static bool Deserialize(const std::string& str, AudioSpaceRenderRotationParams& param);
};

template<>
struct Serializer<AudioSpaceRenderExtensionParams> {
    static std::string Serialize(const AudioSpaceRenderExtensionParams& param);
    static bool Deserialize(const std::string& str, AudioSpaceRenderExtensionParams& param);
};

struct TempoPitchParams {
    float speed;
    float pitch;
};

template<>
struct Serializer<TempoPitchParams> {
    static std::string Serialize(const TempoPitchParams& param);
    static bool Deserialize(const std::string& str, TempoPitchParams& param);
};

template<>
struct Serializer<AudioPureVoiceChangeOption> {
    static std::string Serialize(const AudioPureVoiceChangeOption& param);
    static bool Deserialize(const std::string& str, AudioPureVoiceChangeOption& param);
};

template<>
struct Serializer<AudioGeneralVoiceChangeType> {
    static std::string Serialize(const AudioGeneralVoiceChangeType& param);
    static bool Deserialize(const std::string& str, AudioGeneralVoiceChangeType& param);
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // AUDIO_SUITE_SERIALIZER_H