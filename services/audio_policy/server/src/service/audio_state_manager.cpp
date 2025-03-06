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
    std::lock_guard<std::mutex> lock(mutex_);
    preferredMediaRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredCallRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
    const int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("deviceType: %{public}d, pid: %{public}d", deviceDescriptor->deviceType_, pid);
    if (deviceDescriptor->deviceType_ == DEVICE_TYPE_NONE) {
        if (pid == 0) {
            // clear all
            forcedDeviceMapList_.clear();
        } else if (pid == -1) {
            // clear equal ownerPid_
            RemoveForcedDeviceMapData(ownerPid_);
        } else {
            // clear equal pid
            RemoveForcedDeviceMapData(pid);
        }
    } else {
        std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> currentDeviceMap;
        if (pid == -1) {
            RemoveForcedDeviceMapData(ownerPid_);
            currentDeviceMap = {{ownerPid_, deviceDescriptor}};
        } else {
            RemoveForcedDeviceMapData(pid);
            currentDeviceMap = {{pid, deviceDescriptor}};
        }
        forcedDeviceMapList_.push_back(currentDeviceMap);
    }
}

void AudioStateManager::SetPreferredCallCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredCallCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredRingRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredRingRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredRecordCaptureDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredRecordCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredToneRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredToneRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    if (audioDevUsage == MEDIA_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(mediaExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            auto isPresent = [&desc](const shared_ptr<AudioDeviceDescriptor> &lhs) {
                return lhs->IsSameDeviceDesc(*desc);
            };
            auto it = find_if(mediaExcludedDevices_.begin(), mediaExcludedDevices_.end(), isPresent);
            if (it == mediaExcludedDevices_.end()) {
                mediaExcludedDevices_.push_back(desc);
            }
        }
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(callExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            auto isPresent = [&desc](const shared_ptr<AudioDeviceDescriptor> &lhs) {
                return lhs->IsSameDeviceDesc(*desc);
            };
            auto it = find_if(callExcludedDevices_.begin(), callExcludedDevices_.end(), isPresent);
            if (it == callExcludedDevices_.end()) {
                callExcludedDevices_.push_back(desc);
            }
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
            auto isPresent = [&desc](const shared_ptr<AudioDeviceDescriptor> &lhs) {
                return lhs->IsSameDeviceDesc(*desc);
            };
            auto it = find_if(mediaExcludedDevices_.begin(), mediaExcludedDevices_.end(), isPresent);
            if (it != mediaExcludedDevices_.end()) {
                mediaExcludedDevices_.erase(it);
            }
        }
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(callExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            auto isPresent = [&desc](const shared_ptr<AudioDeviceDescriptor> &lhs) {
                return lhs->IsSameDeviceDesc(*desc);
            };
            auto it = find_if(callExcludedDevices_.begin(), callExcludedDevices_.end(), isPresent);
            if (it != callExcludedDevices_.end()) {
                callExcludedDevices_.erase(it);
            }
        }
    }
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredMediaRenderDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredMediaRenderDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallRenderDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ownerPid_ == 0) {
        if (!forcedDeviceMapList_.empty()) {
            AUDIO_INFO_LOG("deviceType: %{public}d",
                forcedDeviceMapList_.rbegin()->begin()->second->deviceType_);
            return make_shared<AudioDeviceDescriptor>(std::move(forcedDeviceMapList_.rbegin()->begin()->second));
        }
    } else {
        for (auto it = forcedDeviceMapList_.begin(); it != forcedDeviceMapList_.end(); ++it) {
            if (ownerPid_ == it->begin()->first) {
                AUDIO_INFO_LOG("deviceType: %{public}d, ownerPid_: %{public}d", it->begin()->second->deviceType_,
                    ownerPid_);
                return make_shared<AudioDeviceDescriptor>(std::move(it->begin()->second));
            }
        }
        for (auto it = forcedDeviceMapList_.begin(); it != forcedDeviceMapList_.end(); ++it) {
            if (1 == it->begin()->first) {
                AUDIO_INFO_LOG("bluetooth already force selected, deviceType: %{public}d",
                    it->begin()->second->deviceType_);
                return make_shared<AudioDeviceDescriptor>(std::move(it->begin()->second));
            }
        }
    }
    return std::make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallCaptureDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredCallCaptureDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRingRenderDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredRingRenderDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRecordCaptureDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(preferredRecordCaptureDevice_);
    return devDesc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredToneRenderDevice()
{
    std::lock_guard<std::mutex> lock(mutex_);
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

vector<shared_ptr<AudioDeviceDescriptor>> AudioStateManager::GetExcludedDevices(AudioDeviceUsage usage)
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
    AudioDeviceDescriptor audioDeviceDescriptor)
{
    CHECK_AND_RETURN_RET(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES, false);

    auto isPresent = [&audioDeviceDescriptor](const shared_ptr<AudioDeviceDescriptor> &lhs) {
        return lhs->IsSameDeviceDesc(audioDeviceDescriptor);
    };

    auto devDesc = make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor);
    if (audioDevUsage == MEDIA_OUTPUT_DEVICES) {
        shared_lock<shared_mutex> lock(mediaExcludedDevicesMutex_);
        auto it = find_if(mediaExcludedDevices_.begin(), mediaExcludedDevices_.end(), isPresent);
        return it != mediaExcludedDevices_.end();
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        shared_lock<shared_mutex> lock(callExcludedDevicesMutex_);
        auto it = find_if(callExcludedDevices_.begin(), callExcludedDevices_.end(), isPresent);
        return it != callExcludedDevices_.end();
    }

    return false;
}

int32_t AudioStateManager::GetAudioSceneOwnerPid()
{
    return ownerPid_;
}

void AudioStateManager::SetAudioSceneOwnerPid(const int32_t pid)
{
    AUDIO_INFO_LOG("ownerPid_: %{public}d, pid: %{public}d", ownerPid_, pid);
    ownerPid_ = pid;
}

void AudioStateManager::RemoveForcedDeviceMapData(int32_t pid)
{
    if (forcedDeviceMapList_.empty()) {
        return;
    }
    auto it = forcedDeviceMapList_.begin();
    while (it != forcedDeviceMapList_.end()) {
        if (pid == it->begin()->first) {
            it = forcedDeviceMapList_.erase(it);
        } else {
            it++;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
