/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "audio_info.h"
#include "audio_group_manager.h"
#include "microphone_descriptor.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceDescriptor;
class AudioRendererFilter;
class AudioDeviceRefiner;
class AudioPreferredOutputDeviceChangeCallback {
public:
    virtual ~AudioPreferredOutputDeviceChangeCallback() = default;
    /**
     * Called when the prefer output device changes
     *
     * @param vector<sptr<AudioDeviceDescriptor>> deviceDescriptor.
     */
    virtual void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
};

class AudioPreferredInputDeviceChangeCallback {
    public:
    virtual ~AudioPreferredInputDeviceChangeCallback() = default;
    /**
     * Called when the prefer input device changes
     *
     * @param vector<sptr<AudioDeviceDescriptor>> deviceDescriptor.
     */
    virtual void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
};

class AudioRoutingManager {
public:
    AudioRoutingManager() = default;
    virtual ~AudioRoutingManager() = default;

    static AudioRoutingManager *GetInstance();
    int32_t SetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);
    int32_t GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &desc);
    int32_t SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback);
    int32_t UnsetPreferredOutputDeviceChangeCallback();
    int32_t GetPreferredInputDeviceForCapturerInfo(AudioCapturerInfo captureInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &desc);
    int32_t SetPreferredInputDeviceChangeCallback(AudioCapturerInfo captureInfo,
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback);
    int32_t UnsetPreferredInputDeviceChangeCallback();
    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);
    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
    int32_t SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback);
    int32_t UnsetAudioDeviceRefinerCallback();
    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
private:
    int32_t GetCallingPid();
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ROUTING_MANAGER_H

