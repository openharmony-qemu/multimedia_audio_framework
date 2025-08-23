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

#ifndef ST_AUDIO_ZONE_H
#define ST_AUDIO_ZONE_H

#include <mutex>
#include <vector>
#include <set>
#include <list>
#include <utility>
#include <string>
#include "audio_zone_info.h"
#include "audio_device_descriptor.h"
#include "audio_zone_client_manager.h"
#include "audio_connected_device.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneBindKey {
public:
    explicit AudioZoneBindKey(int32_t uid);
    AudioZoneBindKey(int32_t uid, const std::string &deviceTag);
    AudioZoneBindKey(int32_t uid, const std::string &deviceTag, const std::string &streamTag);
    AudioZoneBindKey(int32_t uid, const std::string &deviceTag, const std::string &streamTag,
        const StreamUsage &usage);
    AudioZoneBindKey(const AudioZoneBindKey &other);
    AudioZoneBindKey(AudioZoneBindKey &&other);
    AudioZoneBindKey &operator=(const AudioZoneBindKey &other);
    AudioZoneBindKey &operator=(AudioZoneBindKey &&other);

    bool operator==(const AudioZoneBindKey &other) const;
    bool operator!=(const AudioZoneBindKey &other) const;

    int32_t GetUid() const;
    const std::string GetString() const;
    bool IsContain(const AudioZoneBindKey &other) const;
    const static std::vector<AudioZoneBindKey> GetSupportKeys(int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage);
    const static std::vector<AudioZoneBindKey> GetSupportKeys(const AudioZoneBindKey &key);

private:
    int32_t uid_ = INVALID_UID;
    std::string deviceTag_ = "";
    std::string streamTag_ = "";
    StreamUsage usage_ = StreamUsage::STREAM_USAGE_INVALID;

    void Assign(const AudioZoneBindKey &other);
    void Swap(AudioZoneBindKey &&other);
};

class AudioZone {
public:
    AudioZone(std::shared_ptr<AudioZoneClientManager> manager, const std::string &name,
        const AudioZoneContext &context, pid_t clientPid = 0);
    ~AudioZone() = default;

    int32_t GetId();
    const std::shared_ptr<AudioZoneDescriptor> GetDescriptor();
    const std::string GetStringDescriptor();
    const std::string GetName();

    void BindByKey(const AudioZoneBindKey &key);
    void RemoveKey(const AudioZoneBindKey &key);
    bool IsContainKey(const AudioZoneBindKey &key);
    
    int32_t AddDeviceDescriptor(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices);
    int32_t RemoveDeviceDescriptor(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices);
    int32_t EnableDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> device);
    int32_t DisableDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> device);
    bool IsDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> device);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> FetchOutputDevices(StreamUsage streamUsage,
        int32_t clientUid, const RouterType &bypassType);
    std::shared_ptr<AudioDeviceDescriptor> FetchInputDevice(SourceType sourceType, int32_t clientUid);

    int32_t EnableSystemVolumeProxy(pid_t clientPid, bool enable);
    int32_t SetSystemVolumeLevel(const AudioVolumeType volumeType,
        const int32_t volumeLevel, const int32_t volumeFlag = 0);
    int32_t GetSystemVolumeLevel(AudioVolumeType volumeType);
    bool IsVolumeProxyEnable();

    int32_t EnableChangeReport(pid_t clientPid, bool enable);

    int32_t UpdateDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> device);

    pid_t GetClientPid();

private:
    int32_t zoneId_ = -1;
    std::string name_ = "";
    std::list<AudioZoneBindKey> keys_;
    std::list<std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool>> devices_;
    std::mutex zoneMutex_;
    std::shared_ptr<AudioZoneClientManager> clientManager_;
    std::set<pid_t> changeReportClientList_;
    pid_t volumeProxyClientPid_ = 0;
    pid_t zoneClientPid_ = 0;
    bool isVolumeProxyEnabled_ = false;

    int32_t SetDeviceDescriptorState(const std::shared_ptr<AudioDeviceDescriptor> device, const bool enable);
    void SendZoneChangeEvent(AudioZoneChangeReason reason);
    const std::shared_ptr<AudioZoneDescriptor> GetDescriptorNoLock();
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_ZONE_H