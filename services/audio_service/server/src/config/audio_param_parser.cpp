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
    xmlDoc *doc = nullptr;

#ifdef USE_CONFIG_POLICY
    CfgFiles *cfgFiles = GetCfgFiles(CONFIG_FILE);
    if (cfgFiles == nullptr) {
        AUDIO_ERR_LOG("Not found audio_param_config.xml");
        return false;
    }

    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        if (cfgFiles->paths[i] && *(cfgFiles->paths[i]) != '\0') {
            AUDIO_INFO_LOG("extra parameter config file path: %{public}s", cfgFiles->paths[i]);
            doc = xmlReadFile(cfgFiles->paths[i], nullptr, 0);
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
#endif

    if (doc == nullptr) {
        AUDIO_ERR_LOG("xmlReadFile Failed");
        return false;
    }

    xmlNode *root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        AUDIO_ERR_LOG("xmlDocGetRootElement Failed");
        xmlFreeDoc(doc);
        return false;
    }

    if (!ParseInternal(root, audioParameterKeys)) {
        xmlFreeDoc(doc);
        return false;
    }

    xmlFreeDoc(doc);
    return true;
}

bool AudioParamParser::ParseInternal(xmlNode *node,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    xmlNode *currNode = node;
    if (currNode == nullptr) {
        AUDIO_ERR_LOG("parse node is null");
        return false;
    }

    for (; currNode; currNode = currNode->next) {
        if (XML_ELEMENT_NODE == currNode->type &&
            !xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("mainkeys"))) {
            ParseMainKeys(currNode, audioParameterKeys);
        } else {
            ParseInternal(currNode->xmlChildrenNode, audioParameterKeys);
        }
    }

    return true;
}

void AudioParamParser::ParseMainKeys(xmlNode *node,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    xmlNode* mainKeysNode = node->xmlChildrenNode;
    while (mainKeysNode != nullptr) {
        if (mainKeysNode->type == XML_ELEMENT_NODE) {
            ParseMainKey(mainKeysNode, audioParameterKeys);
        }
        mainKeysNode = mainKeysNode->next;
    }
}

void AudioParamParser::ParseMainKey(xmlNode *node,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    std::string mainKeyName = ExtractPropertyValue("name", *node);
    if (mainKeyName.empty()) {
        AUDIO_ERR_LOG("No name provided for the main key %{public}s", node->name);
        return;
    }

    xmlNode *mainKeyNode = node->xmlChildrenNode;
    while (mainKeyNode != nullptr) {
        if (mainKeyNode->type == XML_ELEMENT_NODE) {
            ParseSubKeys(mainKeyNode, mainKeyName, audioParameterKeys);
        }
        mainKeyNode = mainKeyNode->next;
    }
}

void AudioParamParser::ParseSubKeys(xmlNode *node, std::string &mainKeyName,
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys)
{
    std::unordered_map<std::string, std::set<std::string>> subKeyMap = {};
    std::set<std::string> supportedUsage;
    xmlNode *subKeyNode = node->xmlChildrenNode;

    while (subKeyNode != nullptr) {
        if (subKeyNode->type == XML_ELEMENT_NODE) {
            std::string subKeyName = ExtractPropertyValue("name", *subKeyNode);
            std::regex regexDelimiter(",");
            std::string usage = ExtractPropertyValue("usage", *subKeyNode);
            const std::sregex_token_iterator itEnd;
            for (std::sregex_token_iterator it(usage.begin(), usage.end(), regexDelimiter, -1); it != itEnd; it++) {
                supportedUsage.insert(it->str());
            }
            subKeyMap.emplace(subKeyName, supportedUsage);
            supportedUsage.clear();
        }
        subKeyNode = subKeyNode->next;
    }
    audioParameterKeys.emplace(mainKeyName, subKeyMap);
}

std::string AudioParamParser::ExtractPropertyValue(const std::string &propName, xmlNode &node)
{
    std::string propValue = "";
    xmlChar *tempValue = nullptr;

    if (xmlHasProp(&node, reinterpret_cast<const xmlChar*>(propName.c_str()))) {
        tempValue = xmlGetProp(&node, reinterpret_cast<const xmlChar*>(propName.c_str()));
    }

    if (tempValue != nullptr) {
        propValue = reinterpret_cast<const char*>(tempValue);
        xmlFree(tempValue);
    }

    return propValue;
}
}  // namespace AudioStandard
}  // namespace OHOS
