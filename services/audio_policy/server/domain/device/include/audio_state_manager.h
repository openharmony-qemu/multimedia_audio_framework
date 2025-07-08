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
#include "audio_system_manager.h"
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

    // Set media render device selected by the user
    void SetPreferredMediaRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set call render device selected by the user
    void SetPreferredCallRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const int32_t uid = INVALID_UID, const std::string caller = "");

    // Set call capture device selected by the user
    void SetPreferredCallCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set ring render device selected by the user
    void SetPreferredRingRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set record capture device selected by the user
    void SetPreferredRecordCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    // Set tone render device selected by the user
    void SetPreferredToneRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    void ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);

    void UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);

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

    vector<shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(AudioDeviceUsage audioDevUsage);
    bool IsExcludedDevice(AudioDeviceUsage audioDevUsage,
        const shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptor);

    void SetAudioSceneOwnerUid(const int32_t uid);
    
    int32_t SetAudioClientInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback);
    
    int32_t SetAudioVKBInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback);
    int32_t CheckVKBInfo(const std::string &bundleName, bool &isValid);

private:
    AudioStateManager() {};
    ~AudioStateManager() {};
    std::shared_ptr<AudioDeviceDescriptor> preferredMediaRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredCallRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredCallCaptureDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredRingRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredRecordCaptureDevice_ = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> preferredToneRenderDevice_ = std::make_shared<AudioDeviceDescriptor>();

    unordered_set<shared_ptr<AudioDeviceDescriptor>, AudioDeviceDescriptor::AudioDeviceDescriptorHash,
        AudioDeviceDescriptor::AudioDeviceDescriptorEqual> mediaExcludedDevices_;
    unordered_set<shared_ptr<AudioDeviceDescriptor>, AudioDeviceDescriptor::AudioDeviceDescriptorHash,
        AudioDeviceDescriptor::AudioDeviceDescriptorEqual> callExcludedDevices_;

    std::mutex mutex_;
    shared_mutex mediaExcludedDevicesMutex_;
    shared_mutex callExcludedDevicesMutex_;
    int32_t ownerUid_ = 0;
    std::list<std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>> forcedDeviceMapList_;
    sptr<IStandardAudioPolicyManagerListener> audioClientInfoMgrCallback_;
    sptr<IStandardAudioPolicyManagerListener> audioVKBInfoMgrCallback_;
    void RemoveForcedDeviceMapData(int32_t uid);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_STATE_MANAGER_H

