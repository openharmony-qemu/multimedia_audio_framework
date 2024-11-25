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

#include "audio_system_manager.h"

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

    // Set media render device selected by the user
    void SetPreferredMediaRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set call render device selected by the user
    void SetPreferredCallRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set call capture device selected by the user
    void SetPreferredCallCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set ring render device selected by the user
    void SetPreferredRingRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set record capture device selected by the user
    void SetPreferredRecordCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set tone render device selected by the user
    void SetPreferredToneRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Get media render device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredMediaRenderDevice();

    // Get call render device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredCallRenderDevice();

    // Get call capture device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredCallCaptureDevice();

    // Get ring render device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredRingRenderDevice();

    // Get record capture device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredRecordCaptureDevice();

    // Get tone render device selected by the user
    shared_ptr<AudioDeviceDescriptor> GetPreferredToneRenderDevice();

    void UpdatePreferredMediaRenderDeviceConnectState(ConnectState state);
    void UpdatePreferredCallRenderDeviceConnectState(ConnectState state);
    void UpdatePreferredCallCaptureDeviceConnectState(ConnectState state);
    void UpdatePreferredRecordCaptureDeviceConnectState(ConnectState state);

private:
    AudioStateManager() {};
    ~AudioStateManager() {};
    std::shared_ptr<AudioDeviceDescriptor> preferredMediaRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredCallRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredCallCaptureDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredRingRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredRecordCaptureDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredToneRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::mutex mutex_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_STATE_MANAGER_H

