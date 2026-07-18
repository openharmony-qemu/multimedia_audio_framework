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

#ifndef TAIHE_AUDIO_SPATIALIZATION_MANAGER_H
#define TAIHE_AUDIO_SPATIALIZATION_MANAGER_H

#include "audio_spatialization_manager.h"
#include "audio_utils.h"
#include "taihe_audio_spatialization_manager_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioSpatializationManagerImpl {
public:
    AudioSpatializationManagerImpl();
    explicit AudioSpatializationManagerImpl(std::shared_ptr<AudioSpatializationManagerImpl> obj);
    ~AudioSpatializationManagerImpl();

    static AudioSpatializationManager CreateSpatializationManagerWrapper();

    bool IsSpatializationSupported();
    bool IsSpatializationSupportedForDevice(AudioDeviceDescriptor deviceDescriptor);
    bool IsHeadTrackingSupported();
    bool IsHeadTrackingSupportedForDevice(AudioDeviceDescriptor deviceDescriptor);
    void SetSpatializationEnabledSync(AudioDeviceDescriptor deviceDescriptor, bool enabled);
    bool IsSpatializationEnabled(AudioDeviceDescriptor deviceDescriptor);
    bool IsSpatializationEnabledForCurrentDevice();
    void SetHeadTrackingEnabledSync(AudioDeviceDescriptor deviceDescriptor, bool enabled);
    bool IsHeadTrackingEnabled(AudioDeviceDescriptor deviceDescriptor);
    void UpdateSpatialDeviceState(AudioSpatialDeviceState spatialDeviceState);
    void SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType);
    AudioSpatializationSceneType GetSpatializationSceneType();

    void OnSpatializationEnabledChangeForCurrentDevice(callback_view<void(bool)> callback);
    void OnSpatializationEnabledChangeForAnyDevice(
        callback_view<void(AudioSpatialEnabledStateForDevice const&)> callback);
    void OnHeadTrackingEnabledChangeForAnyDevice(
        callback_view<void(AudioSpatialEnabledStateForDevice const&)> callback);
    void OffSpatializationEnabledChangeForCurrentDevice(optional_view<callback<void(bool)>> callback);
    void OffSpatializationEnabledChangeForAnyDevice(
        optional_view<callback<void(AudioSpatialEnabledStateForDevice const&)>> callback);
    void OffHeadTrackingEnabledChangeForAnyDevice(
        optional_view<callback<void(AudioSpatialEnabledStateForDevice const&)>> callback);

private:
    static void RegisterSpatializationEnabledChangeForCurrentDeviceCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager);
    static void RegisterSpatializationEnabledChangeCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager);
    static void RegisterHeadTrackingEnabledChangeCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager);
    static void UnregisterSpatializationEnabledChangeForCurrentDeviceCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager);
    static void UnregisterSpatializationEnabledChangeCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager);
    static void UnregisterHeadTrackingEnabledChangeCallback(
        std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager);

    OHOS::AudioStandard::AudioSpatializationManager *audioSpatializationMngr_;

    std::shared_ptr<OHOS::AudioStandard::AudioSpatializationEnabledChangeForCurrentDeviceCallback>
        spatializationEnabledChangeForCurrentDeviceCallback_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioSpatializationEnabledChangeCallback>
        spatializationEnabledChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioHeadTrackingEnabledChangeCallback>
        headTrackingEnabledChangeCallback_ = nullptr;
    std::mutex mutex_;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_SPATIALIZATION_MANAGER_H