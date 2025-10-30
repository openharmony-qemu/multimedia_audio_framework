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

#ifndef TAIHE_AUDIO_VOLUME_MANAGER_H
#define TAIHE_AUDIO_VOLUME_MANAGER_H

#include "audio_system_manager.h"
#include "taihe_audio_volume_group_manager.h"
#include "taihe_audio_system_volume_change_callback.h"
#include "taihe_audio_stream_volume_change_callback.h"
#include "taihe_audio_volume_key_event.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioVolumeManagerImpl {
public:
    AudioVolumeManagerImpl();
    explicit AudioVolumeManagerImpl(std::shared_ptr<AudioVolumeManagerImpl> obj);
    ~AudioVolumeManagerImpl();

    static AudioVolumeManager CreateVolumeManagerWrapper();

    array<VolumeGroupInfo> GetVolumeGroupInfosSync(string_view networkId);
    int32_t GetAppVolumePercentageForUidSync(int32_t uid);
    void SetAppVolumePercentageForUidSync(int32_t uid, int32_t volume);
    bool IsAppVolumeMutedForUidSync(int32_t uid, bool owned);
    void SetAppVolumeMutedForUidSync(int32_t uid, bool muted);
    int32_t GetAppVolumePercentageSync();
    void SetAppVolumePercentageSync(int32_t volume);
    AudioVolumeGroupManager GetVolumeGroupManagerSync(int32_t groupId);
    int32_t GetSystemVolume(AudioVolumeType volumeType);
    int32_t GetMinSystemVolume(AudioVolumeType volumeType);
    int32_t GetMaxSystemVolume(AudioVolumeType volumeType);
    int32_t GetVolumeByStream(StreamUsage streamUsage);
    int32_t GetMinVolumeByStream(StreamUsage streamUsage);
    int32_t GetMaxVolumeByStream(StreamUsage streamUsage);
    double GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device);
    double GetVolumeInUnitOfDbByStream(StreamUsage streamUsage, int32_t volumeLevel, DeviceType device);
    array<AudioVolumeType> GetSupportedAudioVolumeTypes();
    AudioVolumeType GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage);
    array<StreamUsage> GetStreamUsagesByVolumeType(AudioVolumeType volumeType);
    bool IsSystemMuted(AudioVolumeType volumeType);
    bool IsSystemMutedForStream(StreamUsage streamUsage);
    int32_t GetSystemVolumeByUid(AudioVolumeType volumeType, int32_t callingUid);
    void SetSystemVolumeByUidSync(AudioVolumeType volumeType, int32_t volume, int32_t callingUid);
    void ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration);
    void OnVolumeChange(callback_view<void(VolumeEvent const&)> callback);
    void OnAppVolumeChangeForUid(int32_t uid, callback_view<void(VolumeEvent const&)> callback);
    void OnAppVolumeChange(callback_view<void(VolumeEvent const&)> callback);
    void OnActiveVolumeTypeChange(callback_view<void(AudioVolumeType)> callback);
    void OnSystemVolumeChange(callback_view<void(VolumeEvent const&)> callback);
    void OnStreamVolumeChange(StreamUsage streamUsage, callback_view<void(StreamVolumeEvent const&)> callback);
    void OffVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback);
    void OffAppVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback);
    void OffAppVolumeChangeForUid(optional_view<callback<void(VolumeEvent const&)>> callback);
    void OffActiveVolumeTypeChange(optional_view<callback<void(AudioVolumeType)>> callback);
    void OffSystemVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback);
    void OffStreamVolumeChange(optional_view<callback<void(StreamVolumeEvent const&)>> callback);

private:
    static void RegisterCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl);
    static void RegisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl);
    static void RegisterActiveVolumeTypeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl);
    static void RegisterAppVolumeChangeForUidCallback(int32_t appUid, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl);
    static void RegisterSystemVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl);
    static void RegisterStreamVolumeChangeCallback(StreamUsage streamUsage, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl);
    static void UnregisterCallback(std::shared_ptr<uintptr_t> &callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);
    static void UnregisterAppVolumeChangeForUidCallback(std::shared_ptr<uintptr_t> &callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);
    static void UnregisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);
    static void UnregisterActiveVolumeTypeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);
    static void UnregisterSystemVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);
    static void UnregisterStreamVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);

    static std::shared_ptr<TaiheAudioVolumeKeyEvent> GetVolumeEventTaiheCallback(std::shared_ptr<uintptr_t> callback,
        AudioVolumeManagerImpl *audioVolMngrImpl);
    static std::shared_ptr<TaiheAudioSystemVolumeChangeCallback> GetSystemVolumeChangeTaiheCallback(
        std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl);
    static std::shared_ptr<TaiheAudioStreamVolumeChangeCallback> GetStreamVolumeChangeTaiheCallback(
        std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl);

    OHOS::AudioStandard::AudioSystemManager *audioSystemMngr_;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<OHOS::AudioStandard::VolumeKeyEventCallback> volumeKeyEventCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::StreamVolumeChangeCallback> streamVolumeChangeCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::SystemVolumeChangeCallback> systemVolumeChangeCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerAppVolumeChangeCallback>
        selfAppVolumeChangeCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerAppVolumeChangeCallback>
        appVolumeChangeCallbackForUidTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerActiveVolumeTypeChangeCallback>
        activeVolumeTypeChangeCallbackTaihe_ = nullptr;
    std::list<std::shared_ptr<TaiheAudioVolumeKeyEvent>> volumeKeyEventCallbackTaiheList_;
    std::list<std::shared_ptr<TaiheAudioStreamVolumeChangeCallback>> streamVolumeChangeCallbackTaiheList_;
    std::list<std::shared_ptr<TaiheAudioSystemVolumeChangeCallback>> systemVolumeChangeCallbackTaiheList_;
    std::mutex mutex_;
};
} // namespace ANI::Audio

#endif // FRAMEWORKS_TAIHE_INCLUDE_TAIHE_AUDIO_VOLUME_MANAGER_H
