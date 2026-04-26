/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_STATE_MANAGER_H
#define ST_AUDIO_STATE_MANAGER_H

#include <unordered_set>
#include <shared_mutex>
#include "audio_device_descriptor.h"
#include "audio_usr_select_manager.h"
#include "istandard_audio_policy_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioStateManager {
public:
    static AudioStateManager& GetAudioStateManager()
    {
        static AudioStateManager audioStateManager;
        return audioStateManager;
    }

    // Set ring render device selected by the user
    void SetPreferredRingRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set tone render device selected by the user
    void SetPreferredToneRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Get ring render device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredRingRenderDevice();

    // Get tone render device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredToneRenderDevice();

    void SetPreferredRecognitionCaptureDevice(const shared_ptr<AudioDeviceDescriptor> &desc);
    shared_ptr<AudioDeviceDescriptor> GetPreferredRecognitionCaptureDevice();

    int32_t SetAudioVKBInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback);
    int32_t CheckVKBInfo(const std::string &bundleName, bool &isValid);

private:
    AudioStateManager() {};
    ~AudioStateManager() {};
    std::shared_ptr<AudioDeviceDescriptor> preferredRingRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredToneRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredRecognitionCaptureDevice_{make_shared<AudioDeviceDescriptor>()};

    std::mutex mutex_;
    sptr<IStandardAudioPolicyManagerListener> audioVKBInfoMgrCallback_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_STATE_MANAGER_H
