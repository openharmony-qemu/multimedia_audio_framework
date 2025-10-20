/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OH_AUDIO_SESSION_MANAGER_H
#define OH_AUDIO_SESSION_MANAGER_H

#include "audio_manager_log.h"
#include "native_audio_session_manager.h"
#include "audio_session_manager.h"
#include "audio_system_manager.h"
#include "OHAudioDeviceDescriptor.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioSessionCallback : public AudioSessionCallback {
public:
    explicit OHAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback)
        : callback_(callback)
    {
    }

    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override;

    OH_AudioSession_DeactivatedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioSessionCallback()
    {
        AUDIO_INFO_LOG("~OHAudioSessionCallback called.");
        callback_ = nullptr;
    }
private:
    OH_AudioSession_DeactivatedCallback callback_;
};

class OHAudioSessionStateCallback : public AudioSessionStateChangedCallback {
public:
    explicit OHAudioSessionStateCallback(OH_AudioSession_StateChangedCallback callback)
        : callback_(callback)
    {
    }

    void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent) override;

    OH_AudioSession_StateChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioSessionStateCallback()
    {
        AUDIO_INFO_LOG("~OHAudioSessionStateCallback called.");
        callback_ = nullptr;
    }

private:
    OH_AudioSession_StateChangedCallback callback_;
};

class OHAudioSessionDeviceCallback : public AudioSessionCurrentDeviceChangedCallback {
public:
    explicit OHAudioSessionDeviceCallback(OH_AudioSession_CurrentOutputDeviceChangedCallback callback)
        : callback_(callback)
    {
    }

    void OnAudioSessionCurrentDeviceChanged(const CurrentOutputDeviceChangedEvent &deviceChangedEvent) override;

    OH_AudioSession_CurrentOutputDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioSessionDeviceCallback()
    {
        AUDIO_INFO_LOG("~OHAudioSessionDeviceCallback called.");
        callback_ = nullptr;
    }

private:
    OH_AudioSession_CurrentOutputDeviceChangedCallback callback_;
};

class OHAudioAvailableDeviceCallback : public AudioManagerAvailableDeviceChangeCallback {
public:
    explicit OHAudioAvailableDeviceCallback(AudioDeviceUsage deviceUsage,
        OH_AudioSession_AvailableDeviceChangedCallback callback)
        : deviceUsage_(deviceUsage), callback_(callback)
    {
    }

    void OnAvailableDeviceChange(const AudioDeviceUsage usage, const DeviceChangeAction &deviceChangeAction) override;

    OH_AudioSession_AvailableDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioAvailableDeviceCallback()
    {
        AUDIO_INFO_LOG("~OHAudioAvailableDeviceCallback called.");
        callback_ = nullptr;
    }

private:
    AudioDeviceUsage deviceUsage_;
    OH_AudioSession_AvailableDeviceChangedCallback callback_;
};

class OHAudioSessionInputDeviceCallback : public AudioSessionCurrentInputDeviceChangedCallback {
public:
    explicit OHAudioSessionInputDeviceCallback(OH_AudioSession_CurrentInputDeviceChangedCallback callback)
        : callback_(callback)
    {
    }

    void OnAudioSessionCurrentInputDeviceChanged(const CurrentInputDeviceChangedEvent &deviceChangedEvent) override;

    OH_AudioSession_CurrentInputDeviceChangedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioSessionInputDeviceCallback()
    {
        AUDIO_INFO_LOG("~OHAudioSessionInputDeviceCallback called.");
        callback_ = nullptr;
    }

private:
    OH_AudioSession_CurrentInputDeviceChangedCallback callback_;
};

class OHAudioSessionManager {
public:
    ~OHAudioSessionManager();

    static OHAudioSessionManager* GetInstance()
    {
        static OHAudioSessionManager ohAudioSessionManager;
        return &ohAudioSessionManager;
    }

    OH_AudioCommon_Result ActivateAudioSession(const AudioSessionStrategy &strategy);

    OH_AudioCommon_Result DeactivateAudioSession();

    bool IsAudioSessionActivated();

    OH_AudioDeviceDescriptorArray *GetAvailableDevices(AudioDeviceUsage deviceUsage);
    OH_AudioCommon_Result SelectMediaInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    OH_AudioDeviceDescriptor *GetSelectedMediaInputDevice();
    OH_AudioCommon_Result ClearSelectedMediaInputDevice();
    OH_AudioCommon_Result PreferBluetoothAndNearlinkRecord(BluetoothAndNearlinkPreferredRecordCategory category);
    BluetoothAndNearlinkPreferredRecordCategory GetPreferredBluetoothAndNearlinkRecord();

    OH_AudioCommon_Result SetAvailableDeviceChangeCallback(
        AudioDeviceUsage deviceUsage, OH_AudioSession_AvailableDeviceChangedCallback callback);
    OH_AudioCommon_Result UnsetAvailableDeviceChangeCallback(
        OH_AudioSession_AvailableDeviceChangedCallback callback);
    OH_AudioCommon_Result SetAudioSessionCurrentInputDeviceChangeCallback(
        OH_AudioSession_CurrentInputDeviceChangedCallback callback);
    OH_AudioCommon_Result UnsetAudioSessionCurrentInputDeviceChangeCallback(
        OH_AudioSession_CurrentInputDeviceChangedCallback callback);

    OH_AudioCommon_Result SetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback);

    OH_AudioCommon_Result UnsetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback);

    OH_AudioCommon_Result SetAudioSessionScene(AudioSessionScene sene);
    OH_AudioCommon_Result SetAudioSessionStateChangeCallback(OH_AudioSession_StateChangedCallback callback);
    OH_AudioCommon_Result UnsetAudioSessionStateChangeCallback(OH_AudioSession_StateChangedCallback callback);
    OH_AudioCommon_Result SetDefaultOutputDevice(DeviceType deviceType);
    OH_AudioCommon_Result GetDefaultOutputDevice(DeviceType &deviceType);
    OH_AudioCommon_Result SetAudioSessionCurrentDeviceChangeCallback(
        OH_AudioSession_CurrentOutputDeviceChangedCallback callback);
    OH_AudioCommon_Result UnsetAudioSessionCurrentDeviceChangeCallback(
        OH_AudioSession_CurrentOutputDeviceChangedCallback callback);

private:
    OHAudioSessionManager();

    AudioSessionManager *audioSessionManager_ = AudioSessionManager::GetInstance();
    AudioSystemManager *audioMngr_ = AudioSystemManager::GetInstance();

    std::map<OH_AudioSession_StateChangedCallback,
        std::shared_ptr<OHAudioSessionStateCallback>> sessionStateCallbacks_;
    std::map<OH_AudioSession_CurrentOutputDeviceChangedCallback,
        std::shared_ptr<OHAudioSessionDeviceCallback>> sessionDeviceCallbacks_;
    std::map<OH_AudioSession_AvailableDeviceChangedCallback,
        std::shared_ptr<OHAudioAvailableDeviceCallback>> availebleDeviceCallbacks_;
    std::map<OH_AudioSession_CurrentInputDeviceChangedCallback,
        std::shared_ptr<OHAudioSessionInputDeviceCallback>> sessionInputDeviceCallbacks_;

    std::mutex sessionStateCbMutex_;
    std::mutex sessionDeviceCbMutex_;
    std::mutex availableDeviceCbMutex_;
    std::mutex sessionInputDeviceCbMutex_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_SESSION_MANAGER_H