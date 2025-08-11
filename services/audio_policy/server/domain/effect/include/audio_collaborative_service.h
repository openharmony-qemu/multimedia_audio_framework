/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_COLLABORATIVE_SERVICE_H
#define ST_AUDIO_COLLABORATIVE_SERVICE_H
#include <mutex>
#include <map>
#include "audio_device_descriptor.h"
#include "audio_effect.h"
#include "audio_policy_manager_factory.h"
#include "iaudio_policy_interface.h"
namespace OHOS {
namespace AudioStandard {
class AudioCollaborativeService {
public:
    static AudioCollaborativeService& GetAudioCollaborativeService()
    {
        static AudioCollaborativeService audioCollaborativeService;
        return audioCollaborativeService;
    }
    void Init(const std::vector<EffectChain> &effectChains);
    bool IsCollaborativePlaybackSupported();
    bool IsCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);
    // only function to change map state
    int32_t SetCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled);
    // current device change, map state unchanged
    void UpdateCurrentDevice(const AudioDeviceDescriptor &selectedAudioDevice);
    bool GetRealCollaborativeState();
private:
    AudioCollaborativeService()
        :audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())
    {}
    ~AudioCollaborativeService();
    // outputDeviceChange differentiate if updation is caused by output device change
    int32_t UpdateCollaborativeStateReal();
    bool isCollaborativePlaybackSupported_ = false;
    // same with current device in map
    bool isCollaborativeStateEnabled_ = false;
    std::string curDeviceAddress_;
    std::mutex collaborativeServiceMutex_;
    std::map<std::string, bool> addressToCollaborativeEnabledMap_;
    std::map<std::string, bool> addressToCollaborativeMemoryMap_;
    IAudioPolicyInterface& audioPolicyManager_;
};
} // OHOS
} // AudioStandard
#endif