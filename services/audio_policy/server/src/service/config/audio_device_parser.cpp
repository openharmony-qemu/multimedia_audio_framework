/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDeviceParser"
#endif

#include "audio_device_parser.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
bool AudioDeviceParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(DEVICE_CONFIG_FILE, nullptr, 0);
    if (mDoc_ == nullptr) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_DEVICE_PRIVACY);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    }
    CHECK_AND_RETURN_RET_LOG(mDoc_ != nullptr, false,
        "xmlReadFile Failed");

    return true;
}

bool AudioDeviceParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(mDoc_);
    CHECK_AND_RETURN_RET_LOG(root != nullptr, false,
        "xmlDocGetRootElement Failed");

    if (!ParseInternal(root)) {
        return false;
    }
    audioDeviceManager_->OnXmlParsingCompleted(devicePrivacyMaps_);
    return true;
}

void AudioDeviceParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
}

bool AudioDeviceParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;
    for (; currNode; currNode = currNode->next) {
        if (XML_ELEMENT_NODE == currNode->type) {
            switch (GetDeviceNodeNameAsInt(currNode)) {
                case ADAPTER:
                    ParseAudioDevicePrivacyType(currNode, devicePrivacyType_);
                    break;
                default:
                    ParseInternal((currNode->xmlChildrenNode));
                    break;
            }
        }
    }
    return true;
}

void AudioDeviceParser::ParseDevicePrivacyInfo(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists)
{
    xmlNode *deviceNode = node;

    while (deviceNode != nullptr) {
        if (deviceNode->type == XML_ELEMENT_NODE) {
            DevicePrivacyInfo deviceInfo = {};
            char *pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            deviceInfo.deviceName = pValue;
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));
            deviceInfo.deviceType = deviceTypeMap_[pValue];
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("role"))));
            uint32_t intValue = 0;
            ParseDeviceRole(pValue, intValue);
            deviceInfo.deviceRole = static_cast<DeviceRole>(intValue);
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("Category"))));
            intValue = 0;
            ParseDeviceCategory(pValue, intValue);
            deviceInfo.deviceCategory = static_cast<DeviceCategory>(intValue);
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("usage"))));
            intValue = 0;
            ParseDeviceUsage(pValue, intValue);
            deviceInfo.deviceUsage = static_cast<DeviceUsage>(intValue);
            xmlFree(pValue);
            deviceLists.push_back(deviceInfo);
            AUDIO_DEBUG_LOG("AudioDeviceParser: name:%{public}s, type:%{public}d, role:%{public}d, Category:%{public}d,"
                "Usage:%{public}d", deviceInfo.deviceName.c_str(), deviceInfo.deviceType, deviceInfo.deviceRole,
                deviceInfo.deviceCategory, deviceInfo.deviceUsage);
        }
        deviceNode = deviceNode->next;
    }
}

void AudioDeviceParser::ParserDevicePrivacyInfoList(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists)
{
    xmlNode *currentNode = node;
    while (currentNode != nullptr) {
        if (currentNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currentNode->name, reinterpret_cast<const xmlChar*>("devices")))) {
            ParseDevicePrivacyInfo(currentNode->xmlChildrenNode, deviceLists);
        }
        currentNode = currentNode->next;
    }
}

void AudioDeviceParser::ParseAudioDevicePrivacyType(xmlNode *node, AudioDevicePrivacyType &deviceType)
{
    xmlNode *currNode = node;
    std::string adapterName = ExtractPropertyValue("name", currNode);

    while (currNode != nullptr) {
        //read deviceType
        if (currNode->type == XML_ELEMENT_NODE &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("adapter")))) {
            if (adapterName.empty()) {
                AUDIO_ERR_LOG("AudioDeviceParser: No name provided for the adapter %{public}s", node->name);
                return;
            } else {
                AUDIO_DEBUG_LOG("AudioDeviceParser: adapter name: %{public}s", adapterName.c_str());
                devicePrivacyType_ = GetDevicePrivacyType(adapterName);
                std::list<DevicePrivacyInfo> deviceLists = {};
                ParserDevicePrivacyInfoList(currNode->xmlChildrenNode, deviceLists);
                devicePrivacyMaps_[devicePrivacyType_] = deviceLists;
            }
        } else {
            return;
        }
        currNode = currNode->next;
    }
}

AudioDevicePrivacyType AudioDeviceParser::GetDevicePrivacyType(const std::string &devicePrivacyType)
{
    if (devicePrivacyType == PRIVACY_TYPE) {
        return AudioDevicePrivacyType::TYPE_PRIVACY;
    } else if (devicePrivacyType == PUBLIC_TYPE) {
        return AudioDevicePrivacyType::TYPE_PUBLIC;
    } else {
        return AudioDevicePrivacyType::TYPE_NEGATIVE;
    }
}

std::string AudioDeviceParser::ExtractPropertyValue(const std::string &propName, xmlNode *node)
{
    std::string propValue = "";
    xmlChar *tempValue = nullptr;

    if (xmlHasProp(node, reinterpret_cast<const xmlChar*>(propName.c_str()))) {
        tempValue = xmlGetProp(node, reinterpret_cast<const xmlChar*>(propName.c_str()));
    }

    if (tempValue != nullptr) {
        propValue = reinterpret_cast<const char*>(tempValue);
        xmlFree(tempValue);
    }

    return propValue;
}

DeviceNodeName AudioDeviceParser::GetDeviceNodeNameAsInt(xmlNode *node)
{
    if (!xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>("adapter"))) {
        return DeviceNodeName::ADAPTER;
    } else {
        return DeviceNodeName::UNKNOWN_NODE;
    }
}

std::vector<std::string> split(const std::string &line, const std::string &sep)
{
    std::vector<std::string> buf;
    size_t temp = 0;
    std::string::size_type pos = 0;
    while (true) {
        pos = line.find(sep, temp);
        if (pos == std::string::npos) {
            break;
        }
        buf.push_back(line.substr(temp, pos-temp));
        temp = pos + sep.length();
    }
    buf.push_back(line.substr(temp, line.length()));
    return buf;
}

void AudioDeviceParser::ParseDeviceRole(const std::string &deviceRole, uint32_t &deviceRoleFlag)
{
    std::vector<std::string> buf = split(deviceRole, ",");
    for (const auto &role : buf) {
        if (role == "output") {
            deviceRoleFlag |= DeviceRole::OUTPUT_DEVICE;
        } else if (role == "input") {
            deviceRoleFlag |= DeviceRole::INPUT_DEVICE;
        }
    }
}

void AudioDeviceParser::ParseDeviceCategory(const std::string &deviceCategory, uint32_t &deviceCategoryFlag)
{
    std::vector<std::string> buf = split(deviceCategory, ",");
    for (const auto &category : buf) {
        if (category == "HEADPHONE") {
            deviceCategoryFlag |= DeviceCategory::BT_HEADPHONE;
        } else if (category == "GLASSES") {
            deviceCategoryFlag |= DeviceCategory::BT_GLASSES;
        } else if (category == "SOUNDBOX") {
            deviceCategoryFlag |= DeviceCategory::BT_SOUNDBOX;
        } else if (category == "CAR") {
            deviceCategoryFlag |= DeviceCategory::BT_CAR;
        } else if (category == "HEADPHONE_UNWEAR") {
            deviceCategoryFlag |= DeviceCategory::BT_UNWEAR_HEADPHONE;
        } else if (category == "WATCH") {
            deviceCategoryFlag |= DeviceCategory::BT_WATCH;
        }
    }
}

void AudioDeviceParser::ParseDeviceUsage(const std::string &deviceUsage, uint32_t &deviceUsageFlag)
{
    std::vector<std::string> buf = split(deviceUsage, ",");
    for (const auto &usage : buf) {
        if (usage == "media") {
            deviceUsageFlag |= DeviceUsage::MEDIA;
        } else if (usage == "voice") {
            deviceUsageFlag |= DeviceUsage::VOICE;
        } else if (usage == "recongnition") {
            deviceUsageFlag |= DeviceUsage::RECONGNITION;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS