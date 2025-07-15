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
#define LOG_TAG "AudioPolicyGlobalParser"
#endif

#include "audio_policy_global_parser.h"

#include "media_monitor_manager.h"

#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

constexpr int32_t CHANGE_NUMBER_SYSTEM = 10;

bool AudioPolicyGlobalParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(POLICY_GLOBAL_CONFIG_FILE, nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(mDoc_ != nullptr, false, "AudioPolicyGlobalParser xmlReadFile failed");

    return true;
}

bool AudioPolicyGlobalParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(mDoc_);
    CHECK_AND_RETURN_RET_LOG(root != nullptr, false, "xmlDocGetRootElement Failed");

    xmlNode *currNode = nullptr;
    if (root->xmlChildrenNode) {
        currNode = root->xmlChildrenNode;
    } else {
        AUDIO_ERR_LOG("AudioPolicyGlobalParser Missing node");
        return false;
    }
    if (!ParseInternal(currNode)) {
        return false;
    }
    return true;
}

void AudioPolicyGlobalParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
}

bool AudioPolicyGlobalParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;
    while (currNode != nullptr) {
        if (XML_ELEMENT_NODE == currNode->type &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("attribute")))) {
            ParserAttribute(currNode);
        }
        currNode = currNode->next;
    }
    return true;
}

void AudioPolicyGlobalParser::ParserAttribute(xmlNode *currNode)
{
    while (currNode) {
        std::string name;
        uint32_t value;
        xmlChar *attrName = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name"));
        xmlChar *attrValue = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("value"));
        if (attrName == nullptr || attrValue == nullptr) {
            if (attrName != nullptr) {
                xmlFree(attrName);
            }
            if (attrValue != nullptr) {
                xmlFree(attrValue);
            }
            currNode = currNode->next;
            continue;
        }
        name = static_cast<char *>(reinterpret_cast<char *>(attrName));
        value = static_cast<uint32_t>(std::strtol(reinterpret_cast<char *>(attrValue), nullptr, CHANGE_NUMBER_SYSTEM));
        globalConfigs_[name] = value;
        xmlFree(attrName);
        xmlFree(attrValue);
        currNode = currNode->next;
    }
}

int32_t AudioPolicyGlobalParser::GetConfigByKeyName(std::string keyName, uint32_t &value)
{
    if (!globalConfigs_.count(keyName)) {
        AUDIO_ERR_LOG("GetConfigByKeyName key error keyName=%{public}s", keyName.c_str());
        return ERR_CONFIG_NAME_ERROR;
    }
    value = globalConfigs_[keyName];
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
