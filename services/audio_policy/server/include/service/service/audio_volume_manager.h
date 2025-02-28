/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_VOLUME_MANAGER_H
#define ST_AUDIO_VOLUME_MANAGER_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_shared_memory.h"
#include "audio_system_manager.h"
#include "audio_ec_info.h"
#include "datashare_helper.h"
#include "audio_errors.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"
#include "common_event_manager.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

#include "audio_a2dp_device.h"
#include "audio_active_device.h"
#include "audio_scene_manager.h"
#include "audio_connected_device.h"
#include "audio_offload_stream.h"

namespace OHOS {
namespace AudioStandard {

using InternalDeviceType = DeviceType;

class AudioVolumeManager {
public:
    static AudioVolumeManager& GetInstance()
    {
        static AudioVolumeManager instance;
        return instance;
    }
    bool Init(std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler);
    void DeInit(void);
    void InitKVStore();
    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) const;
    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) const;
    bool GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol);
    bool SetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume vol);
    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer);
    void SetSharedAbsVolumeScene(const bool support);
    int32_t GetSystemVolumeLevel(AudioStreamType streamType);
    int32_t GetAppVolumeLevel(int32_t appUid);
    int32_t GetSystemVolumeLevelNoMuteState(AudioStreamType streamType);
    int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel);
    int32_t SetSystemVolumeLevelWithDevice(AudioStreamType streamType, int32_t volumeLevel, DeviceType deviceType);
    int32_t SetAppVolumeMuted(int32_t appUid, bool muted);
    bool IsAppVolumeMute(int32_t appUid, bool owned);
    int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel);
    int32_t DisableSafeMediaVolume();
    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);
    int32_t SetStreamMute(AudioStreamType streamType, bool mute,
        const StreamUsage &streamUsage = STREAM_USAGE_UNKNOWN,
        const DeviceType &deviceType = DEVICE_TYPE_NONE);
    bool GetStreamMute(AudioStreamType streamType) const;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, bool internalCall = false);

    void UpdateGroupInfo(GroupType type, std::string groupName, int32_t& groupId, std::string networkId,
        bool connected, int32_t mappingId);
    void GetVolumeGroupInfo(std::vector<sptr<VolumeGroupInfo>>& volumeGroupInfos);
    void SetVolumeForSwitchDevice(DeviceType deviceType, const std::string &newSinkName = PORT_NONE);

    bool IsRingerModeMute();
    void SetRingerModeMute(bool flag);
    int32_t ResetRingerModeMute();
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData);
    int32_t SetVoiceRingtoneMute(bool isMute);
    void SetVoiceCallVolume(int32_t volume);
    bool GetVolumeGroupInfosNotWait(std::vector<sptr<VolumeGroupInfo>> &infos);
    void SetDefaultDeviceLoadFlag(bool isLoad);
    void NotifyVolumeGroup();
    bool GetLoadFlag();
    void UpdateSafeVolumeByS4();
private:
    AudioVolumeManager() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()) {}
    ~AudioVolumeManager() {}

    int32_t HandleAbsBluetoothVolume(const std::string &macAddress, const int32_t volumeLevel);
    int32_t DealWithSafeVolume(const int32_t volumeLevel, bool isA2dpDevice);
    void CreateCheckMusicActiveThread();
    bool IsBlueTooth(const DeviceType &deviceType);
    int32_t CheckActiveMusicTime();
    void CheckBlueToothActiveMusicTime(int32_t safeVolume);
    void CheckWiredActiveMusicTime(int32_t safeVolume);
    void RestoreSafeVolume(AudioStreamType streamType, int32_t safeVolume);
    void SetSafeVolumeCallback(AudioStreamType streamType);
    void SetDeviceSafeVolumeStatus();
    void SetAbsVolumeSceneAsync(const std::string &macAddress, const bool support);
    int32_t SelectDealSafeVolume(AudioStreamType streamType, int32_t volumeLevel);
    void PublishSafeVolumeNotification(int32_t notificationId);
    void CancelSafeVolumeNotification(int32_t notificationId);
    void UpdateVolumeForLowLatency();
    bool IsWiredHeadSet(const DeviceType &deviceType);
    void CheckToCloseNotification(AudioStreamType streamType, int32_t volumeLevel);
    bool DeviceIsSupportSafeVolume();
    int32_t DealWithEventVolume(const int32_t notificationId);
    void ChangeDeviceSafeStatus(SafeStatus safeStatus);
    bool CheckMixActiveMusicTime(int32_t safeVolume);
private:
    std::shared_ptr<AudioSharedMemory> policyVolumeMap_ = nullptr;
    volatile Volume *volumeVector_ = nullptr;
    volatile bool *sharedAbsVolumeScene_ = nullptr;

    int64_t activeSafeTimeBt_ = 0;
    int64_t activeSafeTime_ = 0;
    std::time_t startSafeTimeBt_ = 0;
    std::time_t startSafeTime_ = 0;

    std::mutex dialogMutex_;
    std::atomic<bool> isDialogSelectDestroy_ = false;
    std::condition_variable dialogSelectCondition_;

    std::unique_ptr<std::thread> calculateLoopSafeTime_ = nullptr;
    std::mutex checkMusicActiveThreadMutex_; // lock calculateLoopSafeTime_

    std::unique_ptr<std::thread> safeVolumeDialogThrd_ = nullptr;
    std::atomic<bool> isSafeVolumeDialogShowing_ = false;
    std::mutex safeVolumeMutex_;

    bool userSelect_ = false;
    bool safeVolumeExit_ = false;
    SafeStatus safeStatusBt_ = SAFE_UNKNOWN;
    SafeStatus safeStatus_ = SAFE_UNKNOWN;

    bool isBtFirstBoot_ = true;

    std::vector<sptr<VolumeGroupInfo>> volumeGroups_;
    std::vector<sptr<InterruptGroupInfo>> interruptGroups_;

    std::mutex ringerModeMuteMutex_;
    std::atomic<bool> ringerModeMute_ = true;
    std::condition_variable ringerModeMuteCondition_;
    bool isVoiceRingtoneMute_ = false;

    std::mutex notifyMutex_;
    int32_t streamMusicVol_ = 0;
    bool restoreNIsShowing_ = false;
    bool increaseNIsShowing_ = false;

    std::mutex defaultDeviceLoadMutex_;
    std::atomic<bool> isPrimaryMicModuleInfoLoaded_ = false;

    IAudioPolicyInterface& audioPolicyManager_;
    AudioA2dpDevice& audioA2dpDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioActiveDevice& audioActiveDevice_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioOffloadStream& audioOffloadStream_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
};

}
}

#endif