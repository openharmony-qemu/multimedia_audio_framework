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

namespace OHOS {
namespace AudioStandard {
const std::string AudioManagerUtil::GetEffectSceneName(const StreamUsage &streamUsage)
{
    SupportedEffectConfig supportedEffectConfig;
    AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    std::string streamUsageString = "";
    if (STREAM_USAGE_MAP.find(streamUsage) != STREAM_USAGE_MAP.end()) {
        streamUsageString = STREAM_USAGE_MAP.find(streamUsage)->second;
    }
    if (supportedEffectConfig.postProcessNew.stream.empty()) {
        AUDIO_WARNING_LOG("empty scene type set!");
        return AUDIO_SUPPORTED_SCENE_TYPES.find(SCENE_OTHERS)->second;
    }
    if (streamUsageString == "") {
        AUDIO_WARNING_LOG("Find streamUsage string failed, not in the parser's string-enum map.");
        return AUDIO_SUPPORTED_SCENE_TYPES.find(SCENE_OTHERS)->second;
    }
    for (const SceneMappingItem &item: supportedEffectConfig.postProcessSceneMap) {
        if (item.name == streamUsageString) {
            return item.sceneType;
        }
    }
    return AUDIO_SUPPORTED_SCENE_TYPES.find(SCENE_OTHERS)->second;
}
} // namespace AudioStandard
} // namespace OHOS