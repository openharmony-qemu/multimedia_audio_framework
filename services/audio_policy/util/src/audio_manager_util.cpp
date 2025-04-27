/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioManagerUtil"
#endif

#include "audio_manager_util.h"

#include "audio_common_log.h"
#include "audio_effect.h"
#include "audio_policy_manager.h"
#include "audio_effect_map.h"

namespace OHOS {
namespace AudioStandard {
const std::unordered_map<StreamUsage, std::string> STREAM_USAGE_MAP = {
    {STREAM_USAGE_UNKNOWN, "STREAM_USAGE_UNKNOWN"},
    // STREAM_USAGE_MUSIC(1), STREAM_USAGE_MEDIA(1), both mapped to STREAM_USAGE_MUSIC
    {STREAM_USAGE_MUSIC, "STREAM_USAGE_MUSIC"},
    {STREAM_USAGE_VOICE_COMMUNICATION, "STREAM_USAGE_VOICE_COMMUNICATION"},
    {STREAM_USAGE_VOICE_ASSISTANT, "STREAM_USAGE_VOICE_ASSISTANT"},
    {STREAM_USAGE_ALARM, "STREAM_USAGE_ALARM"},
    {STREAM_USAGE_VOICE_MESSAGE, "STREAM_USAGE_VOICE_MESSAGE"},
    // STREAM_USAGE_RINGTONE(6), STREAM_USAGE_NOTIFICATION_RINGRONE(6) both mapped to STREAM_USAGE_RINGTONE
    {STREAM_USAGE_RINGTONE, "STREAM_USAGE_RINGTONE"},
    {STREAM_USAGE_NOTIFICATION, "STREAM_USAGE_NOTIFICATION"},
    {STREAM_USAGE_ACCESSIBILITY, "STREAM_USAGE_ACCESSIBILITY"},
    {STREAM_USAGE_SYSTEM, "STREAM_USAGE_SYSTEM"},
    {STREAM_USAGE_MOVIE, "STREAM_USAGE_MOVIE"},
    {STREAM_USAGE_GAME, "STREAM_USAGE_GAME"},
    {STREAM_USAGE_AUDIOBOOK, "STREAM_USAGE_AUDIOBOOK"},
    {STREAM_USAGE_NAVIGATION, "STREAM_USAGE_NAVIGATION"},
    {STREAM_USAGE_DTMF, "STREAM_USAGE_DTMF"},
    {STREAM_USAGE_ENFORCED_TONE, "STREAM_USAGE_ENFORCED_TONE"},
    {STREAM_USAGE_ULTRASONIC, "STREAM_USAGE_ULTRASONIC"},
    {STREAM_USAGE_VIDEO_COMMUNICATION, "STREAM_USAGE_VIDEO_COMMUNICATION"},
    {STREAM_USAGE_VOICE_CALL_ASSISTANT, "STREAM_USAGE_VOICE_CALL_ASSISTANT"},
    {STREAM_USAGE_RANGING, "STREAM_USAGE_RANGING"},
    {STREAM_USAGE_VOICE_MODEM_COMMUNICATION, "STREAM_USAGE_VOICE_MODEM_COMMUNICATION"},
    {STREAM_USAGE_VOICE_RINGTONE, "STREAM_USAGE_VOICE_RINGTONE"},
};
const std::string AudioManagerUtil::GetEffectSceneName(const StreamUsage &streamUsage)
{
    SupportedEffectConfig supportedEffectConfig;
    AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    std::string streamUsageString = "";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (STREAM_USAGE_MAP.find(streamUsage) != STREAM_USAGE_MAP.end()) {
        streamUsageString = STREAM_USAGE_MAP.find(streamUsage)->second;
    }
    if (supportedEffectConfig.postProcessNew.stream.empty()) {
        AUDIO_WARNING_LOG("empty scene type set!");
        return audioSupportedSceneTypes.find(SCENE_OTHERS)->second;
    }
    if (streamUsageString == "") {
        AUDIO_WARNING_LOG("Find streamUsage string failed, not in the parser's string-enum map.");
        return audioSupportedSceneTypes.find(SCENE_OTHERS)->second;
    }
    for (const SceneMappingItem &item: supportedEffectConfig.postProcessSceneMap) {
        if (item.name == streamUsageString) {
            return item.sceneType;
        }
    }
    return audioSupportedSceneTypes.find(SCENE_OTHERS)->second;
}
} // namespace AudioStandard
} // namespace OHOS