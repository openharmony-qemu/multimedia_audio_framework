/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSourceStrategyParser"
#endif

#include "audio_source_strategy_parser.h"
#include "audio_utils.h"
#include "audio_definition_adapter_info.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

const std::unordered_map<std::string, SourceType> AudioSourceStrategyParser::sourceTypeMap = {
    {"SOURCE_TYPE_MIC", SOURCE_TYPE_MIC},
    {"SOURCE_TYPE_CAMCORDER", SOURCE_TYPE_CAMCORDER},
    {"SOURCE_TYPE_VOICE_RECOGNITION", SOURCE_TYPE_VOICE_RECOGNITION},
    {"SOURCE_TYPE_PLAYBACK_CAPTURE", SOURCE_TYPE_PLAYBACK_CAPTURE},
    {"SOURCE_TYPE_WAKEUP", SOURCE_TYPE_WAKEUP},
    {"SOURCE_TYPE_VOICE_COMMUNICATION", SOURCE_TYPE_VOICE_COMMUNICATION},
    {"SOURCE_TYPE_VOICE_CALL", SOURCE_TYPE_VOICE_CALL},
    {"SOURCE_TYPE_ULTRASONIC", SOURCE_TYPE_ULTRASONIC},
    {"SOURCE_TYPE_VIRTUAL_CAPTURE", SOURCE_TYPE_VIRTUAL_CAPTURE},
    {"SOURCE_TYPE_VOICE_MESSAGE", SOURCE_TYPE_VOICE_MESSAGE},
    {"SOURCE_TYPE_VOICE_TRANSCRIPTION", SOURCE_TYPE_VOICE_TRANSCRIPTION},
    {"SOURCE_TYPE_UNPROCESSED", SOURCE_TYPE_UNPROCESSED},
    {"SOURCE_TYPE_LIVE", SOURCE_TYPE_LIVE},
    {"SOURCE_TYPE_EC", SOURCE_TYPE_EC},
    {"SOURCE_TYPE_MIC_REF", SOURCE_TYPE_MIC_REF}
};

AudioSourceStrategyParser::AudioSourceStrategyParser()
{
    curNode_ = AudioXmlNode::Create();
    AUDIO_DEBUG_LOG("AudioSourceStrategyParser ctor");
}

AudioSourceStrategyParser::~AudioSourceStrategyParser()
{
    if (curNode_ != nullptr) {
        curNode_->FreeDoc();
        curNode_ = nullptr;
    }
    AUDIO_DEBUG_LOG("AudioSourceStrategyParser dtor");
}

bool AudioSourceStrategyParser::LoadConfig()
{
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(AUDIO_SOURCE_STRATEGY_CONFIG_FILE, buf, MAX_PATH_LEN);
#else
    const char *path = AUDIO_SOURCE_STRATEGY_CONFIG_FILE;
#endif
    CHECK_AND_RETURN_RET_LOG(path != nullptr && *path != '\0', ERROR, "invalid path!");
    if (curNode_->Config(path, nullptr, 0) != SUCCESS) {
        AUDIO_ERR_LOG("load path: %{public}s fail!", path);
        curNode_->FreeDoc();
        curNode_ = nullptr;
        return false;
    }

    AUDIO_INFO_LOG("LoadConfig enter success");
    CHECK_AND_RETURN_RET_LOG(curNode_->IsNodeValid(), ERROR, "root element is null");
    if (!curNode_->CompareName("audio_source_strategy")) {
        AUDIO_ERR_LOG("Missing tag - audio_source_strategy in : %s", AUDIO_SOURCE_STRATEGY_CONFIG_FILE);
        curNode_->FreeDoc();
        curNode_ = nullptr;
        return false;
    }

    AUDIO_INFO_LOG("LoadConfig audio_source_strategy success");
    curNode_->MoveToChildren();
    CHECK_AND_RETURN_RET_LOG(curNode_->IsNodeValid(), ERROR, "Missing child: %s", AUDIO_SOURCE_STRATEGY_CONFIG_FILE);

    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMap =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    while (curNode_->IsNodeValid()) {
        if (curNode_->CompareName("audio_source_config")) {
            ParseConfig(curNode_->GetChildrenNode(), sourceStrategyMap);
            break;
        } else {
            curNode_->MoveToNext();
        }
    }
    curNode_->FreeDoc();
    curNode_ = nullptr;

    AudioSourceStrategyData &config = AudioSourceStrategyData::GetInstance();
    config.SetSourceStrategyMap(sourceStrategyMap);
    return true;
}

void AudioSourceStrategyParser::ParseSourceStrategyMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &source,
    const std::string &hdiSource, std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> &sourceStrategyMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("item")) {
            AUDIO_DEBUG_LOG("node type: Element, name: %s", curNode->GetName().c_str());
            AddSourceStrategyMap(curNode, source, hdiSource, sourceStrategyMap);
        }
        curNode->MoveToNext();
    }
}

void AudioSourceStrategyParser::ParseConfig(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> &sourceStrategyMap)
{
    if (sourceStrategyMap == nullptr) {
        AUDIO_ERR_LOG("sourceStrategyMap is null");
        return;
    }
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string typeStr_source;
            std::string typeStr_hdiSource;
            curNode->GetProp("source", typeStr_source);
            curNode->GetProp("hdiSource", typeStr_hdiSource);
            AUDIO_INFO_LOG("source type: %{public}s, hdiSource type: %{public}s",
                typeStr_source.c_str(), typeStr_hdiSource.c_str());
            ParseSourceStrategyMap(curNode->GetChildrenNode(), typeStr_source, typeStr_hdiSource, sourceStrategyMap);
        }
        curNode->MoveToNext();
    }
}

void AudioSourceStrategyParser::AddSourceStrategyMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &source,
    const std::string &hdiSource, std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> &sourceStrategyMap)
{
    AUDIO_INFO_LOG("enter");
    if (source.empty() || hdiSource.empty()) {
        AUDIO_ERR_LOG("param null");
        return;
    }

    std::string adapterStr;
    std::string pipeStr;
    std::string priorityStr;
    std::string audioFlagStr;

    curNode->GetProp("adapter", adapterStr);
    curNode->GetProp("pipe", pipeStr);
    curNode->GetProp("audioFlag", audioFlagStr);
    curNode->GetProp("priority", priorityStr);

    int priority = 0;
    if (!priorityStr.empty()) {
        priority = std::stoi(priorityStr);
    }

    auto sourceTypeIt = sourceTypeMap.find(source);
    if (sourceTypeIt == sourceTypeMap.end()) {
        AUDIO_ERR_LOG("sourceType: %{public}s is not in sourceTypeMap", source.c_str());
        return;
    }

    AudioFlag audioFlag;
    auto it = AudioDefinitionPolicyUtils::flagStrToEnum.find(audioFlagStr);
    if (it != AudioDefinitionPolicyUtils::flagStrToEnum.end()) {
        audioFlag = it->second;
    } else {
        AUDIO_ERR_LOG("flagStrToEnum is null");
        return;
    }

    SourceType sourceType = sourceTypeIt->second;
    sourceStrategyMap->emplace(sourceType,
        AudioSourceStrategyType(hdiSource, adapterStr, pipeStr, audioFlag, priority));
    AUDIO_INFO_LOG("sourceType: %{public}d, source: %{public}s, hdiSource: %{public}s, adapterStr: %{public}s, "
        "pipeStr: %{public}s, audioFlag: %{public}u, priority: %{public}d",
        sourceType, source.c_str(), hdiSource.c_str(), adapterStr.c_str(), pipeStr.c_str(), audioFlag,
        priority);
}
} // namespace AudioStandard
} // namespace OHOS
