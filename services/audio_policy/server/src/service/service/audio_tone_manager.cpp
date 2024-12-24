/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "audio_utils.h"
#include "audio_log.h"
#include "audio_utils.h"
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
    std::string AUDIO_TONE_CONFIG_FILE = "/system/etc/audio/audio_tone_dtmf_config.xml";

    if (audioToneParser->LoadConfig(toneDescriptorMap_)) {
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

std::vector<int32_t> AudioToneManager::GetSupportedTones()
{
    std::vector<int> supportedToneList = {};
    for (auto i = toneDescriptorMap_.begin(); i != toneDescriptorMap_.end(); i++) {
        supportedToneList.push_back(i->first);
    }
    return supportedToneList;
}

std::shared_ptr<ToneInfo> AudioToneManager::GetToneConfig(int32_t ltonetype)
{
    if (toneDescriptorMap_.find(ltonetype) != toneDescriptorMap_.end()) {
        AUDIO_DEBUG_LOG("AudioToneManager GetToneConfig %{public}d", ltonetype);
        return toneDescriptorMap_[ltonetype];
    }
    AUDIO_DEBUG_LOG("AudioToneManager GetToneConfig %{public}d", ltonetype);
    return nullptr;
}
#endif

}
}