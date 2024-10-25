/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIMEDIA_AUDIO_ROUTING_MANAGER_CALLBACK_H
#define MULTIMEDIA_AUDIO_ROUTING_MANAGER_CALLBACK_H
#include "audio_routing_manager.h"
#include "audio_system_manager.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
class CjAudioManagerMicrophoneBlockedCallback : public AudioManagerMicrophoneBlockedCallback {
public:
    CjAudioManagerMicrophoneBlockedCallback() = default;
    virtual ~CjAudioManagerMicrophoneBlockedCallback() = default;

    void RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback);

    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override;

private:
    std::function<void(CArrDeviceDescriptor)> func_{};
    std::mutex cbMutex_;
};

class CjAudioPreferredInputDeviceChangeCallback : public AudioPreferredInputDeviceChangeCallback {
public:
    CjAudioPreferredInputDeviceChangeCallback() = default;
    virtual ~CjAudioPreferredInputDeviceChangeCallback() = default;

    void RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback);

    void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;

private:
    std::function<void(CArrDeviceDescriptor)> func_{};
    std::mutex cbMutex_;
};

class CjAudioManagerDeviceChangeCallback : public AudioManagerDeviceChangeCallback {
public:
    CjAudioManagerDeviceChangeCallback() = default;
    virtual ~CjAudioManagerDeviceChangeCallback() = default;

    void RegisterFunc(std::function<void(CDeviceChangeAction)> cjCallback);

    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;

private:
    std::function<void(CDeviceChangeAction)> func_{};
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_ROUTING_MANAGER_CALLBACK_H
