/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "audio_affinity_manager.h"
#include "parameter.h"

#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_affinity_parser.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr int32_t MS_PER_S = 1000;
constexpr int32_t NS_PER_MS = 1000000;
static int64_t GetCurrentTimeMS()
{
    timespec tm{};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

void AudioAffinityManager::ParseAffinityXml()
{
    unique_ptr<AudioAffinityParser> affinityParser = make_unique<AudioAffinityParser>(this);
    if (affinityParser->LoadConfiguration()) {
        AUDIO_INFO_LOG("Audio Affinity manager load configuration successfully.");
    }
    OnXmlParsingCompleted(affinityParser->GetAffinityDeviceInfo());
}

void AudioAffinityManager::OnXmlParsingCompleted(std::vector<AffinityDeviceInfo> &xmlData)
{
    CHECK_AND_RETURN_LOG(!xmlData.empty(), "Failed to parse xml file.");
    for (AffinityDeviceInfo deviceInfo : xmlData) {
        if (deviceInfo.deviceFlag == OUTPUT_DEVICES_FLAG) {
            rendererAffinityDeviceArray_.push_back(deviceInfo);
        } else {
            capturerAffinityDeviceArray_.push_back(deviceInfo);
        }
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioAffinityManager::GetRendererDevice(int32_t clientUID)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    int32_t affinityClientUID = clientUID;
    AFFINITYDEVMAP::iterator item = activeRendererDeviceMap_.find(clientUID);
    if (item != activeRendererDeviceMap_.end()) {
        if (item->second == nullptr) {
            AUDIO_INFO_LOG("AudioDeviceDescriptor sptr not valid");
            return make_shared<AudioDeviceDescriptor>();
        }
        shared_ptr<AudioDeviceDescriptor> desc =
            make_shared<AudioDeviceDescriptor>(activeRendererDeviceMap_[affinityClientUID]);
        return desc;
    } else {
        return make_shared<AudioDeviceDescriptor>();
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioAffinityManager::GetCapturerDevice(int32_t clientUID)
{
    std::lock_guard<std::mutex> lock(capturerMapMutex_);
    int32_t affinityClientUID = clientUID;
    AFFINITYDEVMAP::iterator item = activeCapturerDeviceMap_.find(clientUID);
    if (item != activeCapturerDeviceMap_.end()) {
        if (item->second == nullptr) {
            AUDIO_INFO_LOG("AudioDeviceDescriptor sptr not valid");
            return make_shared<AudioDeviceDescriptor>();
        }
        shared_ptr<AudioDeviceDescriptor> desc =
            make_shared<AudioDeviceDescriptor>(activeCapturerDeviceMap_[affinityClientUID]);
        return desc;
    } else {
        return make_shared<AudioDeviceDescriptor>();
    }
}

void AudioAffinityManager::AddSelectRendererDevice(
    int32_t clientUID, const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    CHECK_AND_RETURN_LOG(desc != nullptr, "AudioDeviceDescriptor sptr not valid");

    activeRendererDeviceMap_[clientUID] = desc;

    AffinityDeviceInfo affinityDeviceInfo =
        GetAffinityDeviceInfoByDeviceType(rendererAffinityDeviceArray_, desc->getType(), desc->networkId_);

    affinityDeviceInfo.chooseTimeStamp = static_cast<uint64_t>(GetCurrentTimeMS());

    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap =
        GetActiveAffinityDeviceMapByGroupName(activeRendererGroupAffinityMap_, affinityDeviceInfo.groupName);

    affinityDeviceInfoMap[clientUID] = affinityDeviceInfo;
    activeRendererGroupAffinityMap_[affinityDeviceInfo.groupName] = affinityDeviceInfoMap;
}

void AudioAffinityManager::AddSelectCapturerDevice(
    int32_t clientUID, const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::lock_guard<std::mutex> lock(capturerMapMutex_);
    CHECK_AND_RETURN_LOG(desc != nullptr, "AudioDeviceDescriptor sptr not valid");

    activeCapturerDeviceMap_[clientUID] = desc;

    AffinityDeviceInfo affinityDeviceInfo =
        GetAffinityDeviceInfoByDeviceType(capturerAffinityDeviceArray_, desc->getType(), desc->networkId_);

    affinityDeviceInfo.chooseTimeStamp = static_cast<uint64_t>(GetCurrentTimeMS());

    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap =
        GetActiveAffinityDeviceMapByGroupName(activeCapturerGroupAffinityMap_, affinityDeviceInfo.groupName);

    affinityDeviceInfoMap[clientUID] = affinityDeviceInfo;
    activeCapturerGroupAffinityMap_[affinityDeviceInfo.groupName] = affinityDeviceInfoMap;
}

void AudioAffinityManager::DelSelectRendererDevice(int32_t clientUID)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    AFFINITYDEVMAP::iterator item = activeRendererDeviceMap_.find(clientUID);
    if (item != activeRendererDeviceMap_.end()) {
        CHECK_AND_RETURN_LOG(item->second != nullptr, "AudioDeviceDescriptor sptr not valid");
        DelActiveGroupAffinityMap(clientUID, item->second->getType(), item->second->networkId_,
            rendererAffinityDeviceArray_, activeRendererGroupAffinityMap_);
        activeRendererDeviceMap_.erase(item);
    }
}

void AudioAffinityManager::DelSelectCapturerDevice(int32_t clientUID)
{
    std::lock_guard<std::mutex> lock(capturerMapMutex_);
    AFFINITYDEVMAP::iterator item = activeCapturerDeviceMap_.find(clientUID);
    if (item != activeCapturerDeviceMap_.end()) {
        CHECK_AND_RETURN_LOG(item->second != nullptr, "AudioDeviceDescriptor sptr not valid");
        DelActiveGroupAffinityMap(clientUID, item->second->getType(), item->second->networkId_,
            capturerAffinityDeviceArray_, activeCapturerGroupAffinityMap_);
        activeCapturerDeviceMap_.erase(item);
    }
}

void AudioAffinityManager::RemoveOfflineRendererDevice(const AudioDeviceDescriptor &updateDesc)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    AFFINITYDEVMAP::iterator item = activeRendererDeviceMap_.begin();
    while (item != activeRendererDeviceMap_.end()) {
        CHECK_AND_RETURN_LOG(item->second != nullptr, "AudioDeviceDescriptor sptr not valid");
        if (item->second->deviceType_ == updateDesc.deviceType_ &&
            item->second->networkId_ == updateDesc.networkId_) {
            DelActiveGroupAffinityMap(item->first, item->second->getType(), item->second->networkId_,
                rendererAffinityDeviceArray_, activeRendererGroupAffinityMap_);
            item = activeRendererDeviceMap_.erase(item);
        } else {
            ++item;
        }
    }
}

void AudioAffinityManager::RemoveOfflineCapturerDevice(const AudioDeviceDescriptor &updateDesc)
{
    std::lock_guard<std::mutex> lock(capturerMapMutex_);
    AFFINITYDEVMAP::iterator item = activeCapturerDeviceMap_.begin();
    while (item != activeCapturerDeviceMap_.end()) {
        CHECK_AND_RETURN_LOG(item->second != nullptr, "AudioDeviceDescriptor sptr not valid");
        if (item->second->deviceType_ == updateDesc.deviceType_ &&
            item->second->networkId_ == updateDesc.networkId_) {
            DelActiveGroupAffinityMap(item->first, item->second->getType(), item->second->networkId_,
                capturerAffinityDeviceArray_, activeCapturerGroupAffinityMap_);
            item = activeCapturerDeviceMap_.erase(item);
        } else {
            ++item;
        }
    }
}

AffinityDeviceInfo AudioAffinityManager::GetAffinityDeviceInfoByDeviceType(
    const std::vector<AffinityDeviceInfo> &affinityDeviceInfoArray, const DeviceType &deviceType,
    const std::string &networkID)
{
    AffinityDeviceInfo affinityDeviceInfo = {};
    std::vector<AffinityDeviceInfo>::const_iterator item = affinityDeviceInfoArray.begin();
    while (item != rendererAffinityDeviceArray_.end()) {
        if (networkID == item->networkID && deviceType == item->deviceType) {
            affinityDeviceInfo = *item;
            break;
        }
        ++item;
    }
    return affinityDeviceInfo;
}

std::unordered_map<int32_t, AffinityDeviceInfo> AudioAffinityManager::GetActiveAffinityDeviceMapByGroupName(
    const AFFINITYDEVINFOMAP &activeGroupNameMap, const std::string &groupName)
{
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap = {};
    AFFINITYDEVINFOMAP::const_iterator itemGroup = activeGroupNameMap.find(groupName);
    if (itemGroup != activeGroupNameMap.end()) {
        affinityDeviceInfoMap = itemGroup->second;
    }
    return affinityDeviceInfoMap;
}

int32_t AudioAffinityManager::GetAffinityClientUID(const int32_t &clientUID, const DeviceType &deviceType,
    const std::string &networkID, const std::vector<AffinityDeviceInfo> &affinityDeviceInfoArray,
    AFFINITYDEVINFOMAP &activeGroupNameMap)
{
    AffinityDeviceInfo affinityDeviceInfo =
        GetAffinityDeviceInfoByDeviceType(affinityDeviceInfoArray, deviceType, networkID);

    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap =
        GetActiveAffinityDeviceMapByGroupName(activeGroupNameMap, affinityDeviceInfo.groupName);

    return GetAffinityClientUID(clientUID, affinityDeviceInfoMap);
}

int32_t AudioAffinityManager::GetAffinityClientUID(
    const int32_t &clientUID, std::unordered_map<int32_t, AffinityDeviceInfo> &affinityDeviceInfoMap)
{
    std::unordered_map<int32_t, AffinityDeviceInfo>::iterator item = affinityDeviceInfoMap.find(clientUID);
    if (item != affinityDeviceInfoMap.end() && item->second.SupportedConcurrency) {
        return clientUID;
    }
    int32_t affinityClientID = 0;
    uint64_t chooseTimeStamp = 0;
    item = affinityDeviceInfoMap.begin();
    while (item != affinityDeviceInfoMap.end()) {
        if (!item->second.SupportedConcurrency && chooseTimeStamp < item->second.chooseTimeStamp) {
            chooseTimeStamp = item->second.chooseTimeStamp;
            affinityClientID = item->first;
        }
        ++item;
    }
    return affinityClientID;
}

void AudioAffinityManager::DelActiveGroupAffinityMap(const int32_t &clientUID,
    std::unordered_map<int32_t, AffinityDeviceInfo> &affinityDeviceInfoMap)
{
    std::unordered_map<int32_t, AffinityDeviceInfo>::iterator item = affinityDeviceInfoMap.find(clientUID);
    if (item != affinityDeviceInfoMap.end()) {
        affinityDeviceInfoMap.erase(item);
    }
}

void AudioAffinityManager::DelActiveGroupAffinityMap(const int32_t &clientUID, const DeviceType &deviceType,
    const std::string &networkID, std::vector<AffinityDeviceInfo> &affinityDeviceArray,
    AFFINITYDEVINFOMAP &activeGroupNameMap)
{
    AffinityDeviceInfo affinityDeviceInfo =
        GetAffinityDeviceInfoByDeviceType(affinityDeviceArray, deviceType, networkID);
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap =
        GetActiveAffinityDeviceMapByGroupName(activeGroupNameMap, affinityDeviceInfo.groupName);
    DelActiveGroupAffinityMap(clientUID, affinityDeviceInfoMap);
    activeGroupNameMap[affinityDeviceInfo.groupName] = affinityDeviceInfoMap;
}
} // namespace AudioStandard
} // namespace OHOS

