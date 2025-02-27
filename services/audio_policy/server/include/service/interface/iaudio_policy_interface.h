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

#ifndef I_AUDIO_POLICY_INTERFACE_H
#define I_AUDIO_POLICY_INTERFACE_H

#include "audio_adapter_info.h"
#include "audio_policy_manager.h"
#include "audio_policy_ipc_interface_code.h"
#include "audio_stream_removed_callback.h"
#include "audio_volume_config.h"
#include "volume_data_maintainer.h"
#include "audio_manager_base.h"

#include <memory>
#include <string>

namespace OHOS {
namespace AudioStandard {
class IAudioPolicyInterface {
public:
    virtual ~IAudioPolicyInterface() {}

    virtual bool Init() = 0;

    virtual void Deinit(void) = 0;

    virtual void InitKVStore() = 0;

    virtual bool ConnectServiceAdapter() = 0;

    virtual int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) = 0;

    virtual int32_t GetMinVolumeLevel(AudioVolumeType volumeType) = 0;

    virtual int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel) = 0;

    virtual int32_t SetSystemVolumeLevelWithDevice(AudioStreamType streamType, int32_t volumeLevel,
        DeviceType deviceType) = 0;
    virtual int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel) = 0;

    virtual int32_t SetAppVolumeMuted(int32_t appUid, bool muted) = 0;

    virtual bool IsAppVolumeMute(int32_t appUid, bool owned) = 0;

    virtual int32_t GetSystemVolumeLevel(AudioStreamType streamType) = 0;

    virtual int32_t GetAppVolumeLevel(int32_t appUid) = 0;

    virtual int32_t GetSystemVolumeLevelNoMuteState(AudioStreamType streamType) = 0;

    virtual float GetSystemVolumeDb(AudioStreamType streamType) = 0;

    virtual int32_t SetStreamMute(AudioStreamType streamType, bool mute,
        StreamUsage streamUsage = STREAM_USAGE_UNKNOWN, const DeviceType &deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute) = 0;

    virtual bool GetStreamMute(AudioStreamType streamType) = 0;

    virtual std::vector<SinkInfo> GetAllSinks() = 0;

    virtual void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs) = 0;

    virtual std::vector<SourceOutput> GetAllSourceOutputs() = 0;

    virtual AudioIOHandle OpenAudioPort(const AudioModuleInfo &audioPortInfo) = 0;

    virtual int32_t CloseAudioPort(AudioIOHandle ioHandle, bool isSync = false) = 0;

    virtual int32_t SelectDevice(DeviceRole deviceRole, InternalDeviceType deviceType, std::string name) = 0;

    virtual int32_t SetDeviceActive(InternalDeviceType deviceType,
                                    std::string name, bool active, DeviceFlag flag = ALL_DEVICES_FLAG) = 0;

    virtual int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) = 0;

    virtual int32_t MoveSourceOutputByIndexOrName(uint32_t sourceOutputId,
        uint32_t sourceIndex, std::string sourceName) = 0;

    virtual int32_t SetRingerMode(AudioRingerMode ringerMode) = 0;

    virtual AudioRingerMode GetRingerMode() const = 0;

    virtual int32_t SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback) = 0;

    virtual int32_t SuspendAudioDevice(std::string &name, bool isSuspend) = 0;

    virtual void SetVolumeForSwitchDevice(InternalDeviceType deviceType) = 0;

    virtual bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) = 0;

    virtual float CalculateVolumeDb(int32_t volumeLevel) = 0;

    virtual int32_t SetSystemSoundUri(const std::string &key, const std::string &uri) = 0;

    virtual std::string GetSystemSoundUri(const std::string &key) = 0;

    virtual float GetMinStreamVolume() const = 0;

    virtual float GetMaxStreamVolume() const = 0;

    virtual bool IsVolumeUnadjustable() = 0;

    virtual void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos) = 0;

    virtual void SetAbsVolumeScene(bool isAbsVolumeScene) = 0;

    virtual bool IsAbsVolumeScene() const = 0;

    virtual void SetAbsVolumeMute(bool mute) = 0;

    virtual void SetDataShareReady(std::atomic<bool> isDataShareReady) = 0;

    virtual bool IsAbsVolumeMute() const = 0;

    virtual float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) = 0;

    virtual std::string GetModuleArgs(const AudioModuleInfo &audioModuleInfo) const = 0;

    virtual void ResetRemoteCastDeviceVolume() = 0;

    virtual int32_t DoRestoreData() = 0;

    virtual SafeStatus GetCurrentDeviceSafeStatus(DeviceType deviceType) = 0;

    virtual int64_t GetCurentDeviceSafeTime(DeviceType deviceType) = 0;

    virtual int32_t SetDeviceSafeStatus(DeviceType deviceType, SafeStatus status) = 0;

    virtual int32_t SetDeviceSafeTime(DeviceType deviceType, int64_t time) = 0;

    virtual int32_t SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume) = 0;

    virtual int32_t GetRestoreVolumeLevel(DeviceType deviceType) = 0;

    virtual int32_t GetSafeVolumeLevel() const = 0;

    virtual int32_t GetSafeVolumeTimeout() const = 0;

    virtual void SafeVolumeDump(std::string &dumpString) = 0;

    virtual void SetActiveDevice(DeviceType deviceType) = 0;

    virtual DeviceType GetActiveDevice() = 0;

    virtual void NotifyAccountsChanged(const int &id) = 0;

    virtual int32_t GetCurActivateCount() const = 0;

    virtual void HandleKvData(bool isFirstBoot) = 0;

    virtual int32_t SetPersistMicMuteState(const bool isMute) = 0;

    virtual int32_t GetPersistMicMuteState(bool &isMute) const = 0;

    virtual void HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel) = 0;

    virtual void HandleStreamMuteStatus(AudioStreamType streamType, bool mute,
        StreamUsage streamUsage = STREAM_USAGE_UNKNOWN,
        const DeviceType &deviceType = DEVICE_TYPE_NONE) = 0;

    virtual void HandleRingerMode(AudioRingerMode ringerMode) = 0;

    virtual void SetAudioServerProxy(sptr<IStandardAudioService> gsp) = 0;

    virtual void SetOffloadSessionId(uint32_t sessionId) = 0;

    virtual void ResetOffloadSessionId() = 0;

    virtual int32_t SetDoubleRingVolumeDb(const AudioStreamType &streamType, const int32_t &volumeLevel) = 0;

    virtual void SetDeviceSafeVolume(const AudioStreamType streamType, const int32_t volumeLevel) = 0;

    virtual void SetRestoreVolumeFlag(const bool safeVolumeCall) = 0;

    virtual void UpdateSafeVolumeByS4() = 0;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_POLICY_INTERFACE_H
