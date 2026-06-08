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
#define LOG_TAG "AudioCollaborationManager"
#endif

#include "audio_collaboration_manager.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_effect_log.h"
#include "audio_utils.h"
#include "audio_device_type.h"

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t DEFAULT_LATENCY = 205;
static constexpr const char* AUDIO_COLLABORATION_CONFIG_FILE =
    "sys_prod/etc/audio/audio_collaborative_playback_config.xml";
static std::map<std::string, AudioTwsMode> twsModeMap = {
    {"default", TWS_MODE_DEFAULT},
    {"listen", TWS_MODE_LISTEN},
    {"hitws", TWS_MODE_HITWS},
};

static std::map<std::string, StreamUsage> streamUsageMap = {
    {"STREAM_USAGE_UNKNOWN", STREAM_USAGE_UNKNOWN},
    {"STREAM_USAGE_MEDIA", STREAM_USAGE_MEDIA},
    {"STREAM_USAGE_MUSIC", STREAM_USAGE_MUSIC},
    {"STREAM_USAGE_VOICE_COMMUNICATION", STREAM_USAGE_VOICE_COMMUNICATION},
    {"STREAM_USAGE_VOICE_ASSISTANT", STREAM_USAGE_VOICE_ASSISTANT},
    {"STREAM_USAGE_VOICE_CALL_ASSISTANT", STREAM_USAGE_VOICE_CALL_ASSISTANT},
    {"STREAM_USAGE_ALARM", STREAM_USAGE_ALARM},
    {"STREAM_USAGE_VOICE_MESSAGE", STREAM_USAGE_VOICE_MESSAGE},
    {"STREAM_USAGE_NOTIFICATION_RINGTONE", STREAM_USAGE_NOTIFICATION_RINGTONE},
    {"STREAM_USAGE_RINGTONE", STREAM_USAGE_RINGTONE},
    {"STREAM_USAGE_NOTIFICATION", STREAM_USAGE_NOTIFICATION},
    {"STREAM_USAGE_ACCESSIBILITY", STREAM_USAGE_ACCESSIBILITY},
    {"STREAM_USAGE_SYSTEM", STREAM_USAGE_SYSTEM},
    {"STREAM_USAGE_MOVIE", STREAM_USAGE_MOVIE},
    {"STREAM_USAGE_GAME", STREAM_USAGE_GAME},
    {"STREAM_USAGE_AUDIOBOOK", STREAM_USAGE_AUDIOBOOK},
    {"STREAM_USAGE_NAVIGATION", STREAM_USAGE_NAVIGATION},
    {"STREAM_USAGE_DTMF", STREAM_USAGE_DTMF},
    {"STREAM_USAGE_ENFORCED_TONE", STREAM_USAGE_ENFORCED_TONE},
    {"STREAM_USAGE_ULTRASONIC", STREAM_USAGE_ULTRASONIC},
    {"STREAM_USAGE_VIDEO_COMMUNICATION", STREAM_USAGE_VIDEO_COMMUNICATION},
    {"STREAM_USAGE_RANGING", STREAM_USAGE_RANGING},
    {"STREAM_USAGE_VOICE_MODEM_COMMUNICATION", STREAM_USAGE_VOICE_MODEM_COMMUNICATION},
    {"STREAM_USAGE_VOICE_RINGTONE", STREAM_USAGE_VOICE_RINGTONE},
};

enum XML_ERROR {
    XML_PARSE_ERROR = 1 << 5,
    XML_PARSE_NOWARNING = 1 << 6,
};

AudioCollaborationManager::AudioCollaborationManager()
{
    latencyMs_ = DEFAULT_LATENCY;
}

AudioCollaborationManager::~AudioCollaborationManager()
{
    AUDIO_INFO_LOG("~AudioCollaborationManager()");
}

AudioCollaborationManager *AudioCollaborationManager::GetInstance()
{
    static AudioCollaborationManager audioCollaborationManager;
    return &audioCollaborationManager;
}

int32_t AudioCollaborationManager::UpdateCollaborativeProductId(const std::string &productId)
{
    std::lock_guard<std::mutex> lock(collaborationMutex_);
    auto pos = productId.find('_');
    std::string temProductId = (pos == std::string::npos) ? productId : productId.substr(0, pos);
    if (productId_ == temProductId) {
        return SUCCESS;
    }

    int32_t earphoneProduct = 0;
    if (collaborativeEarphoneProductConfig_.find(temProductId) != collaborativeEarphoneProductConfig_.end()) {
        earphoneProduct = collaborativeEarphoneProductConfig_[temProductId];
        productId_ = temProductId;
    } else {
        AUDIO_INFO_LOG("productId %{public}s no found in collaborativeEarphoneProductConfig_", temProductId.c_str());
        return ERROR;
    }

    updateLatencyInner();
    AudioEffectChainManager::GetInstance()->UpdateEarphoneProduct(earphoneProduct);
    AUDIO_INFO_LOG("productId: %{public}s, earphoneProduct: %{public}d, latencyMs: %{public}d",
        productId_.c_str(), earphoneProduct, latencyMs_);
    return SUCCESS;
}

void AudioCollaborationManager::LoadCollaborationConfig()
{
    std::lock_guard<std::mutex> lock(collaborationMutex_);
    AUDIO_INFO_LOG("begin loadCollaborationConfig");
    collaborativeLatencyConfig_.clear();
    std::shared_ptr<AudioXmlNode> node = AudioXmlNode::Create();
    node->Config(AUDIO_COLLABORATION_CONFIG_FILE, nullptr, XML_PARSE_ERROR | XML_PARSE_NOWARNING);
    LoadCollaborationConfigInner(node);

    for (auto iter1 : collaborativeLatencyConfig_) {
        for (auto iter2 : iter1.second) {
            AUDIO_INFO_LOG("productId: %{public}s, twsMode: %{public}d, latencyMs: %{public}d",
                iter1.first.c_str(), iter2.first, iter2.second);
        }
    }

    for (auto iter1 : collaborativeEarphoneNameConfig_) {
        AUDIO_INFO_LOG("productId: %{public}s, name: %{public}s",
            iter1.first.c_str(), iter1.second.c_str());
    }

    for (auto iter1 : collaborativeEarphoneProductConfig_) {
        AUDIO_INFO_LOG("productId: %{public}s, type: %{public}d",
            iter1.first.c_str(), iter1.second);
    }
}

void AudioCollaborationManager::LoadCollaborationConfigInner(const std::shared_ptr<AudioXmlNode> node)
{
    if (node == nullptr) {
        return;
    }
    if (!node->IsNodeValid()) {
        AUDIO_ERR_LOG("could not parse file %{public}s", AUDIO_COLLABORATION_CONFIG_FILE);
        return;
    }

    node->MoveToChildren();
    while (node->IsNodeValid()) {
        if (!node->IsElementNode()) {
            node->MoveToNext();
            continue;
        }
        if (node->CompareName("collabSupportedStreamUsage")) {
            std::shared_ptr<AudioXmlNode> collAbsNode = node->GetCopyNode();
            LoadCollAbsSupportedStreamUsageInner(collAbsNode);
            node->MoveToNext();
            continue;
        }
        std::string productId;
        std::string earphoneType;
        std::string earphoneName;
        if (!node->CompareName("product") ||
            node->GetProp("id", productId) != SUCCESS || node->GetProp("type", earphoneType) != SUCCESS ||
            node->GetProp("name", earphoneName) != SUCCESS) {
            AUDIO_ERR_LOG("product node without id or product or name");
            node->MoveToNext();
            continue;
        }

        try {
            collaborativeEarphoneProductConfig_[productId] = std::stoi(earphoneType);
        } catch (const std::invalid_argument& e) {
            AUDIO_ERR_LOG("Invalid earphoneType:%{public}s", earphoneType.c_str());
            node->MoveToNext();
            continue;
        } catch (const std::out_of_range& e) {
            AUDIO_ERR_LOG("Out of range earphoneType:%{public}s", earphoneType.c_str());
            node->MoveToNext();
            continue;
        }

        collaborativeEarphoneNameConfig_[productId] = earphoneName;

        std::shared_ptr<AudioXmlNode> secondNode = node->GetCopyNode();
        secondNode->MoveToChildren();
        LoadCollaborationLatencyInner(secondNode, productId);
        node->MoveToNext();
    }
}

void AudioCollaborationManager::LoadCollaborationLatencyInner(const std::shared_ptr<AudioXmlNode> secondNode,
                                                              const std::string productId)
{
    if (secondNode == nullptr) {
        return;
    }
    auto &modeMap = collaborativeLatencyConfig_[productId];
    while (secondNode->IsNodeValid()) {
        if (!secondNode->IsElementNode() || !secondNode->CompareName("tws_mode")) {
            secondNode->MoveToNext();
            continue;
        }

        std::string twsMode;
        std::string latency;
        if ((secondNode->GetProp("name", twsMode) != SUCCESS) ||
            (secondNode->GetProp("latency_ms", latency) != SUCCESS)) {
            AUDIO_ERR_LOG("twsMode node without name or latency");
            secondNode->MoveToNext();
            continue;
        }

        auto it = twsModeMap.find(twsMode);
        if (it == twsModeMap.end()) {
            AUDIO_ERR_LOG("Invalid twsMode:%{public}s", twsMode.c_str());
            secondNode->MoveToNext();
            continue;
        }

        try {
            modeMap.insert_or_assign(it->second, std::stoi(latency));
        } catch (const std::invalid_argument& e) {
            AUDIO_ERR_LOG("Invalid lantency:%{public}s", latency.c_str());
        } catch (const std::out_of_range& e) {
            AUDIO_ERR_LOG("Out of range lantency:%{public}s", latency.c_str());
        }
        secondNode->MoveToNext();
    }
}

void AudioCollaborationManager::updateLatencyInner()
{
    auto iterProduct = collaborativeLatencyConfig_.find(productId_);
    if (iterProduct == collaborativeLatencyConfig_.end()) {
        latencyMs_ = DEFAULT_LATENCY;
        return;
    }

    auto iterTwsMode = iterProduct->second.find(twsMode_);
    if (iterTwsMode == iterProduct->second.end()) {
        latencyMs_ = DEFAULT_LATENCY;
        return;
    }

    latencyMs_ = iterTwsMode->second;
    AUDIO_INFO_LOG("productId: %{public}s, twsMode: %{public}d, latencyMs: %{public}d",
        productId_.c_str(), twsMode_, latencyMs_);
}

int32_t AudioCollaborationManager::GetCollaborationLatency()
{
    std::lock_guard<std::mutex> lock(collaborationMutex_);
    return latencyMs_;
}

void AudioCollaborationManager::LoadCollAbsSupportedStreamUsageInner(const std::shared_ptr<AudioXmlNode> node)
{
    if (node == nullptr) {
        return;
    }

    std::string deviceTypeStr;
    if (node->GetProp("deviceType", deviceTypeStr) != SUCCESS) {
        AUDIO_ERR_LOG("collAbsSupportedStreamUsage node without deviceType");
        return;
    }
    const std::unordered_map<std::string, DeviceType> &deviceTypeMap = GetSupportedDeviceTypeMP();
    auto devIt = deviceTypeMap.find(deviceTypeStr);
    if (devIt == deviceTypeMap.end()) {
        AUDIO_ERR_LOG("Invalid deviceType:%{public}s", deviceTypeStr.c_str());
        return;
    }
    DeviceType deviceType = devIt->second;

    node->MoveToChildren();
    while (node->IsNodeValid()) {
        if (!node->IsElementNode() || !node->CompareName("streamUsage")) {
            node->MoveToNext();
            continue;
        }

        std::string streamUsageStr;
        if (node->GetProp("name", streamUsageStr) != SUCCESS) {
            AUDIO_ERR_LOG("streamUsage node without name");
            node->MoveToNext();
            continue;
        }

        auto usageIt = streamUsageMap.find(streamUsageStr);
        if (usageIt == streamUsageMap.end()) {
            AUDIO_ERR_LOG("Invalid streamUsage:%{public}s", streamUsageStr.c_str());
            node->MoveToNext();
            continue;
        }

        collAbsSupportedStreamUsageConfig_[deviceType].insert(usageIt->second);
        AUDIO_INFO_LOG("collAbsSupportedStreamUsage: deviceType=%{public}d, streamUsage=%{public}d(%{public}s)",
            deviceType, usageIt->second, streamUsageStr.c_str());
        node->MoveToNext();
    }
}

bool AudioCollaborationManager::IsStreamUsageSupported(DeviceType deviceType, StreamUsage streamUsage)
{
    auto devIt = collAbsSupportedStreamUsageConfig_.find(deviceType);
    if (devIt == collAbsSupportedStreamUsageConfig_.end()) {
        return false;
    }
    return devIt->second.find(streamUsage) != devIt->second.end();
}
}  // namespace AudioStandard
}  // namespace OHOS