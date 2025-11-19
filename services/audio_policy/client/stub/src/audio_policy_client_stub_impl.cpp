/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyClientStubImpl"
#endif

#include <memory>

#include "audio_policy_client_stub_impl.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t RSS_UID = 1096;
static const int32_t FOCUS_INFO_VALID_SIZE = 128;
static const int32_t DEVICE_CHANGE_VALID_SIZE = 128;
static const int32_t PREFERRED_DEVICE_VALID_SIZE = 128;
static const int32_t STATE_VALID_SIZE = 1024;
static const int32_t MIC_BLOCKED_VALID_SIZE = 128;

int32_t AudioPolicyClientStubImpl::AddVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(volumeKeyEventMutex_);
    volumeKeyEventCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(volumeKeyEventMutex_);
    if (cb == nullptr) {
        volumeKeyEventCallbackList_.clear();
        return SUCCESS;
    }
    auto it = find_if(volumeKeyEventCallbackList_.begin(), volumeKeyEventCallbackList_.end(),
        [&cb](const std::weak_ptr<VolumeKeyEventCallback>& elem) {
            return elem.lock() == cb;
        });
    if (it != volumeKeyEventCallbackList_.end()) {
        volumeKeyEventCallbackList_.erase(it);
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetVolumeKeyEventCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(volumeKeyEventMutex_);
    return volumeKeyEventCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnVolumeKeyEvent(const VolumeEvent &volumeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(volumeKeyEventMutex_);
    for (auto it = volumeKeyEventCallbackList_.begin(); it != volumeKeyEventCallbackList_.end(); ++it) {
        std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallback = (*it).lock();
        if (volumeKeyEventCallback != nullptr) {
            volumeKeyEventCallback->OnVolumeKeyEvent(volumeEvent);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddVolumeDegreeCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(volumeDegreeEventMutex_);
    volumeDegreeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveVolumeDegreeCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(volumeDegreeEventMutex_);
    if (cb == nullptr) {
        volumeDegreeCallbackList_.clear();
        return SUCCESS;
    }
    auto it = find_if(volumeDegreeCallbackList_.begin(), volumeDegreeCallbackList_.end(),
        [&cb](const std::weak_ptr<VolumeKeyEventCallback>& elem) {
            return elem.lock() == cb;
        });
    if (it != volumeDegreeCallbackList_.end()) {
        volumeDegreeCallbackList_.erase(it);
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetVolumeDegreeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(volumeDegreeEventMutex_);
    return volumeDegreeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnVolumeDegreeEvent(const VolumeEvent &volumeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(volumeDegreeEventMutex_);
    for (auto it = volumeDegreeCallbackList_.begin(); it != volumeDegreeCallbackList_.end();) {
        std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallback = (*it).lock();
        if (volumeKeyEventCallback != nullptr) {
            volumeKeyEventCallback->OnVolumeDegreeEvent(volumeEvent);
            ++it;
        } else {
            it = volumeDegreeCallbackList_.erase(it);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddSystemVolumeChangeCallback(const std::shared_ptr<SystemVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(systemVolumeChangeMutex_);
    systemVolumeChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveSystemVolumeChangeCallback(
    const std::shared_ptr<SystemVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(systemVolumeChangeMutex_);
    if (cb == nullptr) {
        systemVolumeChangeCallbackList_.clear();
        return SUCCESS;
    }
    auto it = find_if(systemVolumeChangeCallbackList_.begin(), systemVolumeChangeCallbackList_.end(),
        [&cb](const std::weak_ptr<SystemVolumeChangeCallback>& elem) {
            return elem.lock() == cb;
        });
    if (it != systemVolumeChangeCallbackList_.end()) {
        systemVolumeChangeCallbackList_.erase(it);
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetSystemVolumeChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(systemVolumeChangeMutex_);
    return systemVolumeChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnSystemVolumeChange(const VolumeEvent &volumeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(systemVolumeChangeMutex_);
    for (auto it = systemVolumeChangeCallbackList_.begin(); it != systemVolumeChangeCallbackList_.end(); ++it) {
        std::shared_ptr<SystemVolumeChangeCallback> systemVolumeChangeCallback = (*it).lock();
        if (systemVolumeChangeCallback != nullptr) {
            systemVolumeChangeCallback->OnSystemVolumeChange(volumeEvent);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    focusInfoChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveFocusInfoChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    focusInfoChangeCallbackList_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnAudioFocusInfoChange(
    const std::vector<std::map<AudioInterrupt, int32_t>> &focusInfoList)
{
    int32_t size = static_cast<int32_t>(focusInfoList.size());
    CHECK_AND_RETURN_RET_LOG(size < FOCUS_INFO_VALID_SIZE, ERR_INVALID_PARAM, "get invalid size : %{public}d", size);
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> newFocusInfoList;
    for (const auto& map : focusInfoList) {
        for (const auto& [key, value] : map) {
            newFocusInfoList.emplace_back(key, static_cast<AudioFocuState>(value));
        }
    }

    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusInfoChange(newFocusInfoList);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnAudioFocusRequested(const AudioInterrupt &requestFocus)
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusRequested(requestFocus);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus)
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusAbandoned(abandonFocus);
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetFocusInfoChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    return focusInfoChangeCallbackList_.size();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyClientStubImpl::DeviceFilterByFlag(DeviceFlag flag,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descRet;
    DeviceRole role = DEVICE_ROLE_NONE;
    switch (flag) {
        case DeviceFlag::ALL_DEVICES_FLAG:
            for (std::shared_ptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ == LOCAL_NETWORK_ID) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        case DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG:
            for (std::shared_ptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ != LOCAL_NETWORK_ID) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        case DeviceFlag::ALL_L_D_DEVICES_FLAG:
            descRet = desc;
            break;
        case DeviceFlag::OUTPUT_DEVICES_FLAG:
        case DeviceFlag::INPUT_DEVICES_FLAG:
            role = flag == INPUT_DEVICES_FLAG ? INPUT_DEVICE : OUTPUT_DEVICE;
            for (std::shared_ptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ == LOCAL_NETWORK_ID && var->deviceRole_ == role) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        case DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG:
            role = flag == DISTRIBUTED_INPUT_DEVICES_FLAG ? INPUT_DEVICE : OUTPUT_DEVICE;
            for (std::shared_ptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ != LOCAL_NETWORK_ID && var->deviceRole_ == role) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        default:
            break;
    }
    return descRet;
}

int32_t AudioPolicyClientStubImpl::AddDeviceChangeCallback(const DeviceFlag &flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(deviceChangeMutex_);
    deviceChangeCallbackList_.push_back(std::make_pair(flag, cb));
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveDeviceChangeCallback(DeviceFlag flag,
    std::shared_ptr<AudioManagerDeviceChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(deviceChangeMutex_);
    auto iter = deviceChangeCallbackList_.begin();
    while (iter != deviceChangeCallbackList_.end()) {
        if ((iter->first & flag) && (iter->second == cb || cb == nullptr)) {
            AUDIO_INFO_LOG("remove device change cb flag:%{public}d", flag);
            iter = deviceChangeCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetDeviceChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(deviceChangeMutex_);
    return deviceChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnDeviceChange(const DeviceChangeAction &dca)
{
    int32_t size = static_cast<int32_t>(dca.deviceDescriptors.size());
    CHECK_AND_RETURN_RET_LOG(size < DEVICE_CHANGE_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::lock_guard<std::mutex> lockCbMap(deviceChangeMutex_);
    DeviceChangeAction deviceChangeAction;
    deviceChangeAction.type = dca.type;
    for (auto it = deviceChangeCallbackList_.begin(); it != deviceChangeCallbackList_.end(); ++it) {
        deviceChangeAction.flag = it->first;
        deviceChangeAction.deviceDescriptors = DeviceFilterByFlag(it->first, dca.deviceDescriptors);
        if (it->second && deviceChangeAction.deviceDescriptors.size() > 0) {
            it->second->OnDeviceChange(deviceChangeAction);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddDeviceInfoUpdateCallback(
    const std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(deviceInfoUpdateMutex_);
    deviceInfoUpdateCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveDeviceInfoUpdateCallback(
    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(deviceInfoUpdateMutex_);
    auto iter = deviceInfoUpdateCallbackList_.begin();
    while (iter != deviceInfoUpdateCallbackList_.end()) {
        if (*iter == cb || cb == nullptr) {
            AUDIO_INFO_LOG("remove device info update cb");
            iter = deviceInfoUpdateCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetDeviceInfoUpdateCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(deviceInfoUpdateMutex_);
    return deviceInfoUpdateCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnDeviceInfoUpdate(const DeviceChangeAction &dca)
{
    std::lock_guard<std::mutex> lockCbMap(deviceInfoUpdateMutex_);
    for (auto it = deviceInfoUpdateCallbackList_.begin(); it != deviceInfoUpdateCallbackList_.end(); ++it) {
        if (*it && dca.deviceDescriptors.size() > 0) {
            (*it)->OnDeviceInfoUpdate(dca);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnMicrophoneBlocked(const MicrophoneBlockedInfo &blockedInfo)
{
    int32_t size = static_cast<int32_t>(blockedInfo.devices.size());
    CHECK_AND_RETURN_RET_LOG(size < MIC_BLOCKED_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::lock_guard<std::mutex> lockCbMap(microphoneBlockedMutex_);
    MicrophoneBlockedInfo microphoneBlockedInfo;
    microphoneBlockedInfo.blockStatus = blockedInfo.blockStatus;
    for (auto it = microphoneBlockedCallbackList_.begin(); it != microphoneBlockedCallbackList_.end(); ++it) {
        microphoneBlockedInfo.devices = blockedInfo.devices;
        if (it->second && microphoneBlockedInfo.devices.size() > 0) {
            it->second->OnMicrophoneBlocked(microphoneBlockedInfo);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddMicrophoneBlockedCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(microphoneBlockedMutex_);
    microphoneBlockedCallbackList_.push_back(std::make_pair(clientId, cb));
    AUDIO_INFO_LOG("add mic blocked cb clientId:%{public}d", clientId);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveMicrophoneBlockedCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(microphoneBlockedMutex_);
    auto iter = microphoneBlockedCallbackList_.begin();
    while (iter != microphoneBlockedCallbackList_.end()) {
        if ((iter->first == clientId) && (iter->second == cb || cb == nullptr)) {
            AUDIO_INFO_LOG("remove mic blocked cb flag:%{public}d", clientId);
            iter = microphoneBlockedCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetMicrophoneBlockedCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(microphoneBlockedMutex_);
    return microphoneBlockedCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::AddAudioSceneChangedCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(audioSceneChangedMutex_);
    audioSceneChangedCallbackList_.push_back(cb);
    AUDIO_INFO_LOG("add audio scene change clientId:%{public}d", clientId);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSceneChangedCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(audioSceneChangedMutex_);
    auto iter = audioSceneChangedCallbackList_.begin();
    while (iter != audioSceneChangedCallbackList_.end()) {
        if (*iter == cb) {
            iter = audioSceneChangedCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetAudioSceneChangedCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(audioSceneChangedMutex_);
    return audioSceneChangedCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnAudioSceneChange(int32_t audioScene)
{
    CHECK_AND_RETURN_RET_LOG(audioScene < AUDIO_SCENE_MAX && audioScene > AUDIO_SCENE_INVALID,
        ERR_INVALID_PARAM, "get invalid audioScene : %{public}d", audioScene);

    std::lock_guard<std::mutex> lockCbMap(audioSceneChangedMutex_);
    for (const auto &callback : audioSceneChangedCallbackList_) {
        CHECK_AND_CONTINUE(callback != nullptr);
        callback->OnAudioSceneChange(static_cast<AudioScene>(audioScene));
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAllActiveVolumeTypeChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(activeVolumeTypeChangeMutex_);
    activeVolumeTypeChangeCallbackList_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveActiveVolumeTypeChangeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(activeVolumeTypeChangeMutex_);
    if (cb == nullptr) {
        activeVolumeTypeChangeCallbackList_.clear();
        return SUCCESS;
    }
    auto it = find_if(activeVolumeTypeChangeCallbackList_.begin(), activeVolumeTypeChangeCallbackList_.end(),
        [&cb](const std::weak_ptr<AudioManagerActiveVolumeTypeChangeCallback>& elem) {
            return elem.lock() == cb;
        });
    if (it != activeVolumeTypeChangeCallbackList_.end()) {
        activeVolumeTypeChangeCallbackList_.erase(it);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddActiveVolumeTypeChangeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(activeVolumeTypeChangeMutex_);
    activeVolumeTypeChangeCallbackList_.push_back(cb);
    AUDIO_INFO_LOG("Add activeVolumeTypeChangeCallback P : %{private}p", cb.get());
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddSelfAppVolumeChangeCallback(int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(selfAppVolumeChangeMutex_);
    for (auto iter : selfAppVolumeChangeCallback_) {
        if (iter.first == appUid && iter.second.get() == cb.get()) {
            selfAppVolumeChangeCallbackNum_[appUid]++;
            AUDIO_INFO_LOG("selfAppVolumeChangeCallback_ No need pushback");
            return SUCCESS;
        }
    }
    selfAppVolumeChangeCallbackNum_[appUid]++;
    selfAppVolumeChangeCallback_.push_back({appUid, cb});
    AUDIO_INFO_LOG("Add selfAppVolumeChangeCallback appUid : %{public}d ", appUid);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAllSelfAppVolumeChangeCallback(int32_t appUid)
{
    std::lock_guard<std::mutex> lockCbMap(selfAppVolumeChangeMutex_);
    if (selfAppVolumeChangeCallbackNum_[appUid] != 0) {
        selfAppVolumeChangeCallbackNum_[appUid] = 0;
        auto iter = selfAppVolumeChangeCallback_.begin();
        while (iter != selfAppVolumeChangeCallback_.end()) {
            if (iter->first == appUid) {
                iter = selfAppVolumeChangeCallback_.erase(iter);
            } else {
                iter++;
            }
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveSelfAppVolumeChangeCallback(int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(selfAppVolumeChangeMutex_);
    auto iter = selfAppVolumeChangeCallback_.begin();
    while (iter != selfAppVolumeChangeCallback_.end()) {
        if (iter->first != appUid || iter->second.get() != cb.get()) {
            iter++;
            continue;
        }
        selfAppVolumeChangeCallbackNum_[appUid]--;
        if (selfAppVolumeChangeCallbackNum_[appUid] == 0) {
            iter = selfAppVolumeChangeCallback_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAllAppVolumeChangeForUidCallback()
{
    std::lock_guard<std::mutex> lockCbMap(appVolumeChangeForUidMutex_);
    appVolumeChangeForUidCallback_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAppVolumeChangeForUidCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(appVolumeChangeForUidMutex_);
    auto iter = appVolumeChangeForUidCallback_.begin();
    while (iter != appVolumeChangeForUidCallback_.end()) {
        if (iter->second.get() != cb.get()) {
            iter++;
            continue;
        }
        iter = appVolumeChangeForUidCallback_.erase(iter);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddAppVolumeChangeForUidCallback(const int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(appVolumeChangeForUidMutex_);
    for (auto iter : appVolumeChangeForUidCallback_) {
        if (iter.first == appUid && iter.second.get() == cb.get()) {
            appVolumeChangeForUidCallbackNum[appUid]++;
            AUDIO_INFO_LOG("appVolumeChangeForUidCallback_ No need pushback");
            return SUCCESS;
        }
    }
    appVolumeChangeForUidCallbackNum[appUid]++;
    appVolumeChangeForUidCallback_.push_back({appUid, cb});
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(ringerModeMutex_);
    ringerModeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveRingerModeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(ringerModeMutex_);
    ringerModeCallbackList_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(ringerModeMutex_);
    auto iter = ringerModeCallbackList_.begin();
    while (iter != ringerModeCallbackList_.end()) {
        if (*iter == cb) {
            iter = ringerModeCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetActiveVolumeTypeChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(activeVolumeTypeChangeMutex_);
    return activeVolumeTypeChangeCallbackList_.size();
}

size_t AudioPolicyClientStubImpl::GetRingerModeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(ringerModeMutex_);
    return ringerModeCallbackList_.size();
}

size_t AudioPolicyClientStubImpl::GetAppVolumeChangeCallbackForUidSize() const
{
    std::lock_guard<std::mutex> lockCbMap(appVolumeChangeForUidMutex_);
    return appVolumeChangeForUidCallback_.size();
}

size_t AudioPolicyClientStubImpl::GetSelfAppVolumeChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(selfAppVolumeChangeMutex_);
    return selfAppVolumeChangeCallback_.size();
}

int32_t AudioPolicyClientStubImpl::OnRingerModeUpdated(int32_t ringerMode)
{
    std::lock_guard<std::mutex> lockCbMap(ringerModeMutex_);
    for (auto it = ringerModeCallbackList_.begin(); it != ringerModeCallbackList_.end(); ++it) {
        (*it)->OnRingerModeUpdated(static_cast<AudioRingerMode>(ringerMode));
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnActiveVolumeTypeChanged(int32_t volumeType)
{
    std::lock_guard<std::mutex> lockCbMap(activeVolumeTypeChangeMutex_);
    for (auto it = activeVolumeTypeChangeCallbackList_.begin(); it != activeVolumeTypeChangeCallbackList_.end(); ++it) {
        std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> activeVolumeTypeCallback = (*it).lock();
        if (activeVolumeTypeCallback != nullptr) {
            activeVolumeTypeCallback->OnActiveVolumeTypeChanged(static_cast<AudioVolumeType>(volumeType));
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent)
{
    {
        std::lock_guard<std::mutex> lockCbMap(appVolumeChangeForUidMutex_);
        for (auto iter : appVolumeChangeForUidCallback_) {
            if (iter.first != appUid) {
                continue;
            }
            iter.second->OnAppVolumeChangedForUid(appUid, volumeEvent);
        }
    }
    {
        std::lock_guard<std::mutex> lockCbMap(selfAppVolumeChangeMutex_);
        for (auto iter : selfAppVolumeChangeCallback_) {
            if (iter.first != appUid) {
                continue;
            }
            iter.second->OnSelfAppVolumeChanged(volumeEvent);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &cb)
{
    AUDIO_INFO_LOG("AddAudioSessionCallback in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    audioSessionCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionCallback()
{
    AUDIO_INFO_LOG("RemoveAudioSessionCallback all in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    audioSessionCallbackList_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &cb)
{
    AUDIO_INFO_LOG("RemoveAudioSessionCallback one in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    auto iter = audioSessionCallbackList_.begin();
    while (iter != audioSessionCallbackList_.end()) {
        if (*iter == cb) {
            iter = audioSessionCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetAudioSessionCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    return audioSessionCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnAudioSessionDeactive(int32_t deactiveEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionDeactive in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    AudioSessionDeactiveEvent newDeactiveEvent;
    newDeactiveEvent.deactiveReason = static_cast<AudioSessionDeactiveReason>(deactiveEvent);
    for (auto it = audioSessionCallbackList_.begin(); it != audioSessionCallbackList_.end(); ++it) {
        CHECK_AND_CONTINUE((*it) != nullptr);
        (*it)->OnAudioSessionDeactive(newDeactiveEvent);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddAudioSessionStateCallback(
    const std::shared_ptr<AudioSessionStateChangedCallback> &cb)
{
    AUDIO_INFO_LOG("AddAudioSessionStateCallback in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionStateMutex_);
    audioSessionStateCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionStateCallback()
{
    AUDIO_INFO_LOG("RemoveAudioSessionStateCallback all in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionStateMutex_);
    audioSessionStateCallbackList_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionStateCallback(
    const std::shared_ptr<AudioSessionStateChangedCallback> &cb)
{
    AUDIO_INFO_LOG("RemoveAudioSessionStateCallback one in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionStateMutex_);
    auto it = find_if(audioSessionStateCallbackList_.begin(), audioSessionStateCallbackList_.end(),
        [&cb](const std::weak_ptr<AudioSessionStateChangedCallback>& elem) {
            return elem.lock() == cb;
        });
    if (it != audioSessionStateCallbackList_.end()) {
        audioSessionStateCallbackList_.erase(it);
        AUDIO_INFO_LOG("RemoveAudioSessionStateCallback remove cb succeed");
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetAudioSessionStateCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(audioSessionStateMutex_);
    return audioSessionStateCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnAudioSessionStateChanged(int32_t stateChangeHint)
{
    AudioSessionStateChangedEvent stateChangedEvent;
    stateChangedEvent.stateChangeHint = static_cast<AudioSessionStateChangeHint>(stateChangeHint);
    AUDIO_INFO_LOG("OnAudioSessionStateChanged in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionStateMutex_);
    for (auto it = audioSessionStateCallbackList_.begin(); it != audioSessionStateCallbackList_.end(); ++it) {
        std::shared_ptr<AudioSessionStateChangedCallback> audioSessionStateChangedCallback = (*it).lock();
        if (audioSessionStateChangedCallback != nullptr) {
            audioSessionStateChangedCallback->OnAudioSessionStateChanged(stateChangedEvent);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddAudioSessionDeviceCallback(
    const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &cb)
{
    AUDIO_INFO_LOG("AddAudioSessionDeviceCallback in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionDeviceMutex_);
    audioSessionDeviceCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionDeviceCallback()
{
    AUDIO_INFO_LOG("RemoveAudioSessionDeviceCallback all in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionDeviceMutex_);
    audioSessionDeviceCallbackList_.clear();
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionDeviceCallback(
    const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &cb)
{
    AUDIO_INFO_LOG("RemoveAudioSessionDeviceCallback one in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionDeviceMutex_);
    auto it = find_if(audioSessionDeviceCallbackList_.begin(), audioSessionDeviceCallbackList_.end(),
        [&cb](const std::weak_ptr<AudioSessionCurrentDeviceChangedCallback>& elem) {
            return elem.lock() == cb;
        });
    if (it != audioSessionDeviceCallbackList_.end()) {
        audioSessionDeviceCallbackList_.erase(it);
        AUDIO_INFO_LOG("RemoveAudioSessionDeviceCallback remove cb succeed");
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetAudioSessionDeviceCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(audioSessionDeviceMutex_);
    return audioSessionDeviceCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnAudioSessionCurrentDeviceChanged(
    const CurrentOutputDeviceChangedEvent &deviceChangedEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionCurrentDeviceChanged in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionDeviceMutex_);
    for (auto it = audioSessionDeviceCallbackList_.begin(); it != audioSessionDeviceCallbackList_.end(); ++it) {
        std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> deviceChangedCallback = (*it).lock();
        if (deviceChangedCallback != nullptr) {
            deviceChangedCallback->OnAudioSessionCurrentDeviceChanged(deviceChangedEvent);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddAudioSessionInputDeviceCallback(
    const std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback> &cb)
{
    AUDIO_INFO_LOG("AddAudioSessionInputDeviceCallback in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionInputDeviceMutex_);
    audioSessionInputDeviceCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioSessionInputDeviceCallback(
    const std::optional<std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback>> &cb)
{
    AUDIO_INFO_LOG("RemoveAudioSessionInputDeviceCallback in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionInputDeviceMutex_);
    if (cb.has_value()) {
        auto it = find_if(audioSessionInputDeviceCallbackList_.begin(), audioSessionInputDeviceCallbackList_.end(),
            [&cb](const std::weak_ptr<AudioSessionCurrentInputDeviceChangedCallback>& elem) {
                return elem.lock() == cb.value();
            });
        if (it != audioSessionInputDeviceCallbackList_.end()) {
            audioSessionInputDeviceCallbackList_.erase(it);
            AUDIO_INFO_LOG("RemoveAudioSessionInputDeviceCallback remove cb succeed");
        }
    } else {
        audioSessionInputDeviceCallbackList_.clear();
    }
    
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetAudioSessionInputDeviceCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(audioSessionInputDeviceMutex_);
    return audioSessionInputDeviceCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnAudioSessionCurrentInputDeviceChanged(
    const CurrentInputDeviceChangedEvent &deviceChangedEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionCurrentInputDeviceChanged in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionInputDeviceMutex_);
    for (auto it = audioSessionInputDeviceCallbackList_.begin();
        it != audioSessionInputDeviceCallbackList_.end(); ++it) {
        std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback> deviceChangedCallback = (*it).lock();
        if (deviceChangedCallback != nullptr) {
            CurrentInputDeviceChangedEvent change = deviceChangedEvent;
            AudioDeviceDescriptor::MapInputDeviceType(change.devices);
            deviceChangedCallback->OnAudioSessionCurrentInputDeviceChanged(change);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(micStateChangeMutex_);
    micStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}
int32_t AudioPolicyClientStubImpl::RemoveMicStateChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(micStateChangeMutex_);
    micStateChangeCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetMicStateChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(micStateChangeMutex_);
    return micStateChangeCallbackList_.size();
}

bool AudioPolicyClientStubImpl::HasMicStateChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(micStateChangeMutex_);
    if (micStateChangeCallbackList_.empty()) {
        AUDIO_INFO_LOG("MicStateChangeCallback list is empty.");
        return false;
    }
    return true;
}

int32_t AudioPolicyClientStubImpl::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(micStateChangeMutex_);
    for (auto it = micStateChangeCallbackList_.begin(); it != micStateChangeCallbackList_.end(); ++it) {
        (*it)->OnMicStateUpdated(micStateChangeEvent);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddPreferredOutputDeviceChangeCallback(const AudioRendererInfo &rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(pOutputDeviceChangeMutex_);
    preferredOutputDeviceCallbackMap_[rendererInfo.streamUsage].push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemovePreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(pOutputDeviceChangeMutex_);
    if (cb == nullptr) {
        preferredOutputDeviceCallbackMap_.clear();
        return SUCCESS;
    }
    for (auto &it : preferredOutputDeviceCallbackMap_) {
        auto iter = find(it.second.begin(), it.second.end(), cb);
        if (iter != it.second.end()) {
            it.second.erase(iter);
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetPreferredOutputDeviceChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(pOutputDeviceChangeMutex_);
    return preferredOutputDeviceCallbackMap_.size();
}

int32_t AudioPolicyClientStubImpl::OnPreferredOutputDeviceUpdated(const AudioRendererInfo &rendererInfo,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    int32_t size = static_cast<int32_t>(desc.size());
    CHECK_AND_RETURN_RET_LOG(size < PREFERRED_DEVICE_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::lock_guard<std::mutex> lockCbMap(pOutputDeviceChangeMutex_);
    auto it = preferredOutputDeviceCallbackMap_.find(rendererInfo.streamUsage);
    CHECK_AND_RETURN_RET_LOG(it != preferredOutputDeviceCallbackMap_.end(),
        ERR_CALLBACK_NOT_REGISTERED, "streamUsage not found");
    for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
        CHECK_AND_CONTINUE_LOG(iter != it->second.end() && (*iter) != nullptr, "iter is null");
    (*iter)->OnPreferredOutputDeviceUpdated(desc);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddPreferredInputDeviceChangeCallback(const AudioCapturerInfo &capturerInfo,
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(pInputDeviceChangeMutex_);
    preferredInputDeviceCallbackMap_[capturerInfo.sourceType].push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemovePreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(pInputDeviceChangeMutex_);
    if (cb == nullptr) {
        preferredInputDeviceCallbackMap_.clear();
        return SUCCESS;
    }
    for (auto &it : preferredInputDeviceCallbackMap_) {
        auto iter = find(it.second.begin(), it.second.end(), cb);
        if (iter != it.second.end()) {
            it.second.erase(iter);
        }
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetPreferredInputDeviceChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(pInputDeviceChangeMutex_);
    return preferredInputDeviceCallbackMap_.size();
}

int32_t AudioPolicyClientStubImpl::OnPreferredInputDeviceUpdated(const AudioCapturerInfo &capturerInfo,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    int32_t size = static_cast<int32_t>(desc.size());
    CHECK_AND_RETURN_RET_LOG(size < PREFERRED_DEVICE_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::lock_guard<std::mutex> lockCbMap(pInputDeviceChangeMutex_);
    auto it = preferredInputDeviceCallbackMap_.find(capturerInfo.sourceType);
    CHECK_AND_RETURN_RET_LOG(it != preferredInputDeviceCallbackMap_.end(),
        ERR_CALLBACK_NOT_REGISTERED, "sourceType not found");
    for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
        CHECK_AND_CONTINUE_LOG(iter != it->second.end() && (*iter) != nullptr, "iter is null");
        (*iter)->OnPreferredInputDeviceUpdated(desc);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddRendererStateChangeCallback(
    const std::shared_ptr<AudioRendererStateChangeCallback> &cb)
{
    CHECK_AND_RETURN_RET_LOG(cb, ERR_INVALID_PARAM, "cb is null");

    std::lock_guard<std::mutex> lockCbMap(rendererStateChangeMutex_);
    rendererStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveRendererStateChangeCallback(
    const std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> &callbacks)
{
    std::lock_guard<std::mutex> lockCbMap(rendererStateChangeMutex_);
    for (const auto &cb : callbacks) {
        rendererStateChangeCallbackList_.erase(
            std::remove(rendererStateChangeCallbackList_.begin(),
            rendererStateChangeCallbackList_.end(), cb), rendererStateChangeCallbackList_.end());
    }

    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveRendererStateChangeCallback(
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(rendererStateChangeMutex_);
    rendererStateChangeCallbackList_.erase(
        std::remove(rendererStateChangeCallbackList_.begin(),
        rendererStateChangeCallbackList_.end(), callback), rendererStateChangeCallbackList_.end());

    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetRendererStateChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(rendererStateChangeMutex_);
    return rendererStateChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::AddDeviceChangeWithInfoCallback(
    const uint32_t sessionId, const std::weak_ptr<DeviceChangeWithInfoCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
    deviceChangeWithInfoCallbackMap_[sessionId] = cb;
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveDeviceChangeWithInfoCallback(const uint32_t sessionId)
{
    std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
    deviceChangeWithInfoCallbackMap_.erase(sessionId);
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetDeviceChangeWithInfoCallbackkSize() const
{
    std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
    return deviceChangeWithInfoCallbackMap_.size();
}

int32_t AudioPolicyClientStubImpl::OnRendererDeviceChange(uint32_t sessionId,
    const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt &reason)
{
    Trace trace("AudioPolicyClientStubImpl::OnRendererDeviceChange");
    std::shared_ptr<DeviceChangeWithInfoCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
        if (deviceChangeWithInfoCallbackMap_.count(sessionId) == 0) {
            return ERR_OPERATION_FAILED;
        }
        callback = deviceChangeWithInfoCallbackMap_.at(sessionId).lock();
        if (callback == nullptr) {
            deviceChangeWithInfoCallbackMap_.erase(sessionId);
            return ERR_CALLBACK_NOT_REGISTERED;
        }
    }
    if (callback != nullptr) {
        Trace traceCallback("callback->OnDeviceChangeWithInfo sessionid:" + std::to_string(sessionId)
            + " reason:" + std::to_string(static_cast<int>(reason)));
        callback->OnDeviceChangeWithInfo(sessionId, deviceInfo, reason);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnRendererStateChange(
    const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    int32_t size = static_cast<int32_t>(audioRendererChangeInfos.size());
    CHECK_AND_RETURN_RET_LOG(size < STATE_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lockCbMap(rendererStateChangeMutex_);
        callbacks = rendererStateChangeCallbackList_;
    }
    size_t cBSize = callbacks.size();
    size_t infosSize = audioRendererChangeInfos.size();
    AUDIO_DEBUG_LOG("cbSize: %{public}zu infoSize: %{public}zu", cBSize, infosSize);

    if (getuid() == RSS_UID) {
        AUDIO_INFO_LOG("cbSize: %{public}zu infoSize: %{public}zu", cBSize, infosSize);
    }

    Trace trace("AudioPolicyClientStubImpl::OnRendererStateChange");
    for (auto &cb : callbacks) {
        Trace traceCallback("OnRendererStateChange");
        cb->OnRendererStateChange(audioRendererChangeInfos);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnRecreateRendererStreamEvent(uint32_t sessionId, int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt &reason)
{
    AUDIO_INFO_LOG("Enter");
    std::shared_ptr<DeviceChangeWithInfoCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
        if (deviceChangeWithInfoCallbackMap_.count(sessionId) == 0) {
            AUDIO_ERR_LOG("No session id %{public}d", sessionId);
            return ERR_OPERATION_FAILED;
        }
        callback = deviceChangeWithInfoCallbackMap_.at(sessionId).lock();
    }
    if (callback != nullptr) {
        callback->OnRecreateStreamEvent(sessionId, streamFlag, reason);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnRecreateCapturerStreamEvent(uint32_t sessionId, int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt &reason)
{
    AUDIO_INFO_LOG("Enter");
    std::shared_ptr<DeviceChangeWithInfoCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
        if (deviceChangeWithInfoCallbackMap_.count(sessionId) == 0) {
            AUDIO_ERR_LOG("No session id %{public}d", sessionId);
            return ERR_OPERATION_FAILED;
        }
        callback = deviceChangeWithInfoCallbackMap_.at(sessionId).lock();
    }
    if (callback != nullptr) {
        callback->OnRecreateStreamEvent(sessionId, streamFlag, reason);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddCapturerStateChangeCallback(
    const std::shared_ptr<AudioCapturerStateChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(capturerStateChangeMutex_);
    capturerStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveCapturerStateChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(capturerStateChangeMutex_);
    capturerStateChangeCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetCapturerStateChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(capturerStateChangeMutex_);
    return capturerStateChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnCapturerStateChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    int32_t size = static_cast<int32_t>(audioCapturerChangeInfos.size());
    CHECK_AND_RETURN_RET_LOG(size < STATE_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::vector<std::shared_ptr<AudioCapturerStateChangeCallback>> tmpCallbackList;
    {
        std::lock_guard<std::mutex> lockCbMap(capturerStateChangeMutex_);
        for (auto it = capturerStateChangeCallbackList_.begin(); it != capturerStateChangeCallbackList_.end(); ++it) {
            std::shared_ptr<AudioCapturerStateChangeCallback> capturerStateChangeCallback = (*it).lock();
            if (capturerStateChangeCallback != nullptr) {
                tmpCallbackList.emplace_back(capturerStateChangeCallback);
            }
        }
    }
    for (auto it = tmpCallbackList.begin(); it != tmpCallbackList.end(); ++it) {
        if (*it == nullptr) {
            AUDIO_WARNING_LOG("tmpCallbackList is nullptr");
            continue;
        }
        (*it)->OnCapturerStateChange(audioCapturerChangeInfos);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddHeadTrackingDataRequestedChangeCallback(const std::string &macAddress,
    const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingDataRequestedChangeMutex_);
    if (!headTrackingDataRequestedChangeCallbackMap_.count(macAddress)) {
        AUDIO_INFO_LOG("First registeration for the specified device");
        headTrackingDataRequestedChangeCallbackMap_.insert(std::make_pair(macAddress, cb));
    } else {
        AUDIO_INFO_LOG("Repeated registeration for the specified device, replaced by the new one");
        headTrackingDataRequestedChangeCallbackMap_[macAddress] = cb;
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveHeadTrackingDataRequestedChangeCallback(const std::string &macAddress)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingDataRequestedChangeMutex_);
    headTrackingDataRequestedChangeCallbackMap_.erase(macAddress);
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetHeadTrackingDataRequestedChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingDataRequestedChangeMutex_);
    return headTrackingDataRequestedChangeCallbackMap_.size();
}

int32_t AudioPolicyClientStubImpl::OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo)
{
    int32_t size = static_cast<int32_t>(changeInfo.size());
    CHECK_AND_RETURN_RET_LOG(size < DEVICE_CHANGE_VALID_SIZE, ERR_INVALID_PARAM,
        "get invalid size : %{public}d", size);
    std::lock_guard<std::mutex> lockCbMap(headTrackingDataRequestedChangeMutex_);
    if (headTrackingDataRequestedChangeCallbackMap_.size() == 0) {
        return ERR_INVALID_PARAM;
    }
    for (const auto &pair : changeInfo) {
        if (!headTrackingDataRequestedChangeCallbackMap_.count(pair.first)) {
            AUDIO_WARNING_LOG("the specified device has not been registered");
            continue;
        }
        std::shared_ptr<HeadTrackingDataRequestedChangeCallback> headTrackingDataRequestedChangeCallback =
            headTrackingDataRequestedChangeCallbackMap_[pair.first];
        if (headTrackingDataRequestedChangeCallback != nullptr) {
            AUDIO_DEBUG_LOG("head tracking data requested change event of the specified device has been notified");
            headTrackingDataRequestedChangeCallback->OnHeadTrackingDataRequestedChange(pair.second);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddSpatializationEnabledChangeCallback(
    const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    spatializationEnabledChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveSpatializationEnabledChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    spatializationEnabledChangeCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetSpatializationEnabledChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    return spatializationEnabledChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnSpatializationEnabledChange(bool enabled)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    for (const auto &callback : spatializationEnabledChangeCallbackList_) {
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        callback->OnSpatializationEnabledChange(enabled);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, bool enabled)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    for (const auto &callback : spatializationEnabledChangeCallbackList_) {
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        callback->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddSpatializationEnabledChangeForCurrentDeviceCallback(
    const std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeForCurrentDeviceMutex_);
    spatializationEnabledChangeForCurrentDeviceCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveSpatializationEnabledChangeForCurrentDeviceCallback()
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeForCurrentDeviceMutex_);
    spatializationEnabledChangeForCurrentDeviceCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetSpatializationEnabledChangeForCurrentDeviceCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeForCurrentDeviceMutex_);
    return spatializationEnabledChangeForCurrentDeviceCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnSpatializationEnabledChangeForCurrentDevice(bool enabled)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeForCurrentDeviceMutex_);
    for (const auto &callback : spatializationEnabledChangeForCurrentDeviceCallbackList_) {
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        callback->OnSpatializationEnabledChangeForCurrentDevice(enabled);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddHeadTrackingEnabledChangeCallback(
    const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    headTrackingEnabledChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveHeadTrackingEnabledChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    headTrackingEnabledChangeCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetHeadTrackingEnabledChangeCallbacSize() const
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    return headTrackingEnabledChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnHeadTrackingEnabledChange(bool enabled)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    for (const auto &callback : headTrackingEnabledChangeCallbackList_) {
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        callback->OnHeadTrackingEnabledChange(enabled);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnHeadTrackingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, bool enabled)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    for (const auto &callback : headTrackingEnabledChangeCallbackList_) {
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        callback->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddNnStateChangeCallback(const std::shared_ptr<AudioNnStateChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(nnStateChangeMutex_);
    nnStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveNnStateChangeCallback()
{
    std::lock_guard<std::mutex> lockCbMap(nnStateChangeMutex_);
    nnStateChangeCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetNnStateChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(nnStateChangeMutex_);
    return nnStateChangeCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnNnStateChange(int32_t nnState)
{
    std::lock_guard<std::mutex> lockCbMap(nnStateChangeMutex_);
    for (const auto &callback : nnStateChangeCallbackList_) {
        callback->OnNnStateChange(nnState);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddAudioFormatUnsupportedErrorCallback(
    const std::shared_ptr<AudioFormatUnsupportedErrorCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(formatUnsupportedErrorMutex_);
    AudioFormatUnsupportedErrorCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveAudioFormatUnsupportedErrorCallback()
{
    std::lock_guard<std::mutex> lockCbMap(formatUnsupportedErrorMutex_);
    AudioFormatUnsupportedErrorCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetAudioFormatUnsupportedErrorCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(formatUnsupportedErrorMutex_);
    return AudioFormatUnsupportedErrorCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnFormatUnsupportedError(int32_t errorCode)
{
    std::lock_guard<std::mutex> lockCbMap(formatUnsupportedErrorMutex_);
    for (const auto &callback : AudioFormatUnsupportedErrorCallbackList_) {
        CHECK_AND_CONTINUE(callback != nullptr);
        callback->OnFormatUnsupportedError(static_cast<AudioErrors>(errorCode));
    }
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetStreamVolumeChangeCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(streamVolumeChangeMutex_);
    return streamVolumeChangeCallbackList_.size();
}

std::set<StreamUsage> AudioPolicyClientStubImpl::GetStreamVolumeChangeCallbackStreamUsages() const
{
    std::lock_guard<std::mutex> lockCbMap(streamVolumeChangeMutex_);
    std::set<StreamUsage> allStreamUsages;
    for (auto &[streamUsages, cb] : streamVolumeChangeCallbackList_) {
        (void)cb;
        allStreamUsages.insert(streamUsages.begin(), streamUsages.end());
    }
    return allStreamUsages;
}

int32_t AudioPolicyClientStubImpl::AddStreamVolumeChangeCallback(const std::set<StreamUsage> &streamUsages,
    const std::shared_ptr<StreamVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(streamVolumeChangeMutex_);
    streamVolumeChangeCallbackList_.emplace_back(streamUsages, cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveStreamVolumeChangeCallback(
    const std::shared_ptr<StreamVolumeChangeCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(streamVolumeChangeMutex_);
    if (cb == nullptr) {
        streamVolumeChangeCallbackList_.clear();
        return SUCCESS;
    }
    auto it = find_if(streamVolumeChangeCallbackList_.begin(), streamVolumeChangeCallbackList_.end(),
        [&cb](const std::pair<std::set<StreamUsage>, std::weak_ptr<StreamVolumeChangeCallback>> &elem) {
            return elem.second.lock() == cb;
        });
    if (it != streamVolumeChangeCallbackList_.end()) {
        streamVolumeChangeCallbackList_.erase(it);
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::OnStreamVolumeChange(const StreamVolumeEvent &streamVolumeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(volumeKeyEventMutex_);
    for (auto &[streamUsages, cb] : streamVolumeChangeCallbackList_) {
        std::shared_ptr<StreamVolumeChangeCallback> streamVolumeChangeCallback = cb.lock();
        if (streamVolumeChangeCallback != nullptr && streamUsages.count(streamVolumeEvent.streamUsage)) {
            streamVolumeChangeCallback->OnStreamVolumeChange(streamVolumeEvent);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::AddCollaborationEnabledChangeForCurrentDeviceCallback(
    const std::shared_ptr<AudioCollaborationEnabledChangeForCurrentDeviceCallback> &cb)
{
    std::lock_guard<std::mutex> lockCbMap(collaborationEnabledChangeForCurrentDeviceMutex_);
    collaborationEnabledChangeForCurrentDeviceCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::RemoveCollaborationEnabledChangeForCurrentDeviceCallback()
{
    std::lock_guard<std::mutex> lockCbMap(collaborationEnabledChangeForCurrentDeviceMutex_);
    collaborationEnabledChangeForCurrentDeviceCallbackList_.clear();
    return SUCCESS;
}

size_t AudioPolicyClientStubImpl::GetCollaborationEnabledChangeForCurrentDeviceCallbackSize() const
{
    std::lock_guard<std::mutex> lockCbMap(collaborationEnabledChangeForCurrentDeviceMutex_);
    return collaborationEnabledChangeForCurrentDeviceCallbackList_.size();
}

int32_t AudioPolicyClientStubImpl::OnCollaborationEnabledChangeForCurrentDevice(bool enabled)
{
    std::lock_guard<std::mutex> lockCbMap(collaborationEnabledChangeForCurrentDeviceMutex_);
    for (const auto &callback : collaborationEnabledChangeForCurrentDeviceCallbackList_) {
        CHECK_AND_CONTINUE_LOG(callback != nullptr, "callback is nullptr");
        AUDIO_INFO_LOG("OnCollaborationEnabledChangeForCurrentDevice enabled: %{public}d", enabled);
        callback->OnCollaborationEnabledChangeForCurrentDevice(enabled);
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS