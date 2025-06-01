/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_AFFINITY_MANAGER_H
#define ST_AUDIO_AFFINITY_MANAGER_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "audio_device_info.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

using AFFINITYDEVINFOMAP = std::unordered_map<std::string, std::unordered_map<int32_t, AffinityDeviceInfo>>;
using AFFINITYDEVMAP = std::unordered_map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>;

class AudioAffinityManager {
public:
    static AudioAffinityManager& GetAudioAffinityManager()
    {
        static AudioAffinityManager audioAffinityManager;
        return audioAffinityManager;
    }

    void ParseAffinityXml();

    void OnXmlParsingCompleted(std::vector<AffinityDeviceInfo> &xmlData);

    std::shared_ptr<AudioDeviceDescriptor> GetRendererDevice(int32_t clientUID);
    std::shared_ptr<AudioDeviceDescriptor> GetCapturerDevice(int32_t clientUID);

    void AddSelectRendererDevice(int32_t clientUID, const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    void RemoveOfflineRendererDevice(const AudioDeviceDescriptor &updatedDesc);
    void DelSelectRendererDevice(int32_t clientUID);

    void AddSelectCapturerDevice(int32_t clientUID, const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    void RemoveOfflineCapturerDevice(const AudioDeviceDescriptor &updatedDesc);
    void DelSelectCapturerDevice(int32_t clientUID);

private:
    AudioAffinityManager(){};
    ~AudioAffinityManager(){};

    AffinityDeviceInfo GetAffinityDeviceInfoByDeviceType(
        const std::vector<AffinityDeviceInfo> &affinityDeviceInfoArray,
        const DeviceType &deviceType, const std::string &networkID);
    std::unordered_map<int32_t, AffinityDeviceInfo> GetActiveAffinityDeviceMapByGroupName(
        const AFFINITYDEVINFOMAP &activeGroupNameMap, const std::string &groupName);
    int32_t GetAffinityClientUID(const int32_t &clientUID,
        std::unordered_map<int32_t, AffinityDeviceInfo> &affinityDeviceInfoMap);
    int32_t GetAffinityClientUID(const int32_t &clientUID, const DeviceType &deviceType, const std::string &networkID,
        const std::vector<AffinityDeviceInfo> &affinityDeviceInfoArray,
        AFFINITYDEVINFOMAP &activeGroupNameMap);

    void DelActiveGroupAffinityMap(const int32_t &clientUID,
        std::unordered_map<int32_t, AffinityDeviceInfo> &affinityDeviceInfoMap);
    void DelActiveGroupAffinityMap(const int32_t &clientUID, const DeviceType &deviceType,
        const std::string &networkID, std::vector<AffinityDeviceInfo> &affinityDeviceArray,
        AFFINITYDEVINFOMAP &activeGroupNameMap);

    AFFINITYDEVMAP activeRendererDeviceMap_;
    AFFINITYDEVMAP activeCapturerDeviceMap_;

    std::vector<AffinityDeviceInfo> rendererAffinityDeviceArray_;
    std::vector<AffinityDeviceInfo> capturerAffinityDeviceArray_;

    AFFINITYDEVINFOMAP activeRendererGroupAffinityMap_;
    AFFINITYDEVINFOMAP activeCapturerGroupAffinityMap_;

    std::mutex rendererMapMutex_;
    std::mutex capturerMapMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_AFFINITY_MANAGER_H
