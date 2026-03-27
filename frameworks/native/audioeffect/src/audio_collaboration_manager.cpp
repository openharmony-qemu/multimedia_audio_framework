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
#define LOG_TAG "AudioCollaborationManager "
#endif

#include "audio_collaboration_manager.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_effect_log.h"
#include "audio_utils.h"

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
        std::string productId;
        std::string earphoneType;
        std::string earphoneName;
        if (!node->IsElementNode() || !node->CompareName("product") ||
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
}  // namespace AudioStandard
}  // namespace OHOS