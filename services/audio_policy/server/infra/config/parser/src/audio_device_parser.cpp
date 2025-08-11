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
#include "audio_errors.h"
#include "audio_utils.h"
#include "config_policy_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static std::map<std::string, DeviceType> deviceTypeMap_ = {
    {"DEVICE_TYPE_WIRED_HEADSET", DEVICE_TYPE_WIRED_HEADSET},
    {"DEVICE_TYPE_WIRED_HEADPHONES", DEVICE_TYPE_WIRED_HEADPHONES},
    {"DEVICE_TYPE_BLUETOOTH_SCO", DEVICE_TYPE_BLUETOOTH_SCO},
    {"DEVICE_TYPE_BLUETOOTH_A2DP", DEVICE_TYPE_BLUETOOTH_A2DP},
    {"DEVICE_TYPE_BLUETOOTH_A2DP_IN", DEVICE_TYPE_BLUETOOTH_A2DP_IN},
    {"DEVICE_TYPE_USB_HEADSET", DEVICE_TYPE_USB_HEADSET},
    {"DEVICE_TYPE_DP", DEVICE_TYPE_DP},
    {"DEVICE_TYPE_HDMI", DEVICE_TYPE_HDMI},
    {"DEVICE_TYPE_LINE_DIGITAL", DEVICE_TYPE_LINE_DIGITAL},
    {"DEVICE_TYPE_USB_ARM_HEADSET", DEVICE_TYPE_USB_ARM_HEADSET},
    {"DEVICE_TYPE_ACCESSORY", DEVICE_TYPE_ACCESSORY},
    {"DEVICE_TYPE_NEARLINK", DEVICE_TYPE_NEARLINK},
    {"DEVICE_TYPE_NEARLINK_IN", DEVICE_TYPE_NEARLINK_IN},
    {"DEVICE_TYPE_HEARING_AID", DEVICE_TYPE_HEARING_AID},
    {"DEVICE_TYPE_REMOTE_DAUDIO", DEVICE_TYPE_REMOTE_DAUDIO},
};
}
bool AudioDeviceParser::LoadConfiguration()
{
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(DEVICE_CONFIG_FILE, buf, MAX_PATH_LEN);
    curNode_ = AudioXmlNode::Create();
    if (path == nullptr || *path == '\0' || curNode_->Config(path, nullptr, 0) != SUCCESS) {
        Trace trace("SYSEVENT FAULT EVENT LOAD_CONFIG_ERROR, CATEGORY: "
            + std::to_string(Media::MediaMonitor::AUDIO_DEVICE_PRIVACY));
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_DEVICE_PRIVACY);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(curNode_->IsNodeValid(), false, "xmlReadFile Failed");
    if (!ParseInternal(curNode_->GetCopyNode())) {
        return false;
    }
    audioDeviceManager_->OnXmlParsingCompleted(devicePrivacyMaps_);
    return true;
}

void AudioDeviceParser::Destroy()
{
    curNode_->FreeDoc();
}

bool AudioDeviceParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode)
{
    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (curNode->IsElementNode()) {
            switch (GetDeviceNodeNameAsInt(curNode)) {
                case ADAPTER:
                    ParseAudioDevicePrivacyType(curNode->GetCopyNode(), devicePrivacyType_);
                    break;
                default:
                    ParseInternal(curNode->GetChildrenNode());
                    break;
            }
        }
    }
    return true;
}

void AudioDeviceParser::ParseDevicePrivacyInfo(std::shared_ptr<AudioXmlNode> curNode,
    std::list<DevicePrivacyInfo> &deviceLists)
{
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            DevicePrivacyInfo deviceInfo = {};
            curNode->GetProp("name", deviceInfo.deviceName);

            std::string pValue;
            curNode->GetProp("type", pValue);
            deviceInfo.deviceType = deviceTypeMap_[pValue];

            curNode->GetProp("role", pValue);
            uint32_t intValue = 0;
            ParseDeviceRole(pValue, intValue);
            deviceInfo.deviceRole = static_cast<DeviceRole>(intValue);

            curNode->GetProp("Category", pValue);
            intValue = 0;
            ParseDeviceCategory(pValue, intValue);
            deviceInfo.deviceCategory = static_cast<DeviceCategory>(intValue);

            curNode->GetProp("usage", pValue);
            intValue = 0;
            ParseDeviceUsage(pValue, intValue);
            deviceInfo.deviceUsage = static_cast<DeviceUsage>(intValue);

            deviceLists.push_back(deviceInfo);
            AUDIO_DEBUG_LOG("AudioDeviceParser: name:%{public}s, type:%{public}d, role:%{public}d, Category:%{public}d,"
                "Usage:%{public}d", deviceInfo.deviceName.c_str(), deviceInfo.deviceType, deviceInfo.deviceRole,
                deviceInfo.deviceCategory, deviceInfo.deviceUsage);
        }
        curNode->MoveToNext();
    }
}

void AudioDeviceParser::ParserDevicePrivacyInfoList(std::shared_ptr<AudioXmlNode> curNode,
    std::list<DevicePrivacyInfo> &deviceLists)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("devices")) {
            ParseDevicePrivacyInfo(curNode->GetChildrenNode(), deviceLists);
        }
        curNode->MoveToNext();
    }
}

void AudioDeviceParser::ParseAudioDevicePrivacyType(std::shared_ptr<AudioXmlNode> curNode,
    AudioDevicePrivacyType &deviceType)
{
    while (curNode->IsNodeValid()) {
        //read deviceType
        if (curNode->CompareName("adapter")) {
            std::string adapterName;
            CHECK_AND_RETURN_LOG(curNode->GetProp("name", adapterName) == SUCCESS, "get prop adapterName fail!");
            AUDIO_DEBUG_LOG("AudioDeviceParser: adapter name: %{public}s", adapterName.c_str());
            devicePrivacyType_ = GetDevicePrivacyType(adapterName);
            std::list<DevicePrivacyInfo> deviceLists = {};

            ParserDevicePrivacyInfoList(curNode->GetChildrenNode(), deviceLists);
            devicePrivacyMaps_[devicePrivacyType_] = deviceLists;
        } else {
            return;
        }
        curNode->MoveToNext();
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

DeviceNodeName AudioDeviceParser::GetDeviceNodeNameAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("adapter")) {
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
        } else if (category == "HEARAID") {
            deviceCategoryFlag |= DeviceCategory::BT_HEARAID;
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
            deviceUsageFlag |= DeviceUsage::RECOGNITION;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS