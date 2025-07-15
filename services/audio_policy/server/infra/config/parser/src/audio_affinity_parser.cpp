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
#define LOG_TAG "AudioAffinityParser"
#endif

#include "audio_affinity_parser.h"

#include "media_monitor_manager.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

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
    {"DEVICE_TYPE_ACCESSORY", DEVICE_TYPE_ACCESSORY},
    {"DEVICE_TYPE_MIC", DEVICE_TYPE_MIC},
    {"DEVICE_TYPE_HDMI", DEVICE_TYPE_HDMI},
    {"DEVICE_TYPE_HEARING_AID", DEVICE_TYPE_HEARING_AID},
};

bool AudioAffinityParser::LoadConfiguration()
{
    bool ret = curNode_->Config(AFFINITY_CONFIG_FILE, nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "AudioAffinityParser xmlReadFile failed");

    curNode_->MoveToChildren();
    CHECK_AND_RETURN_RET_LOG(curNode_->IsNodeValid(), false, "AudioAffinityParser Missing node");
    if (!ParseInternal(curNode_->GetCopyNode())) {
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(audioAffinityManager_ != nullptr, false, "audioAffinityManager_ is null");
    audioAffinityManager_->OnXmlParsingCompleted(affinityDeviceInfoArray_);
    return true;
}

void AudioAffinityParser::Destroy()
{
    curNode_->FreeDoc();
}

bool AudioAffinityParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("OutputDevices")) {
            ParserAffinityGroups(curNode->GetCopyNode(), OUTPUT_DEVICES_FLAG);
        } else if (curNode->CompareName("InputDevices")) {
            ParserAffinityGroups(curNode->GetCopyNode(), INPUT_DEVICES_FLAG);
        }
        curNode->MoveToNext();
    }
    return true;
}

void AudioAffinityParser::ParserAffinityGroups(std::shared_ptr<AudioXmlNode> curNode, const DeviceFlag& deviceFlag)
{
    curNode->MoveToChildren();
    CHECK_AND_RETURN_LOG(curNode->IsNodeValid(), "audioAffinityParser Missing node groups");

    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("AffinityGroups")) {
            ParserAffinityGroupAttribute(curNode->GetCopyNode(), deviceFlag);
        }
        curNode->MoveToNext();
    }
}

void AudioAffinityParser::ParserAffinityGroupAttribute(std::shared_ptr<AudioXmlNode> curNode,
    const DeviceFlag& deviceFlag)
{
    curNode->MoveToChildren();
    CHECK_AND_RETURN_LOG(curNode->IsNodeValid(), "audioAffinityParser Missing node attr");

    AffinityDeviceInfo deviceInfo = {};
    deviceInfo.deviceFlag = deviceFlag;
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("AffinityGroup")) {
            std::string attrPrimary;
            if (curNode->GetProp("isPrimary", attrPrimary) == SUCCESS) {
                deviceInfo.isPrimary = (attrPrimary == "True" ? true : false);
            }
            std::string attrGroupName;
            if (curNode->GetProp("name", attrGroupName) == SUCCESS) {
                deviceInfo.groupName = attrGroupName;
            }
            ParserAffinityGroupDeviceInfos(curNode->GetCopyNode(), deviceInfo);
        }
        curNode->MoveToNext();
    }
}

void AudioAffinityParser::ParserAffinityGroupDeviceInfos(std::shared_ptr<AudioXmlNode> curNode,
    AffinityDeviceInfo& deviceInfo)
{
    curNode->MoveToChildren();
    CHECK_AND_RETURN_LOG(curNode->IsNodeValid(), "audioAffinityParser Missing node device");

    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("Affinity")) {
            curNode->GetProp("networkId", deviceInfo.networkID);

            std::string deviceType;
            curNode->GetProp("deviceType", deviceType);
            std::map<std::string, DeviceType>::iterator item = deviceTypeMap_.find(deviceType);
            deviceInfo.deviceType = (item != deviceTypeMap_.end() ? item->second : DEVICE_TYPE_INVALID);

            std::string supportedConcurrency;
            curNode->GetProp("supportedConcurrency", supportedConcurrency);
            deviceInfo.SupportedConcurrency = (supportedConcurrency == "True") ? true : false;
            
            affinityDeviceInfoArray_.push_back(deviceInfo);
        }
        curNode->MoveToNext();
    }
}

} // namespace AudioStandard
} // namespace OHOS
