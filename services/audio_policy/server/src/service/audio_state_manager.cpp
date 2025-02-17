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

#include "audio_state_manager.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

void AudioStateManager::SetPreferredMediaRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    preferredMediaRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredCallRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    preferredCallRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredCallCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredCallCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredRingRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    preferredRingRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredRecordCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    preferredRecordCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredToneRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    preferredToneRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    if (audioDevUsage == MEDIA_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(mediaExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            mediaExcludedDevices_.insert(desc);
        }
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(callExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            callExcludedDevices_.insert(desc);
        }
    }
}

void AudioStateManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    if (audioDevUsage == MEDIA_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(mediaExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            mediaExcludedDevices_.erase(desc);
        }
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(callExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            callExcludedDevices_.erase(desc);
        }
    }
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredMediaRenderDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredMediaRenderDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallRenderDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredCallRenderDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallCaptureDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredCallCaptureDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRingRenderDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredRingRenderDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRecordCaptureDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredRecordCaptureDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredToneRenderDevice()
{
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredToneRenderDevice_);
    return devDesc;
}

void AudioStateManager::UpdatePreferredMediaRenderDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredMediaRenderDevice_ != nullptr, "preferredMediaRenderDevice_ is nullptr");
    preferredMediaRenderDevice_->connectState_ = state;
}

void AudioStateManager::UpdatePreferredCallRenderDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredCallRenderDevice_ != nullptr, "preferredCallRenderDevice_ is nullptr");
    preferredCallRenderDevice_->connectState_ = state;
}

void AudioStateManager::UpdatePreferredCallCaptureDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredCallCaptureDevice_ != nullptr, "preferredCallCaptureDevice_ is nullptr");
    preferredCallCaptureDevice_->connectState_ = state;
}

void AudioStateManager::UpdatePreferredRecordCaptureDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredRecordCaptureDevice_ != nullptr, "preferredRecordCaptureDevice_ is nullptr");
    preferredRecordCaptureDevice_->connectState_ = state;
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioStateManager::GetExcludedOutputDevices(AudioDeviceUsage usage)
{
    vector<shared_ptr<AudioDeviceDescriptor>> devices;
    if (usage == MEDIA_OUTPUT_DEVICES) {
        shared_lock<shared_mutex> lock(mediaExcludedDevicesMutex_);
        for (const auto &desc : mediaExcludedDevices_) {
            devices.push_back(make_shared<AudioDeviceDescriptor>(*desc));
        }
    } else if (usage == CALL_OUTPUT_DEVICES) {
        shared_lock<shared_mutex> lock(callExcludedDevicesMutex_);
        vector<shared_ptr<AudioDeviceDescriptor>> devices;
        for (const auto &desc : callExcludedDevices_) {
            devices.push_back(make_shared<AudioDeviceDescriptor>(*desc));
        }
    }

    return devices;
}

bool AudioStateManager::IsExcludedDevice(AudioDeviceUsage audioDevUsage,
    shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptor)
{
    CHECK_AND_RETURN_RET(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES, false);

    if (audioDevUsage == MEDIA_OUTPUT_DEVICES) {
        shared_lock<shared_mutex> lock(mediaExcludedDevicesMutex_);
        return mediaExcludedDevices_.contains(audioDeviceDescriptor);
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        shared_lock<shared_mutex> lock(callExcludedDevicesMutex_);
        return callExcludedDevices_.contains(audioDeviceDescriptor);
    }

    return false;
}
} // namespace AudioStandard
} // namespace OHOS
