/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ROUTING_MANAGER_H
#define ST_AUDIO_ROUTING_MANAGER_H

#include <iostream>

#include "audio_system_manager.h"
#include "audio_group_manager.h"
#include "microphone_descriptor.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceDescriptor;
class AudioRendererFilter;
class AudioDeviceRefiner;

class AudioRoutingManager {
public:
    AudioRoutingManager() = default;
    virtual ~AudioRoutingManager() = default;

    static AudioRoutingManager *GetInstance();
    int32_t SetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);
    int32_t GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    int32_t SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback);
    int32_t UnsetPreferredOutputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback = nullptr);
    int32_t GetPreferredInputDeviceForCapturerInfo(AudioCapturerInfo captureInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    int32_t SetPreferredInputDeviceChangeCallback(AudioCapturerInfo capturerInfo,
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback);
    int32_t UnsetPreferredInputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback = nullptr);
    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);
    std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
    int32_t SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback);
    int32_t UnsetAudioDeviceRefinerCallback();
    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t SetPreferredDevice(const PreferredType preferredType, const std::shared_ptr<AudioDeviceDescriptor> &desc,
        const int32_t uid = INVALID_UID);
    void SaveRemoteInfo(const std::string &networkId, DeviceType deviceType);
    int32_t SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc, const bool isConnected);
private:
    int32_t GetCallingPid();
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ROUTING_MANAGER_H

