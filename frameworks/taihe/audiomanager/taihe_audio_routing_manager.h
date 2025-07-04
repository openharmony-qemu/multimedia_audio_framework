/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_ROUTING_MANAGER_H
#define TAIHE_AUDIO_ROUTING_MANAGER_H

#include "audio_log.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "taihe_work.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_routing_manager_callbacks.h"
#include "taihe_audio_manager_callbacks.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
class AudioRoutingManagerImpl {
public:
    AudioRoutingManagerImpl();
    explicit AudioRoutingManagerImpl(std::shared_ptr<AudioRoutingManagerImpl> obj);
    ~AudioRoutingManagerImpl();

    static AudioRoutingManager CreateRoutingManagerWrapper();

#if !defined(IOS_PLATFORM) && !defined(ANDROID_PLATFORM)
    bool IsMicBlockDetectionSupportedSync();
#endif
    array<AudioDeviceDescriptor> GetDevicesSync(DeviceFlag deviceFlag);
    array<AudioDeviceDescriptor> GetAvailableDevices(DeviceUsage usage);
    array<AudioDeviceDescriptor> GetExcludedDevices(DeviceUsage usage);
    array<AudioDeviceDescriptor> GetPreferredOutputDeviceForRendererInfoSync(AudioRendererInfo const &rendererInfo);
    array<AudioDeviceDescriptor> GetPreferredInputDeviceForCapturerInfoSync(AudioCapturerInfo const &capturerInfo);
    void SelectOutputDeviceSync(array_view<AudioDeviceDescriptor> outputAudioDevices);
    void SelectOutputDeviceByFilterSync(AudioRendererFilter const &filter,
        array_view<AudioDeviceDescriptor> outputAudioDevices);
    array<AudioDeviceDescriptor> GetPreferredInputDeviceByFilter(AudioCapturerFilter const &filter);
    array<AudioDeviceDescriptor> GetPreferredOutputDeviceByFilter(AudioRendererFilter const &filter);
    void SelectInputDeviceByFilterSync(AudioCapturerFilter const &filter,
        array_view<AudioDeviceDescriptor> inputAudioDevices);
    void SelectInputDeviceSync(array_view<AudioDeviceDescriptor> inputAudioDevices);
    void SetCommunicationDeviceSync(CommunicationDeviceType deviceType, bool active);
    bool IsCommunicationDeviceActiveSync(CommunicationDeviceType deviceType);
    void ExcludeOutputDevicesSync(DeviceUsage usage, array_view<AudioDeviceDescriptor> devices);
    void UnexcludeOutputDevicesWithUsageAndDevices(DeviceUsage usage, array_view<AudioDeviceDescriptor> devices);
    void UnexcludeOutputDevicesWithUsage(DeviceUsage usage);
    void OnPreferredInputDeviceChangeForCapturerInfo(AudioCapturerInfo const &capturerInfo,
        callback_view<void(array_view<AudioDeviceDescriptor>)> callback);
    void OnMicBlockStatusChanged(callback_view<void(DeviceBlockStatusInfo const&)> callback);
    void OnDeviceChange(DeviceFlag deviceFlag, callback_view<void(DeviceChangeAction const&)> callback);
    void OnAvailableDeviceChange(DeviceUsage deviceUsage, callback_view<void(DeviceChangeAction const&)> callback);
    void OnPreferOutputDeviceChangeForRendererInfo(AudioRendererInfo const &rendererInfo,
        callback_view<void(array_view<AudioDeviceDescriptor>)> callback);
    void OffMicBlockStatusChanged(optional_view<callback<void(DeviceBlockStatusInfo const&)>> callback);
    void OffAvailableDeviceChange(optional_view<callback<void(DeviceChangeAction const&)>> callback);
    void OffPreferredInputDeviceChangeForCapturerInfo(optional_view<callback<void(array_view<AudioDeviceDescriptor>)>>
        callback);
    void OffPreferOutputDeviceChangeForRendererInfo(optional_view<callback<void(array_view<AudioDeviceDescriptor>)>>
        callback);
    void OffDeviceChange(optional_view<callback<void(DeviceChangeAction const&)>> callback);

private:
    static void RegisterDeviceChangeCallback(DeviceFlag deviceFlag, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRoutingManagerImpl *taiheRoutingMgr);
    static void RegisterAvaiableDeviceChangeCallback(DeviceUsage deviceUsage, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRoutingManagerImpl *taiheRoutingMgr);
    static void RegisterPreferredInputDeviceChangeCallback(AudioCapturerInfo const &capturerInfo,
        std::shared_ptr<uintptr_t> &callback, const std::string &cbName,
            AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void RegisterPreferredOutputDeviceChangeCallback(AudioRendererInfo const &rendererInfo,
        std::shared_ptr<uintptr_t> &callback, const std::string &cbName,
            AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void RegisterMicrophoneBlockedCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void UnregisterDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void UnregisterAvailableDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void UnregisterPreferredInputDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void UnregisterPreferredOutputDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void UnregisterMicrophoneBlockedCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void AddPreferredInputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl,
        std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb);
    static void AddPreferredOutputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl,
        std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb);
    static std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> GetTaihePrefInputDeviceChangeCb(
        std::shared_ptr<uintptr_t> &callback, AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> GetTaihePrefOutputDeviceChangeCb(
        std::shared_ptr<uintptr_t> &callback, AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void RemovePreferredInputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl,
        std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb);
    static void RemovePreferredOutputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl,
        std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb);
    static void RemoveAllPrefInputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl);
    static void RemoveAllPrefOutputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl);

    std::mutex preferredOutputDeviceMutex_;
    std::mutex preferredInputDeviceMutex_;
    std::mutex mutex_;

    OHOS::AudioStandard::AudioSystemManager *audioMngr_;
    OHOS::AudioStandard::AudioRoutingManager *audioRoutingMngr_ = nullptr;

    std::shared_ptr<OHOS::AudioStandard::AudioManagerMicrophoneBlockedCallback> microphoneBlockedCallbackTaihe_
        = nullptr;
    std::list<std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback>> preferredInputDeviceCallbacks_;
    std::list<std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback>> preferredOutputDeviceCallbacks_;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerDeviceChangeCallback> deviceChangeCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerAvailableDeviceChangeCallback>
        availableDeviceChangeCallbackTaihe_ = nullptr;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_ROUTING_MANAGER_H
