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
#include "audio_volume_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioZone;
class AudioZoneClientManager;
class SessionTimeOutCallback;

class AudioZoneService : public SessionTimeOutCallback {
public:
    static AudioZoneService &GetInstance();

    void Init(std::shared_ptr<AudioPolicyServerHandler> handler,
        std::shared_ptr<AudioInterruptService> interruptService);
    void DeInit();
    
    int32_t CreateAudioZone(const std::string &name, const AudioZoneContext &context, pid_t clientPid);
    void ReleaseAudioZone(int32_t zoneId);
    void UpdateContextForAudioZone(int32_t zoneId, const AudioZoneContext &context);
    int32_t GetAudioZoneForApp(const std::vector<int32_t> &userIds, std::vector<int32_t> &retUserIds);
    const std::vector<std::shared_ptr<AudioZoneDescriptor>> GetAllAudioZone();
    const std::shared_ptr<AudioZoneDescriptor> GetAudioZone(int32_t zoneId);
    int32_t GetAudioZoneByName(std::string name);
    std::string GetZoneNameById(int32_t zoneId);

    int32_t BindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices);
    int32_t UnBindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices);
    void MoveDeviceToGlobalFromZones(std::shared_ptr<AudioDeviceDescriptor> device);
    int32_t UpdateDeviceFromGlobalForAllZone(std::shared_ptr<AudioDeviceDescriptor> device);
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceDescriptor(DeviceType type, std::string networkId);
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceDescriptor(int32_t zoneId, DeviceType type);

    int32_t RegisterAudioZoneClient(pid_t clientPid, sptr<IStandardAudioZoneClient> client);
    void UnRegisterAudioZoneClient(pid_t clientPid);
    int32_t EnableAudioZoneReport(pid_t clientPid, bool enable);
    int32_t EnableAudioZoneChangeReport(pid_t clientPid, int32_t zoneId, bool enable);

    int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid);
    int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid);
    int32_t AddUidUsagesToAudioZone(int32_t zoneId, int32_t uid, const std::set<StreamUsage> &usages);
    int32_t RemoveUidUsagesFromAudioZone(int32_t zoneId, int32_t uid, const std::set<StreamUsage> &usages);
    int32_t AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream);
    int32_t AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams);
    int32_t RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream);
    int32_t RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams);
    int32_t AddStreamIdToAudioZone(int32_t zoneId, uint32_t streamId);
    int32_t RemoveStreamIdFromAudioZone(int32_t zoneId, uint32_t streamId);
    void SetZoneDeviceVisible(bool visible);
    bool IsZoneDeviceVisible();
    int32_t FindAudioZoneByUid(int32_t uid, DeviceRole deviceRole = DEVICE_ROLE_NONE);
    int32_t FindAudioZone(int32_t uid, StreamUsage usage, DeviceRole deviceRole = DEVICE_ROLE_NONE,
        uint32_t streamId = INVALID_STREAM_ID);
    virtual std::string FindAudioZoneNameByUid(int32_t uid, DeviceRole deviceRole = DEVICE_ROLE_NONE,
        uint32_t streamId = INVALID_STREAM_ID);

    int32_t EnableSystemVolumeProxy(pid_t clientPid, int32_t zoneId, DeviceType deviceType, bool enable);
    bool IsSystemVolumeProxyEnable(int32_t zoneId, DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE);
    int32_t SetSystemVolumeLevel(int32_t zoneId, DeviceType deviceType, AudioVolumeType volumeType,
        const VolumeScale &volume, int32_t volumeFlag = 0);
    int32_t GetSystemVolumeLevel(int32_t zoneId, DeviceType deviceType, AudioVolumeType volumeType);
    int32_t GetSystemVolumeDegree(int32_t zoneId, DeviceType deviceType, AudioVolumeType volumeType);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(int32_t zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(int32_t zoneId,
        const std::string &deviceTag);
    int32_t EnableAudioZoneInterruptReport(pid_t clientPid, int32_t zoneId,
        const std::string &deviceTag, bool enable);

    AudioInterruptResult ActivateAudioInterrupt(int32_t zoneId, const AudioInterrupt &audioInterrupt,
        bool isUpdatedAudioStrategy = false);
    AudioInterruptResult DeactivateAudioInterrupt(int32_t zoneId, const AudioInterrupt &audioInterrupt,
        bool isRemoveFocusHis = false);
    int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts);
    int32_t InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts);
    
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> FetchOutputDevices(int32_t zoneId,
        StreamUsage streamUsage, int32_t clientUid, RouterType bypassType);
    std::shared_ptr<AudioDeviceDescriptor> FetchInputDevice(int32_t zoneId,
       SourceType sourceType, int32_t clientUid);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAllOutputDevices(int32_t zoneId);

    const std::string GetZoneStringDescriptor(int32_t zoneId);
    int32_t ClearAudioFocusBySessionID(const int32_t &sessionID);
    bool CheckZoneExist(int32_t zoneId);

    void ReleaseAudioZoneByClientPid(pid_t clientPid);
    bool CheckDeviceInAudioZone(AudioDeviceDescriptor device);
    bool HasVolumeControllableDeviceInAudioZone();
    void NotifyStreamSilentChange(uint32_t streamId);
    int32_t GetActiveAudioInterruptZone(int32_t &zoneId, AudioStreamType &streamType);
    AudioScene GetAudioSceneFromAllZones();
    AudioInterruptResult ActivateAudioSession(const int32_t zoneId, const int32_t callerPid,
        const AudioSessionStrategy &strategy, const bool isStandalone = false,
        const bool stateChangeCallbackFlag = false, int32_t callerUid = INVALID_UID);
    int32_t DeactivateAudioSession(const int32_t zoneId, const int32_t callerPid);
    // interfaces of SessionTimeOutCallback
    void OnSessionTimeout(const int32_t pid) override;

private:
    AudioZoneService()
        : audioVolumeManager_(AudioVolumeManager::GetInstance()),
          audioConnectedDevice_(AudioConnectedDevice::GetInstance()) {}

    ~AudioZoneService() = default;

    AudioVolumeManager &audioVolumeManager_;
    AudioConnectedDevice &audioConnectedDevice_;
    std::shared_ptr<AudioInterruptService> interruptService_;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager_;
    std::shared_ptr<SessionTimeOutCallback> sessionTimeoutCallbackPtr_;
    std::unordered_map<int32_t, std::shared_ptr<AudioZone>> zoneMaps_;
    AudioZoneContext mainZoneContext_;
    std::set<pid_t> zoneReportClientList_;
    std::mutex zoneMutex_;
    bool zoneDeviceVisible_ = true;
    pid_t mainZoneVolumeProxyClientPid_ = 0;
    std::unordered_set<DeviceType> mainZoneVolumeProxyDeviceTypes_;

    std::shared_ptr<AudioZone> FindZone(int32_t zoneId);
    int32_t AddKeyToAudioZone(int32_t zoneId, int32_t uid, const std::string &deviceTag,
        uint32_t streamId, const StreamUsage &usage);
    int32_t RemoveKeysFromAudioZone(int32_t zoneId, const std::vector<AudioZoneBindKey> &bindKeys);
    int32_t FindAudioZoneByKey(int32_t uid, const std::string &deviceTag, uint32_t streamId,
        const StreamUsage &usage, DeviceRole deviceRole);
    bool CheckIsZoneValid(int32_t zoneId);
    void RemoveDeviceFromGlobal(std::shared_ptr<AudioDeviceDescriptor> device);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_ZONE_SERVICE_H