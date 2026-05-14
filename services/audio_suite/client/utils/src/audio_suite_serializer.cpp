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

#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteSerializer"
#endif

#include "audio_suite_serializer.h"
#include <cerrno>
#include <climits>
#include <sstream>
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str);
    
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool StringConverter(const std::string& str, int32_t& value)
{
    if (str.empty()) {
        AUDIO_ERR_LOG("StringConverter: input string is empty");
        return false;
    }
    
    char *end = nullptr;
    errno = 0;
    long result = std::strtol(str.c_str(), &end, 10);
    if (end == str.c_str() || *end != '\0' || errno != 0) {
        AUDIO_ERR_LOG("StringConverter: invalid conversion for '%s'", str.c_str());
        return false;
    }
    if (result < INT32_MIN || result > INT32_MAX) {
        AUDIO_ERR_LOG("StringConverter: value out of range for '%s'", str.c_str());
        return false;
    }
    value = static_cast<int32_t>(result);
    return true;
}

bool StringConverterFloat(const std::string& str, float& value)
{
    if (str.empty()) {
        AUDIO_ERR_LOG("StringConverterFloat: input string is empty");
        return false;
    }
    
    char *end = nullptr;
    errno = 0;
    value = std::strtof(str.c_str(), &end);
    if (end == str.c_str() || *end != '\0' || errno != 0) {
        AUDIO_ERR_LOG("StringConverterFloat: invalid conversion for '%s'", str.c_str());
        return false;
    }
    return true;
}

std::string Serializer<AudioSpaceRenderPositionParams>::Serialize(const AudioSpaceRenderPositionParams& param)
{
    return std::to_string(param.x) + "," + std::to_string(param.y) + "," + std::to_string(param.z);
}

bool Serializer<AudioSpaceRenderPositionParams>::Deserialize(const std::string& str,
    AudioSpaceRenderPositionParams& param)
{
    std::vector<std::string> tokens = SplitString(str, ',');
    if (tokens.size() != POSITION_PARAMS_NUM) {
        AUDIO_ERR_LOG("Invalid Position parameter format, expected %{public}zu fields, got %{public}zu",
            POSITION_PARAMS_NUM, tokens.size());
        return false;
    }
    
    float x;
    float y;
    float z;
    if (!StringConverterFloat(tokens[PARAM_INDEX_0], x) ||
        !StringConverterFloat(tokens[PARAM_INDEX_1], y) ||
        !StringConverterFloat(tokens[PARAM_INDEX_2], z)) {
        AUDIO_ERR_LOG("Failed to convert position parameters");
        return false;
    }
    
    param.x = x;
    param.y = y;
    param.z = z;
    return true;
}

std::string Serializer<AudioSpaceRenderRotationParams>::Serialize(const AudioSpaceRenderRotationParams& param)
{
    return std::to_string(param.x) + "," + std::to_string(param.y) + "," +
           std::to_string(param.z) + "," + std::to_string(param.surroundTime) + "," +
           std::to_string(static_cast<int32_t>(param.surroundDirection));
}

bool Serializer<AudioSpaceRenderRotationParams>::Deserialize(const std::string& str,
    AudioSpaceRenderRotationParams& param)
{
    std::vector<std::string> tokens = SplitString(str, ',');
    if (tokens.size() != ROTATION_PARAMS_NUM) {
        AUDIO_ERR_LOG("Invalid Rotation parameter format, expected %{public}zu fields, got %{public}zu",
            ROTATION_PARAMS_NUM, tokens.size());
        return false;
    }
    
    float x;
    float y;
    float z;
    int32_t surroundTime;
    int32_t directionInt;
    if (!StringConverterFloat(tokens[PARAM_INDEX_0], x) ||
        !StringConverterFloat(tokens[PARAM_INDEX_1], y) ||
        !StringConverterFloat(tokens[PARAM_INDEX_2], z) ||
        !StringConverter(tokens[PARAM_INDEX_3], surroundTime) ||
        !StringConverter(tokens[PARAM_INDEX_4], directionInt)) {
        AUDIO_ERR_LOG("Failed to convert rotation parameters");
        return false;
    }
    
    param.x = x;
    param.y = y;
    param.z = z;
    param.surroundTime = surroundTime;
    param.surroundDirection = static_cast<AudioSurroundDirection>(directionInt);
    return true;
}

std::string Serializer<AudioSpaceRenderExtensionParams>::Serialize(const AudioSpaceRenderExtensionParams& param)
{
    return std::to_string(param.extRadius) + "," + std::to_string(param.extAngle);
}

bool Serializer<AudioSpaceRenderExtensionParams>::Deserialize(const std::string& str,
    AudioSpaceRenderExtensionParams& param)
{
    std::vector<std::string> tokens = SplitString(str, ',');
    if (tokens.size() != EXTENSION_PARAMS_NUM) {
        AUDIO_ERR_LOG("Invalid Extension parameter format, expected %{public}zu fields, got %{public}zu",
            EXTENSION_PARAMS_NUM, tokens.size());
        return false;
    }
    
    float extRadius;
    int32_t extAngle;
    if (!StringConverterFloat(tokens[PARAM_INDEX_0], extRadius) ||
        !StringConverter(tokens[PARAM_INDEX_1], extAngle)) {
        AUDIO_ERR_LOG("Failed to convert extension parameters");
        return false;
    }
    
    param.extRadius = extRadius;
    param.extAngle = extAngle;
    return true;
}

std::string Serializer<TempoPitchParams>::Serialize(const TempoPitchParams& param)
{
    return std::to_string(param.speed) + "," + std::to_string(param.pitch);
}

bool Serializer<TempoPitchParams>::Deserialize(const std::string& str, TempoPitchParams& param)
{
    std::vector<std::string> tokens = SplitString(str, ',');
    if (tokens.size() != TEMPO_PITCH_PARAMS_NUM) {
        AUDIO_ERR_LOG("Invalid TempoPitch parameter format, expected %{public}zu fields, got %{public}zu",
            TEMPO_PITCH_PARAMS_NUM, tokens.size());
        return false;
    }
    
    float speed;
    float pitch;
    if (!StringConverterFloat(tokens[PARAM_INDEX_0], speed) ||
        !StringConverterFloat(tokens[PARAM_INDEX_1], pitch)) {
        AUDIO_ERR_LOG("Failed to convert tempo/pitch parameters");
        return false;
    }
    
    param.speed = speed;
    param.pitch = pitch;
    return true;
}

std::string Serializer<AudioPureVoiceChangeOption>::Serialize(const AudioPureVoiceChangeOption& param)
{
    return std::to_string(static_cast<int32_t>(param.optionGender)) + "," +
           std::to_string(static_cast<int32_t>(param.optionType)) + "," +
           std::to_string(param.pitch);
}

bool Serializer<AudioPureVoiceChangeOption>::Deserialize(const std::string& str,
    AudioPureVoiceChangeOption& param)
{
    std::vector<std::string> tokens = SplitString(str, ',');
    if (tokens.size() < PURE_VOICE_CHANGE_MIN_PARAMS_NUM) {
        AUDIO_ERR_LOG("Invalid PureVoiceChange parameter format, expected at least %{public}zu fields, got %{public}zu",
            PURE_VOICE_CHANGE_MIN_PARAMS_NUM, tokens.size());
        return false;
    }
    
    int32_t genderInt;
    int32_t typeInt;
    if (!StringConverter(tokens[PARAM_INDEX_0], genderInt) ||
        !StringConverter(tokens[PARAM_INDEX_1], typeInt)) {
        AUDIO_ERR_LOG("Failed to convert gender or type parameters");
        return false;
    }
    
    float pitch = 0.0f;
    if (tokens.size() > PURE_VOICE_CHANGE_PITCH_PARAM_INDEX) {
        if (!StringConverterFloat(tokens[PURE_VOICE_CHANGE_PITCH_PARAM_INDEX], pitch)) {
            AUDIO_ERR_LOG("Failed to convert pitch parameter");
            return false;
        }
    }
    
    param.optionGender = static_cast<AudioPureVoiceChangeGenderOption>(genderInt);
    param.optionType = static_cast<AudioPureVoiceChangeType>(typeInt);
    param.pitch = pitch;
    return true;
}

std::string Serializer<AudioGeneralVoiceChangeType>::Serialize(const AudioGeneralVoiceChangeType& param)
{
    return std::to_string(static_cast<int32_t>(param));
}

bool Serializer<AudioGeneralVoiceChangeType>::Deserialize(const std::string& str,
    AudioGeneralVoiceChangeType& param)
{
    int32_t value;
    if (!StringConverter(str, value)) {
        AUDIO_ERR_LOG("Failed to convert GeneralVoiceChange type parameter");
        return false;
    }
    param = static_cast<AudioGeneralVoiceChangeType>(value);
    return true;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS