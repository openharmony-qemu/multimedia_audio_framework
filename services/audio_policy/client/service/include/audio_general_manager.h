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

#ifndef ST_AUDIO_GENERAL_MANAGER_H
#define ST_AUDIO_GENERAL_MANAGER_H

#include <mutex>
#include "audio_policy_interface.h"
#include "audio_manager_base.h"
#include "audio_system_manager.h"
#include "istandard_audio_service.h"

namespace OHOS {
namespace AudioStandard {
class AudioGeneralManager {
public:
    static AudioGeneralManager *GetInstance();

    virtual ~AudioGeneralManager();

    AudioGeneralManager();

    int32_t GetCallingPid();

    const sptr<IStandardAudioService> GetAudioGeneralManagerProxy();

    int32_t SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback);

    int32_t GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);

    int32_t UnsetAudioDeviceRefinerCallback();

    int32_t SetDeviceVolumeBehavior(const std::string &networkId, DeviceType deviceType, VolumeBehavior volumeBehavior);

    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason);

    int32_t SetPreferredDevice(const PreferredType preferredType,
        const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid = INVALID_UID);

    int32_t SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback);

    int32_t RegisterFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

    int32_t SetDeviceChangeCallback(const DeviceFlag flag,
        const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback);

    int32_t SetDeviceInfoUpdateCallback(const std::shared_ptr<AudioManagerDeviceInfoUpdateCallback>& callback);

    int32_t SetQueryClientTypeCallback(
        const std::shared_ptr<AudioQueryClientTypeCallback>& callback);

    int32_t SetQueryDeviceVolumeBehaviorCallback(
        const std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallback> &callback);

    int32_t SetExtraParameters(const std::string &key,
        const std::vector<std::pair<std::string, std::string>> &kvpairs);

    int32_t GetVolume(AudioVolumeType volumeType) const;

    int32_t RegisterVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v = API_9);

    DeviceType GetActiveOutputDevice();

    AudioScene GetAudioScene() const;

    int32_t SetAudioSceneChangeCallback(const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &callback);

    int32_t GetMaxVolume(AudioVolumeType volumeType);

    int32_t UnregisterFocusInfoChangeCallback(
        const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    int32_t GetAudioFocusInfoList(
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);

    int32_t SelectOutputDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    int32_t UnregisterVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback = nullptr);

    int32_t RegisterAudioCapturerEventListener(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerStateChangeCallback> &callback);
    
    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);

    int32_t RegisterAudioRendererEventListener(const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    int32_t GetPreferredInputDeviceForCapturerInfo(
        AudioCapturerInfo captureInfo, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);

    int32_t SetPreferredInputDeviceChangeCallback(
        AudioCapturerInfo &capturerInfo, const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback);

    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    int32_t SetAudioClientInfoMgrCallback(const std::shared_ptr<AudioClientInfoMgrCallback> &callback);

    int32_t SetDeviceConnectionStatus(std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, bool isConnected);
    int32_t UpdateDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, DeviceInfoUpdateCommand command);
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;
    int32_t SelectPrivateDevice(int32_t devType, const std::string &macAddress);
    int32_t ForceSelectDevice(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter);
    int32_t SetSleAudioOperationCallback(const std::shared_ptr<SleAudioOperationCallback> &callback);
    int32_t RestoreDistributedDeviceInfo();
private:
    std::shared_ptr<AudioFocusInfoChangeCallback> audioFocusInfoCallback_ = nullptr;
    int32_t volumeChangeClientPid_ = -1;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_GENERAL_MANAGER_H
