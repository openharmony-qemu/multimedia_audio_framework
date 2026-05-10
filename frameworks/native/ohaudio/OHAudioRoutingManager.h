/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OH_AUDIO_ROUTING_MANAGER_H
#define OH_AUDIO_ROUTING_MANAGER_H

#include <unordered_map>
#include <vector>

#include "audio_common_log.h"
#include "native_audio_routing_manager.h"
#include "native_audio_common.h"
#include "native_audio_device_base.h"
#include "audio_system_manager.h"
#include "OHAudioDeviceDescriptor.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioDeviceChangedCallback : public AudioManagerDeviceChangeCallback {
public:
    explicit OHAudioDeviceChangedCallback(OH_AudioRoutingManager_OnDeviceChangedCallback callback)
        : callback_(callback)
    {
    }
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;

    OH_AudioRoutingManager_OnDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioDeviceChangedCallback()
    {
        AUDIO_INFO_LOG("~OHAudioDeviceChangedCallback called.");
        if (callback_ != nullptr) {
            callback_ = nullptr;
        }
    }

private:
    OH_AudioRoutingManager_OnDeviceChangedCallback callback_;
};

class OHAudioPreferredOutputDeviceChangedCallback : public AudioPreferredOutputDeviceChangeCallback {
public:
    explicit OHAudioPreferredOutputDeviceChangedCallback(
        OH_AudioRoutingManager_OnPreferredOutputDeviceChangedCallback callback)
        : callback_(callback)
    {
    }
    void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override;

    OH_AudioRoutingManager_OnPreferredOutputDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioPreferredOutputDeviceChangedCallback()
    {
        AUDIO_INFO_LOG("~OHAudioPreferredOutputDeviceChangedCallback called.");
        if (callback_ != nullptr) {
            callback_ = nullptr;
        }
    }

private:
    OH_AudioRoutingManager_OnPreferredOutputDeviceChangedCallback callback_;
};

class OHAudioPreferredInputDeviceChangedCallback : public AudioPreferredInputDeviceChangeCallback {
public:
    explicit OHAudioPreferredInputDeviceChangedCallback(
        OH_AudioRoutingManager_OnPreferredInputDeviceChangedCallback callback)
        : callback_(callback)
    {
    }
    void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override;

    OH_AudioRoutingManager_OnPreferredInputDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioPreferredInputDeviceChangedCallback()
    {
        AUDIO_INFO_LOG("~OHAudioPreferredInputDeviceChangedCallback called.");
        if (callback_ != nullptr) {
            callback_ = nullptr;
        }
    }

private:
    OH_AudioRoutingManager_OnPreferredInputDeviceChangedCallback callback_;
};

class OHMicrophoneBlockCallback : public AudioManagerMicrophoneBlockedCallback {
public:
    explicit OHMicrophoneBlockCallback(OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback, void *userData)
        : blockedCallback_(callback)
    {
    }

    OH_AudioRoutingManager_OnDeviceBlockStatusCallback GetCallback()
    {
        return blockedCallback_;
    }

    ~OHMicrophoneBlockCallback()
    {
        AUDIO_INFO_LOG("~OHMicrophoneBlockCallback called.");
        blockedCallback_ = nullptr;
    }
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override;

private:
    OH_AudioRoutingManager_OnDeviceBlockStatusCallback blockedCallback_;
};

class OHAudioRoutingManager {
public:
    ~OHAudioRoutingManager();

    static OHAudioRoutingManager* GetInstance()
    {
        static OHAudioRoutingManager ohAudioRoutingManager;
        return &ohAudioRoutingManager;
    }
    OH_AudioDeviceDescriptorArray* GetDevices(DeviceFlag deviceFlag);

    OH_AudioDeviceDescriptorArray *ConvertDesc(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    OH_AudioDeviceDescriptorArray *GetAvailableDevices(AudioDeviceUsage deviceUsage);
    OH_AudioDeviceDescriptorArray *GetPreferredOutputDevice(StreamUsage streamUsage);
    OH_AudioDeviceDescriptorArray *GetPreferredInputDevice(SourceType sourceType);

    OH_AudioCommon_Result SetDeviceChangeCallback(const DeviceFlag flag,
        OH_AudioRoutingManager_OnDeviceChangedCallback callback);
    OH_AudioCommon_Result UnsetDeviceChangeCallback(DeviceFlag flag,
        OH_AudioRoutingManager_OnDeviceChangedCallback ohOnDeviceChangedcallback);
    OH_AudioCommon_Result SetPreferredOutputDeviceChangeCallback(StreamUsage streamUsage,
        OH_AudioRoutingManager_OnPreferredOutputDeviceChangedCallback callback);
    OH_AudioCommon_Result RemoveAllPreferredOutputDeviceCallback();
    OH_AudioCommon_Result UnsetPreferredOutputDeviceChangeCallback(
        OH_AudioRoutingManager_OnPreferredOutputDeviceChangedCallback callback);
    OH_AudioCommon_Result SetPreferredInputDeviceChangeCallback(SourceType sourceType,
        OH_AudioRoutingManager_OnPreferredInputDeviceChangedCallback callback);
    OH_AudioCommon_Result RemoveAllPreferredInputDeviceCallback();
    OH_AudioCommon_Result UnsetPreferredInputDeviceChangeCallback(
        OH_AudioRoutingManager_OnPreferredInputDeviceChangedCallback callback);
    OH_AudioCommon_Result SetMicrophoneBlockedCallback(OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback,
        void* userData);
    OH_AudioCommon_Result UnsetMicrophoneBlockedCallback(OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback);

private:
    OHAudioRoutingManager();
    struct PreferredOutputCallbackInfo {
        int32_t uid;
        StreamUsage streamUsage;
    };
    struct PreferredInputCallbackInfo {
        int32_t uid;
        SourceType sourceType;
    };
    AudioSystemManager *audioSystemManager_ = AudioSystemManager::GetInstance();
    AudioRoutingManager *audioRoutingManager_ =  AudioRoutingManager::GetInstance();
    std::vector<std::shared_ptr<OHAudioDeviceChangedCallback>> ohAudioOnDeviceChangedCallbackArray_;
    std::vector<std::shared_ptr<OHMicrophoneBlockCallback>> ohMicroPhoneBlockCallbackArray_;

    std::vector<std::pair<std::shared_ptr<OHAudioPreferredOutputDeviceChangedCallback>, PreferredOutputCallbackInfo>>
        preferredOutputDeviceCallbackList_;
    std::vector<std::pair<std::shared_ptr<OHAudioPreferredInputDeviceChangedCallback>, PreferredInputCallbackInfo>>
        preferredInputDeviceCallbackList_;

    std::mutex audioPreferredOutputDeviceCbMutex_;
    std::mutex audioPreferredInputDeviceCbMutex_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_ROUTING_MANAGER_H