/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioParamParser"
#endif

#include "audio_service_log.h"
#include "config/audio_param_parser.h"
#include "audio_errors.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif

namespace OHOS {
namespace AudioStandard {
AudioParamParser::AudioParamParser()
{
    AUDIO_DEBUG_LOG("audio extra parameters constructor");
}

AudioParamParser::~AudioParamParser()
{
    AUDIO_DEBUG_LOG("audio extra parameters destructor");
}

bool AudioParamParser::LoadConfiguration(
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    AUDIO_INFO_LOG("start LoadConfiguration");
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    int32_t ret = 0;

#ifdef USE_CONFIG_POLICY
    CfgFiles *cfgFiles = GetCfgFiles(CONFIG_FILE);
    if (cfgFiles == nullptr) {
        AUDIO_ERR_LOG("Not found audio_param_config.xml");
        return false;
    }

    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        if (cfgFiles->paths[i] && *(cfgFiles->paths[i]) != '\0') {
            AUDIO_INFO_LOG("extra parameter config file path: %{public}s", cfgFiles->paths[i]);
            ret = curNode->Config(cfgFiles->paths[i], nullptr, 0);
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
#endif

    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Load Config Failed");
        curNode = nullptr;
        return false;
    }

    if (!ParseInternal(curNode->GetCopyNode(), audioParameterKeys)) {
        curNode = nullptr;
        return false;
    }

    curNode = nullptr;
    return true;
}

bool AudioParamParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    if (!curNode->IsNodeValid()) {
        AUDIO_ERR_LOG("parse node is null");
        return false;
    }

    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (curNode->CompareName("mainkeys")) {
            ParseMainKeys(curNode->GetCopyNode(), audioParameterKeys);
        } else {
            ParseInternal(curNode->GetChildrenNode(), audioParameterKeys);
        }
    }

    return true;
}

void AudioParamParser::ParseMainKeys(std::shared_ptr<AudioXmlNode> curNode,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ParseMainKey(curNode->GetCopyNode(), audioParameterKeys);
        }
        curNode->MoveToNext();
    }
}

void AudioParamParser::ParseMainKey(std::shared_ptr<AudioXmlNode> curNode,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    std::string mainKeyName;
    CHECK_AND_RETURN_LOG(curNode->GetProp("name", mainKeyName) == SUCCESS,
        "get mainKeyName: %{public}s fail", mainKeyName.c_str());

    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ParseSubKeys(curNode->GetCopyNode(), mainKeyName, audioParameterKeys);
        }
        curNode->MoveToNext();
    }
}

void AudioParamParser::ParseSubKeys(std::shared_ptr<AudioXmlNode> curNode, std::string &mainKeyName,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    std::unordered_map<std::string, std::set<std::string>> subKeyMap = {};
    std::set<std::string> supportedUsage;
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string subKeyName;
            std::string usage;
            std::regex regexDelimiter(",");
            curNode->GetProp("name", subKeyName);
            curNode->GetProp("usage", usage);

            const std::sregex_token_iterator itEnd;
            for (std::sregex_token_iterator it(usage.begin(), usage.end(), regexDelimiter, -1); it != itEnd; it++) {
                supportedUsage.insert(it->str());
            }
            subKeyMap.emplace(subKeyName, supportedUsage);
            supportedUsage.clear();
        }
        curNode->MoveToNext();
    }
    audioParameterKeys.emplace(mainKeyName, subKeyMap);
}
}  // namespace AudioStandard
}  // namespace OHOS
