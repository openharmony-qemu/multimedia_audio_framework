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
#ifndef LOG_TAG
#define LOG_TAG "AudioStateManager"
#endif

#include "audio_state_manager.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"
#include "audio_bundle_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

const int32_t AUDIO_UID = 1041;
const int32_t ANCO_SERVICE_BROKER_UID = 5557;

void AudioStateManager::SetPreferredMediaRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredMediaRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredCallRenderDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
    const int32_t uid, const std::string caller)
{
    std::lock_guard<std::mutex> lock(mutex_);

    bool ret = false;
    int32_t callerUid = uid;
    auto callerPid = IPCSkeleton::GetCallingPid();
    std::string bundleName = AudioBundleManager::GetBundleNameFromUid(callerUid);
    AUDIO_INFO_LOG(
        "deviceType: %{public}d, callerUid: %{public}d, callerPid: %{public}d, ownerUid:%{public}d,\
        bundle name: %{public}s, caller: %{public}s",
        deviceDescriptor->deviceType_, callerUid, callerPid, ownerUid_, bundleName.c_str(), caller.c_str());
    if (audioClientInfoMgrCallback_ != nullptr) {
        audioClientInfoMgrCallback_->OnCheckClientInfo(bundleName, callerUid, callerPid, ret);
    }
    AUDIO_INFO_LOG("check result uid: %{public}d", callerUid);
    if (deviceDescriptor->deviceType_ == DEVICE_TYPE_NONE) {
        if (callerUid == CLEAR_UID) {
            // clear all
            forcedDeviceMapList_.clear();
        } else if (callerUid == SYSTEM_UID || callerUid == ownerUid_) {
            // clear equal ownerUid_ and SYSTEM_UID
            RemoveForcedDeviceMapData(ownerUid_);
            RemoveForcedDeviceMapData(SYSTEM_UID);
        } else {
            // clear equal uid
            RemoveForcedDeviceMapData(callerUid);
        }
    } else {
        std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> currentDeviceMap;
        if (callerUid == SYSTEM_UID && ownerUid_ != 0) {
            forcedDeviceMapList_.clear();
            currentDeviceMap = {{ownerUid_, deviceDescriptor}};
            forcedDeviceMapList_.push_back(currentDeviceMap);
        } else if (callerUid == SYSTEM_UID && ownerUid_ == 0) {
            forcedDeviceMapList_.clear();
        }

        RemoveForcedDeviceMapData(callerUid);
        currentDeviceMap = {{callerUid, deviceDescriptor}};

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
            auto it = mediaExcludedDevices_.find(desc);
            if (it != mediaExcludedDevices_.end()) {
                mediaExcludedDevices_.erase(it);
            }
        }
    } else if (audioDevUsage == CALL_OUTPUT_DEVICES) {
        lock_guard<shared_mutex> lock(callExcludedDevicesMutex_);
        for (const auto &desc : audioDeviceDescriptors) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "Invalid device descriptor");
            auto it = callExcludedDevices_.find(desc);
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
    if (ownerUid_ == 0) {
        if (!forcedDeviceMapList_.empty()) {
            AUDIO_INFO_LOG("ownerUid_: 0, deviceType: %{public}d, Uid: %{public}d",
                forcedDeviceMapList_.rbegin()->begin()->second->deviceType_,
                forcedDeviceMapList_.rbegin()->begin()->first);
            return make_shared<AudioDeviceDescriptor>(std::move(forcedDeviceMapList_.rbegin()->begin()->second));
        }
    } else {
        for (auto it = forcedDeviceMapList_.begin(); it != forcedDeviceMapList_.end(); ++it) {
            if (ownerUid_ == it->begin()->first) {
                AUDIO_INFO_LOG("deviceType: %{public}d, ownerUid_: %{public}d", it->begin()->second->deviceType_,
                    ownerUid_);
                return make_shared<AudioDeviceDescriptor>(std::move(it->begin()->second));
            }
        }
        for (auto it = forcedDeviceMapList_.begin(); it != forcedDeviceMapList_.end(); ++it) {
            if (SYSTEM_UID == it->begin()->first) {
                AUDIO_INFO_LOG("bluetooth already force selected, deviceType: %{public}d",
                    it->begin()->second->deviceType_);
                return make_shared<AudioDeviceDescriptor>(std::move(it->begin()->second));
            }
        }
    }
    return std::make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallRenderDeviceForUid(const int32_t clientUid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!forcedDeviceMapList_.empty(), std::make_shared<AudioDeviceDescriptor>(),
        "ForcedDeviceMapList_ is empty");
    for (auto it = forcedDeviceMapList_.begin(); it != forcedDeviceMapList_.end(); ++it) {
        if (clientUid == it->begin()->first) {
            CHECK_AND_CONTINUE(it->begin()->second != nullptr);
            AUDIO_INFO_LOG("deviceType: %{public}d, clientUid: %{public}d", it->begin()->second->deviceType_,
                clientUid);
            return make_shared<AudioDeviceDescriptor>(it->begin()->second);
        }
    }
    for (auto it = forcedDeviceMapList_.begin(); it != forcedDeviceMapList_.end(); ++it) {
        if (SYSTEM_UID == it->begin()->first) {
            CHECK_AND_CONTINUE(it->begin()->second != nullptr);
            AUDIO_INFO_LOG("system force selected, deviceType: %{public}d",
                it->begin()->second->deviceType_);
            return make_shared<AudioDeviceDescriptor>(it->begin()->second);
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

void AudioStateManager::SetPreferredRecognitionCaptureDevice(const shared_ptr<AudioDeviceDescriptor> &desc)
{
    std::lock_guard<std::mutex> lock(mutex_);
    preferredRecognitionCaptureDevice_ = desc;
}

shared_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRecognitionCaptureDevice()
{
    lock_guard<std::mutex> lock(mutex_);
    return preferredRecognitionCaptureDevice_;
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
        for (const auto &desc : callExcludedDevices_) {
            devices.push_back(make_shared<AudioDeviceDescriptor>(*desc));
        }
    }

    return devices;
}

bool AudioStateManager::IsExcludedDevice(AudioDeviceUsage audioDevUsage,
    const shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptor)
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

void AudioStateManager::SetAudioSceneOwnerUid(const int32_t uid)
{
    AUDIO_INFO_LOG("ownerUid_: %{public}d, uid: %{public}d", ownerUid_, uid);
    ownerUid_ = uid;
    if (uid == AUDIO_UID) {
        ownerUid_ = ANCO_SERVICE_BROKER_UID;
    }
}

int32_t AudioStateManager::SetAudioClientInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioClientInfoMgrCallback_ = callback;
    return 0;
}

int32_t AudioStateManager::SetAudioVKBInfoMgrCallback(sptr<IStandardAudioPolicyManagerListener> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioVKBInfoMgrCallback_ = callback;
    AUDIO_INFO_LOG("VKB audioVKBInfoMgrCallback_ is nullptr:%{public}s",
        audioVKBInfoMgrCallback_ == nullptr ? "T" : "F");
    return 0;
}

int32_t AudioStateManager::CheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (audioVKBInfoMgrCallback_ != nullptr) {
        audioVKBInfoMgrCallback_->OnCheckVKBInfo(bundleName, isValid);
    }
    AUDIO_INFO_LOG("isVKB:%{public}s", isValid ? "T" : "F");
    return 0;
}

void AudioStateManager::RemoveForcedDeviceMapData(int32_t uid)
{
    if (forcedDeviceMapList_.empty()) {
        return;
    }
    auto it = forcedDeviceMapList_.begin();
    while (it != forcedDeviceMapList_.end()) {
        if (uid == it->begin()->first) {
            it = forcedDeviceMapList_.erase(it);
        } else {
            it++;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
