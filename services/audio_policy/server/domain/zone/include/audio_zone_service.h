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

#ifndef ST_AUDIO_ZONE_SERVICE_H
#define ST_AUDIO_ZONE_SERVICE_H

#include <unordered_map>
#include <memory>
#include "audio_interrupt_service.h"
#include "istandard_audio_zone_client.h"
#include "audio_policy_server_handler.h"
#include "audio_zone.h"

namespace OHOS {
namespace AudioStandard {
class AudioZone;
class AudioZoneClientManager;

class AudioZoneService {
public:
    static AudioZoneService &GetInstance();

    void Init(std::shared_ptr<AudioPolicyServerHandler> handler,
        std::shared_ptr<AudioInterruptService> interruptService);
    void DeInit();
    
    int32_t CreateAudioZone(const std::string &name, const AudioZoneContext &context, pid_t clientPid);
    void ReleaseAudioZone(int32_t zoneId);
    const std::vector<std::shared_ptr<AudioZoneDescriptor>> GetAllAudioZone();
    const std::shared_ptr<AudioZoneDescriptor> GetAudioZone(int32_t zoneId);
    int32_t GetAudioZoneByName(std::string name);

    int32_t BindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices);
    int32_t UnBindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices);
    void MoveDeviceToGlobalFromZones(std::shared_ptr<AudioDeviceDescriptor> device);
    int32_t UpdateDeviceFromGlobalForAllZone(std::shared_ptr<AudioDeviceDescriptor> device);

    int32_t RegisterAudioZoneClient(pid_t clientPid, sptr<IStandardAudioZoneClient> client);
    void UnRegisterAudioZoneClient(pid_t clientPid);
    int32_t EnableAudioZoneReport(pid_t clientPid, bool enable);
    int32_t EnableAudioZoneChangeReport(pid_t clientPid, int32_t zoneId, bool enable);

    int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid);
    int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid);
    int32_t AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream);
    int32_t AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams);
    int32_t RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream);
    int32_t RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams);
    void SetZoneDeviceVisible(bool visible);
    bool IsZoneDeviceVisible();
    int32_t FindAudioZoneByUid(int32_t uid);
    int32_t FindAudioZone(int32_t uid, StreamUsage usage);

    int32_t EnableSystemVolumeProxy(pid_t clientPid, int32_t zoneId, bool enable);
    bool IsSystemVolumeProxyEnable(int32_t zoneId);
    int32_t SetSystemVolumeLevel(int32_t zoneId, AudioVolumeType volumeType, int32_t volumeLevel, int32_t volumeFlag);
    int32_t GetSystemVolumeLevel(int32_t zoneId, AudioVolumeType volumeType);
    int32_t SetSystemVolumeDegree(int32_t zoneId, AudioVolumeType volumeType,
        int32_t volumeDegree, int32_t volumeFlag);
    int32_t GetSystemVolumeDegree(int32_t zoneId, AudioVolumeType volumeType);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(int32_t zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(int32_t zoneId,
        const std::string &deviceTag);
    int32_t EnableAudioZoneInterruptReport(pid_t clientPid, int32_t zoneId,
        const std::string &deviceTag, bool enable);

    int32_t ActivateAudioInterrupt(int32_t zoneId, const AudioInterrupt &audioInterrupt,
        bool isUpdatedAudioStrategy = false);
    int32_t DeactivateAudioInterrupt(int32_t zoneId, const AudioInterrupt &audioInterrupt);
    int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts);
    int32_t InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts);
    
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> FetchOutputDevices(int32_t zoneId,
        StreamUsage streamUsage, int32_t clientUid, const RouterType &bypassType);
    std::shared_ptr<AudioDeviceDescriptor> FetchInputDevice(int32_t zoneId,
       SourceType sourceType, int32_t clientUid);
    
    const std::string GetZoneStringDescriptor(int32_t zoneId);
    int32_t ClearAudioFocusBySessionID(const int32_t &sessionID);
    bool CheckZoneExist(int32_t zoneId);
    int32_t FindAudioSessionZoneid(int32_t callerUid, int32_t callerPid, bool isActivate);

    void ReleaseAudioZoneByClientPid(pid_t clientPid);
    bool CheckDeviceInAudioZone(AudioDeviceDescriptor device);
    bool CheckExistUidInAudioZone();

private:
    AudioZoneService() = default;
    ~AudioZoneService() = default;

    std::shared_ptr<AudioInterruptService> interruptService_;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager_;
    std::unordered_map<int32_t, std::shared_ptr<AudioZone>> zoneMaps_;
    std::set<pid_t> zoneReportClientList_;
    std::mutex zoneMutex_;
    bool zoneDeviceVisible_ = true;

    std::shared_ptr<AudioZone> FindZone(int32_t zoneId);
    int32_t AddKeyToAudioZone(int32_t zoneId, int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage);
    int32_t RemoveKeyFromAudioZone(int32_t zoneId, int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage);
    int32_t FindAudioZoneByKey(int32_t uid, const std::string &deviceTag, const std::string &streamTag,
        const StreamUsage &usage);
    bool CheckIsZoneValid(int32_t zoneId);
    void RemoveDeviceFromGlobal(std::shared_ptr<AudioDeviceDescriptor> device);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_ZONE_SERVICE_H