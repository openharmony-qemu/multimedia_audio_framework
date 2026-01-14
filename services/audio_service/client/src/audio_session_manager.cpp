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
#include "audio_session_manager.h"
#include "audio_session_client_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioSessionManager *AudioSessionManager::GetInstance()
{
    static AudioSessionManager audioSessionManager;
    return &audioSessionManager;
}

int32_t AudioSessionManager::ActivateAudioSession(const AudioSessionStrategy &strategy)
{
    return AudioSessionClientManager::GetInstance().ActivateAudioSession(strategy);
}

int32_t AudioSessionManager::DeactivateAudioSession()
{
    return AudioSessionClientManager::GetInstance().DeactivateAudioSession();
}

bool AudioSessionManager::IsAudioSessionActivated()
{
    return AudioSessionClientManager::GetInstance().IsAudioSessionActivated();
}

bool AudioSessionManager::IsOtherMediaPlaying()
{
    return AudioSessionClientManager::GetInstance().IsOtherMediaPlaying();
}

int32_t AudioSessionManager::SetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
{
    return AudioSessionClientManager::GetInstance().SetAudioSessionCallback(audioSessionCallback);
}

int32_t AudioSessionManager::UnsetAudioSessionCallback()
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionCallback();
}

int32_t AudioSessionManager::UnsetAudioSessionCallback(
    const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionCallback(audioSessionCallback);
}

int32_t AudioSessionManager::SetAudioSessionScene(const AudioSessionScene audioSessionScene)
{
    return AudioSessionClientManager::GetInstance().SetAudioSessionScene(audioSessionScene);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSessionManager::GetAvailableDevices(AudioDeviceUsage usage)
{
    return AudioSessionClientManager::GetInstance().GetAvailableDevices(usage);
}

int32_t AudioSessionManager::SelectInputDevice(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor)
{
    return AudioSessionClientManager::GetInstance().SelectInputDevice(audioDeviceDescriptor);
}

std::shared_ptr<AudioDeviceDescriptor> AudioSessionManager::GetSelectedInputDevice()
{
    return AudioSessionClientManager::GetInstance().GetSelectedInputDevice();
}

int32_t AudioSessionManager::ClearSelectedInputDevice()
{
    return AudioSessionClientManager::GetInstance().ClearSelectedInputDevice();
}

int32_t AudioSessionManager::PreferBluetoothAndNearlinkRecord(BluetoothAndNearlinkPreferredRecordCategory category)
{
    return AudioSessionClientManager::GetInstance().PreferBluetoothAndNearlinkRecord(category);
}

BluetoothAndNearlinkPreferredRecordCategory AudioSessionManager::GetPreferBluetoothAndNearlinkRecord()
{
    return AudioSessionClientManager::GetInstance().GetPreferBluetoothAndNearlinkRecord();
}

int32_t AudioSessionManager::SetAudioSessionStateChangeCallback(
    const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback)
{
    return AudioSessionClientManager::GetInstance().SetAudioSessionStateChangeCallback(stateChangedCallback);
}

int32_t AudioSessionManager::UnsetAudioSessionStateChangeCallback()
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionStateChangeCallback();
}

int32_t AudioSessionManager::UnsetAudioSessionStateChangeCallback(
    const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback)
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionStateChangeCallback(stateChangedCallback);
}

int32_t AudioSessionManager::GetDefaultOutputDevice(DeviceType &deviceType)
{
    return AudioSessionClientManager::GetInstance().GetDefaultOutputDevice(deviceType);
}

int32_t AudioSessionManager::SetDefaultOutputDevice(DeviceType deviceType)
{
    return AudioSessionClientManager::GetInstance().SetDefaultOutputDevice(deviceType);
}

int32_t AudioSessionManager::SetAudioSessionCurrentDeviceChangeCallback(
    const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback)
{
    return AudioSessionClientManager::GetInstance().SetAudioSessionCurrentDeviceChangeCallback(deviceChangedCallback);
}

int32_t AudioSessionManager::SetAudioSessionCurrentInputDeviceChangeCallback(
    const std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback> &deviceChangedCallback)
{
    return AudioSessionClientManager::GetInstance().SetAudioSessionCurrentInputDeviceChangeCallback(
        deviceChangedCallback);
}

int32_t AudioSessionManager::UnsetAudioSessionCurrentDeviceChangeCallback()
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionCurrentDeviceChangeCallback();
}

int32_t AudioSessionManager::UnsetAudioSessionCurrentDeviceChangeCallback(
    const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback)
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionCurrentDeviceChangeCallback(
        deviceChangedCallback);
}

int32_t AudioSessionManager::UnsetAudioSessionCurrentInputDeviceChangeCallback(
    const std::optional<std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback>> &callback)
{
    return AudioSessionClientManager::GetInstance().UnsetAudioSessionCurrentInputDeviceChangeCallback(callback);
}

int32_t AudioSessionManager::EnableMuteSuggestionWhenMixWithOthers(bool enable)
{
    return AudioSessionClientManager::GetInstance().EnableMuteSuggestionWhenMixWithOthers(enable);
}

void AudioSessionManager::RegisterAudioPolicyServerDiedCb()
{
    AudioSessionClientManager::GetInstance().RegisterAudioPolicyServerDiedCb();
}

bool AudioSessionManager::Restore()
{
    return AudioSessionClientManager::GetInstance().Restore();
}

void AudioSessionManager::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    AudioSessionClientManager::GetInstance().OnAudioSessionDeactive(deactiveEvent);
}

void AudioSessionManager::OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent)
{
    AudioSessionClientManager::GetInstance().OnAudioSessionStateChanged(stateChangedEvent);
}

} // namespace AudioStandard
} // namespace OHOS
