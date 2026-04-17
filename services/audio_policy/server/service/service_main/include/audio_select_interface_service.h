/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef AUDIO_SELECT_INTERFACE_SERVICE_H
#define AUDIO_SELECT_INTERFACE_SERVICE_H

#include "audio_a2dp_offload_manager.h"
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_types.h"
#include "istandard_audio_policy_manager_listener.h"
#include "audio_router_select_strategy.h"

namespace OHOS {
namespace AudioStandard {
class AudioSelectInterfaceService {
public:
    static AudioSelectInterfaceService &GetInstance()
    {
        static AudioSelectInterfaceService instance;
        return instance;
    };

    // AudioRoutingManager#setCommunicationDevice & AudioManager#setDeviceActive (deprecated)
    int32_t SetDeviceActive(DeviceType deviceType, bool active, const std::string &address = "",
        const int32_t uid = INVALID_UID);
    // AudioRoutingManager#selectOutputDevice & AudioRoutingManager#selectOutputDeviceByFilter
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc,
        const int32_t audioDeviceSelectMode = 0, const bool isNeedNotifyBt = true);
    int32_t SetMediaOutputDeviceByUid(DeviceType deviceType, const int32_t uid);

    // AudioRoutingManager#selectInputDeviceByFilter
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc);
    // AudioRoutingManager#selectInputDevice & AudioSessionManager#selectMediaInputDevice
    int32_t SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
        int32_t uid);

    // AudioRoutingManager#excludeOutputDevices & AudioRoutingManager#unexcludeOutputDevices
    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);

    // AudioSessionManager#setDefaultOutputDevice
    int32_t SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType);
    // AudioRenderer#setDefaultOutputDevice
    int32_t SetDefaultOutputDevice(
        const DeviceType deviceType, const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning,
        bool skipForce = false);

    // AudioCapturer#setInputDeviceToAccessory
    int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionID, int32_t uid);
    // AudioCapturer#AudioCapturerOptions#preferredInputDevice
    void SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc);

    // Called by Bluetooth and Nearlink Service
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr, const std::string &caller = "");
    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
        const std::string &caller = "");

    int32_t SetA2dpDeviceOffloadManager(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager);
    int32_t SetAudioClientInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback);
    int32_t GetPreferredUid(int32_t uid);
    bool GetEnhancedRoutingSupported() const;
    void SetDeviceEnableAndUsage(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);

private:
    int32_t ExcludeOutputDevicesInner(AudioDeviceUsage audioDevUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);
    int32_t ClearActiveHfpDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    int32_t UnexcludeOutputDevicesInner(AudioDeviceUsage audioDevUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);
    void HandleActiveBt(DeviceType deviceType, std::string macAddress);
    void HandleNegtiveBt(DeviceType deviceType);
    void SelectOutputDeviceLog(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors,
        const int32_t audioDeviceSelectMode = 0);
    int32_t SelectOutputDeviceForFastInner(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc);
    int32_t SetRenderDeviceForUsage(StreamUsage streamUsage, std::shared_ptr<AudioDeviceDescriptor> desc,
        const int32_t uid = -1);
    int32_t SelectFastOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    int32_t FetchInputAndOutputDevice(const std::string &caller,
        const AudioStreamDeviceChangeReasonExt outputReason = AudioStreamDeviceChangeReason::UNKNOWN,
        const AudioStreamDeviceChangeReasonExt inputReason = AudioStreamDeviceChangeReason::UNKNOWN);
    void HandleFetchDeviceChange(const AudioStreamDeviceChangeReason &reason, const std::string &caller);
    static void WriteDesignateAudioCaptureDeviceEvent(SourceType sourceType, int32_t deviceType,
        bool isNormalSelection);
    int32_t RefreshVirtualDevice(std::shared_ptr<AudioDeviceDescriptor> &desc);

    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
    sptr<IStandardAudioPolicyManagerListener> audioClientInfoMgrCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS


#endif // AUDIO_SELECT_INTERFACE_SERVICE_H
