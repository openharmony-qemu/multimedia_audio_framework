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
#define LOG_TAG "audioAffinityParser"
#endif

#include "audio_affinity_parser.h"
#include "audio_errors.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

static std::map<std::string, DeviceType> deviceTypeMap_ = {
    {"DEVICE_TYPE_EARPIECE", DEVICE_TYPE_EARPIECE},
    {"DEVICE_TYPE_SPEAKER", DEVICE_TYPE_SPEAKER},
    {"DEVICE_TYPE_WIRED_HEADSET", DEVICE_TYPE_WIRED_HEADSET},
    {"DEVICE_TYPE_WIRED_HEADPHONES", DEVICE_TYPE_WIRED_HEADPHONES},
    {"DEVICE_TYPE_BLUETOOTH_SCO", DEVICE_TYPE_BLUETOOTH_SCO},
    {"DEVICE_TYPE_BLUETOOTH_A2DP", DEVICE_TYPE_BLUETOOTH_A2DP},
    {"DEVICE_TYPE_BLUETOOTH_A2DP_IN", DEVICE_TYPE_BLUETOOTH_A2DP_IN},
    {"DEVICE_TYPE_USB_HEADSET", DEVICE_TYPE_USB_HEADSET},
    {"DEVICE_TYPE_USB_ARM_HEADSET", DEVICE_TYPE_USB_ARM_HEADSET},
    {"DEVICE_TYPE_DP", DEVICE_TYPE_DP},
    {"DEVICE_TYPE_REMOTE_CAST", DEVICE_TYPE_REMOTE_CAST},
    {"DEVICE_TYPE_MIC", DEVICE_TYPE_MIC},
};

bool audioAffinityParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(AFFINITY_CONFIG_FILE, nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(mDoc_ != nullptr, false, "audioAffinityParser xmlReadFile failed");

    return true;
}

bool audioAffinityParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(mDoc_);
    CHECK_AND_RETURN_RET_LOG(root != nullptr, false, "xmlDocGetRootElement Failed");

    xmlNode *currNode = nullptr;
    if (root->xmlChildrenNode) {
        currNode = root->xmlChildrenNode;
    } else {
        AUDIO_ERR_LOG("audioAffinityParser Missing node");
        return false;
    }
    if (!ParseInternal(currNode)) {
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(audioAffinityManager_ != nullptr, false, "audioAffinityManager_ is null");
    audioAffinityManager_->OnXmlParsingCompleted(affinityDeviceInfoArray_);
    return true;
}

void audioAffinityParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
}

bool audioAffinityParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;
    while (currNode != nullptr) {
        if (XML_ELEMENT_NODE == currNode->type &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("OutputDevices")))) {
            ParserAffinityGroups(currNode, OUTPUT_DEVICES_FLAG);
        } else if (XML_ELEMENT_NODE == currNode->type &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("InputDevices")))) {
            ParserAffinityGroups(currNode, INPUT_DEVICES_FLAG);
        }
        currNode = currNode->next;
    }
    return true;
}

void audioAffinityParser::ParserAffinityGroups(xmlNode *node, const DeviceFlag& deviceFlag)
{
    xmlNode *currNode = nullptr;
    if (node->xmlChildrenNode) {
        currNode = node->xmlChildrenNode;
    } else {
        AUDIO_ERR_LOG("audioAffinityParser Missing node groups");
        return;
    }

    while (currNode) {
        if (XML_ELEMENT_NODE == currNode->type &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("AffinityGroups")))) {
            ParserAffinityGroupAttribute(currNode, deviceFlag);
        }
        currNode = currNode->next;
    }
}

void audioAffinityParser::ParserAffinityGroupAttribute(xmlNode *node, const DeviceFlag& deviceFlag)
{
    xmlNode *currNode = nullptr;
    if (node->xmlChildrenNode) {
        currNode = node->xmlChildrenNode;
    } else {
        AUDIO_ERR_LOG("audioAffinityParser Missing node attr");
        return;
    }

    AffinityDeviceInfo deviceInfo = {};
    deviceInfo.deviceFlag = deviceFlag;
    while (currNode) {
        if (XML_ELEMENT_NODE == currNode->type &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("AffinityGroup")))) {
            xmlChar *attrPrimary = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("isPrimary"));
            if (attrPrimary != nullptr) {
                deviceInfo.isPrimary = static_cast<uint32_t>(atoi(reinterpret_cast<char *>(attrPrimary)));
                xmlFree(attrPrimary);
            }
            xmlChar *attrGroupName = xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name"));
            if (attrGroupName != nullptr) {
                deviceInfo.groupName = reinterpret_cast<char *>(attrGroupName);
                xmlFree(attrGroupName);
            }
            ParserAffinityGroupDeviceInfos(currNode, deviceInfo);
        }
        currNode = currNode->next;
    }
}

void audioAffinityParser::ParserAffinityGroupDeviceInfos(xmlNode *node, AffinityDeviceInfo& deviceInfo)
{
    xmlNode *currNode = nullptr;
    if (node->xmlChildrenNode) {
        currNode = node->xmlChildrenNode;
    } else {
        AUDIO_ERR_LOG("audioAffinityParser Missing node device");
        return;
    }

    while (currNode) {
        if (XML_ELEMENT_NODE == currNode->type &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("Affinity")))) {
            char *pValue = reinterpret_cast<char *>(
                xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("networkId")));
            deviceInfo.networkID = pValue;
            xmlFree(pValue);

            pValue = reinterpret_cast<char *>(
                xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("deviceType")));
            std::map<std::string, DeviceType>::iterator item = deviceTypeMap_.find(std::string(pValue));
            deviceInfo.deviceType = (item != deviceTypeMap_.end() ? item->second : DEVICE_TYPE_INVALID);
            xmlFree(pValue);

            pValue = reinterpret_cast<char *>(
                xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("supportedConcurrency")));
            deviceInfo.SupportedConcurrency = (std::string(pValue) == "True") ? true : false;
            xmlFree(pValue);

            affinityDeviceInfoArray_.push_back(deviceInfo);
        }
        currNode = currNode->next;
    }
}

} // namespace AudioStandard
} // namespace OHOS
