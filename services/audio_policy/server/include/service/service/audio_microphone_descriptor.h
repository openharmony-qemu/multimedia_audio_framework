/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_MICROPHONE_DESCRIPTOR_H
#define ST_AUDIO_MICROPHONE_DESCRIPTOR_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "microphone_descriptor.h"
#include "audio_errors.h"


namespace OHOS {
namespace AudioStandard {

class AudioMicrophoneDescriptor {
public:
    static AudioMicrophoneDescriptor& GetInstance()
    {
        static AudioMicrophoneDescriptor instance;
        return instance;
    }
    int32_t SetMicrophoneMute(bool isMute);
    int32_t SetMicrophoneMutePersistent(const bool isMute);
    int32_t InitPersistentMicrophoneMuteState(bool &isMute);
    bool GetPersistentMicMuteState();
    bool IsMicrophoneMute();
    bool GetMicrophoneMuteTemporary();
    bool GetMicrophoneMutePersistent();

    void AddMicrophoneDescriptor(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    void RemoveMicrophoneDescriptor(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    void AddAudioCapturerMicrophoneDescriptor(int32_t sessionId, DeviceType devType);

    void UpdateAudioCapturerMicrophoneDescriptor(DeviceType devType);

    void RemoveAudioCapturerMicrophoneDescriptor(int32_t uid);

    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);

    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();

    void RemoveAudioCapturerMicrophoneDescriptorBySessionID(int32_t sessionID);
private:
    AudioMicrophoneDescriptor() {}
    ~AudioMicrophoneDescriptor() {}
private:
    bool isMicrophoneMuteTemporary_ = false;
    bool isMicrophoneMutePersistent_ = false;

    std::vector<sptr<MicrophoneDescriptor>> connectedMicrophones_;
    std::unordered_map<int32_t, sptr<MicrophoneDescriptor>> audioCaptureMicrophoneDescriptor_;
};

}
}

#endif