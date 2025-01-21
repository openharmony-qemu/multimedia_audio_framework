/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioToneManager"
#endif

#include "audio_tone_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"

#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "audio_tone_parser.h"
#include "media_monitor_manager.h"

#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {

#ifdef FEATURE_DTMF_TONE
bool AudioToneManager::LoadToneDtmfConfig()
{
    AUDIO_INFO_LOG("Enter");
    std::unique_ptr<AudioToneParser> audioToneParser = std::make_unique<AudioToneParser>();
    if (audioToneParser == nullptr) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Tone Load Configuration failed");
    }
    CHECK_AND_RETURN_RET_LOG(audioToneParser != nullptr, false, "Failed to create AudioToneParser");
    if (audioToneParser->LoadNewConfig(AudioToneParser::AUDIO_TONE_CONFIG_FILE, toneDescriptorMap_,
        customToneDescriptorMap_)) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::EventType::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_TONE_DTMF_CONFIG);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Tone Load Configuration failed");
        AUDIO_ERR_LOG("Audio Tone Load Configuration failed");
        return false;
    }
    AUDIO_INFO_LOG("Done");
    return true;
}

std::vector<int32_t> AudioToneManager::GetSupportedTones(const std::string &countryCode)
{
    AUDIO_DEBUG_LOG("countryCode: %{public}s", countryCode.c_str());
    std::set<int32_t> supportedToneList = {};
    auto customToneDescriptorItem = customToneDescriptorMap_.find(countryCode);
    if (customToneDescriptorItem != customToneDescriptorMap_.end()) {
        for (auto &[number, toneInfo] : customToneDescriptorItem->second) {
            supportedToneList.insert(number);
        }
    }

    for (auto &[number, toneInfo] : toneDescriptorMap_) {
        supportedToneList.insert(number);
    }

    return std::vector<int32_t>(supportedToneList.begin(), supportedToneList.end());
}

std::shared_ptr<ToneInfo> AudioToneManager::GetToneConfig(int32_t ltonetype, const std::string &countryCode)
{
    AUDIO_DEBUG_LOG("ltonetype: %{public}d, countryCode: %{public}s", ltonetype, countryCode.c_str());
    auto customToneDescriptorItem = customToneDescriptorMap_.find(countryCode);
    if (customToneDescriptorItem != customToneDescriptorMap_.end()) {
        auto toneInfo = customToneDescriptorItem->second.find(ltonetype);
        if (toneInfo != customToneDescriptorItem->second.end()) {
            AUDIO_DEBUG_LOG("Get custom ToneConfig %{public}d", ltonetype);
            return toneInfo->second;
        }
    }

    if (toneDescriptorMap_.find(ltonetype) != toneDescriptorMap_.end()) {
        AUDIO_DEBUG_LOG("Get default ToneConfig %{public}d", ltonetype);
        return toneDescriptorMap_[ltonetype];
    }
    AUDIO_DEBUG_LOG("Get ToneConfig %{public}d fail", ltonetype);
    return nullptr;
}
#endif

}
}