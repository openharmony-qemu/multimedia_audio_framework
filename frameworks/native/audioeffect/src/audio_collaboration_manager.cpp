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
#define LOG_TAG "AudioEffectChainManager"
#endif

#include "audio_collaboration_manager.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_effect_log.h"
#include "audio_utils.h"
#include "audio_xml_parser.h"

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

static std::map<std::string, AudioEarphoneProduct> productIdToEarphoneMap = {
    {"00014B", EARPHONE_PRODUCT_DEVO},
    {"000167", EARPHONE_PRODUCT_ROBIN},
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

void AudioCollaborationManager::updateCollaborativeProductId(const std::string &productId)
{
    std::lock_guard<std::mutex> lock(collaborationMutex_);
    auto pos = productId.find('_');
    std::string tmProductId = (pos == std::string::npos) ? productId : productId.substr(0, pos);
    CHECK_AND_RETURN_LOG(productId_ != tmProductTd, "same productId, return.");
    productId_ = tmProductId;
    if (productIdToEarphoneMap.find(productId_) != productIdToEarphoneMap.end()) {
        earphoneProduct_ = productIdToEarphoneMap[productId_];
    } else {
        AUDIO_INFO_LOG("productId no found in productIdToEarphoneMap");
    }
    updateLatencyInner();
    AudioEffectChainManager::GetInstance()->UpdateEarphoneProduct(earphoneProduct_);
    AUDIO_INFO_LOG("productId: %{public}s, earphoneProduct: %{public}d, latencyMs: %{public}d",
        productId_.c_str(), earphoneProduct_, latencyMs_);
}

void AudioCollaborationManager::LoadCollaborationConfig()
{
    std::lock_guard<std::mutex> lock(collaborationMutex_);
    AUDIO_INFO_LOG("begin loadCollaborationConfig");
    collaborativeLatencyConfig_.clear();
    std::shared_ptr<AudioXmlNode> firstNode = AudioXmlNode::Create();
    firstNode->Config(AUDIO_COLLABORATION_CONFIG_FILE, nullptr, XML_PARSE_ERROR | XML_PARSE_NOWARNING);
    if (!firstNode->IsNodeValid()) {
        AUDIO_ERR_LOG("could not parse file %{public}s", AUDIO_COLLABORATION_CONFIG_FILE);
        return;
    }

    firstNode->MoveToChildren();
    while (firstNode->IsNodeValid()) {
        if (!firstNode->IsElementNode()) {
            firstNode->MoveToNext();
            continue;
        }
        
        if (!firstNode->CompareName("product")) {
            firstNode->MoveToNext();
            continue;
        }
        std::string productId;
        if (firstNode->GetProp("name", productId) != SUCCESS) {
            AUDIO_ERR_LOG("product node without name");
            firstNode->MoveToNext();
            continue;
        }
        auto &modeMap = collaborativeLatencyConfig_[productId];
        std::shared_ptr<AudioXmlNode> secondNode = firstNode->GetCopyNode();
        secondNode->MoveToChildren();

        while (secondNode->IsNodeValid()) {
            if (!secondNode->IsElementNode()) {
                secondNode->MoveToNext();
                continue;
            }

            if (!secondNode->CompareName("tws_mode")) {
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

            modeMap.insert_or_assign(twsModeMap[twsMode], std::stoi(latency));
            secondNode->MoveToNext();
        }

        firstNode->MoveToNext();
    }

    for (auto iter1 : collaborativeLatencyConfig_) {
        for (auto iter2 : iter1.second) {
            AUDIO_INFO_LOG("productId: %{public}s, twsMode: %{public}d, latencyMs: %{public}d",
                iter1.first.c_str(), iter2.first, iter2.second);
        }
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

}  // namespace AudioStandard
}  // namespace OHOS