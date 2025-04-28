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

#include "audio_policy_client_stub_impl.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t RSS_UID = 1096;
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

void AudioPolicyClientStubImpl::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(volumeKeyEventMutex_);
    for (auto it = volumeKeyEventCallbackList_.begin(); it != volumeKeyEventCallbackList_.end(); ++it) {
        std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallback = (*it).lock();
        if (volumeKeyEventCallback != nullptr) {
            volumeKeyEventCallback->OnVolumeKeyEvent(volumeEvent);
        }
    }
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

void AudioPolicyClientStubImpl::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusInfoChange(focusInfoList);
    }
}

void AudioPolicyClientStubImpl::OnAudioFocusRequested(const AudioInterrupt &requestFocus)
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusRequested(requestFocus);
    }
}

void AudioPolicyClientStubImpl::OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus)
{
    std::lock_guard<std::mutex> lockCbMap(focusInfoChangeMutex_);
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusAbandoned(abandonFocus);
    }
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

void AudioPolicyClientStubImpl::OnDeviceChange(const DeviceChangeAction &dca)
{
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
}

void AudioPolicyClientStubImpl::OnDistribuitedOutputChange(const AudioDeviceDescriptor &deviceDesc, bool isRemote)
{
    for (auto &item : distribuitedOutputChangeCallback_) {
        item->OnDistribuitedOutputChange(deviceDesc, isRemote);
    }
}

void AudioPolicyClientStubImpl::OnMicrophoneBlocked(const MicrophoneBlockedInfo &blockedInfo)
{
    std::lock_guard<std::mutex> lockCbMap(microphoneBlockedMutex_);
    MicrophoneBlockedInfo microphoneBlockedInfo;
    microphoneBlockedInfo.blockStatus = blockedInfo.blockStatus;
    for (auto it = microphoneBlockedCallbackList_.begin(); it != microphoneBlockedCallbackList_.end(); ++it) {
        microphoneBlockedInfo.devices= blockedInfo.devices;
        if (it->second && microphoneBlockedInfo.devices.size() > 0) {
            it->second->OnMicrophoneBlocked(microphoneBlockedInfo);
        }
    }
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

void AudioPolicyClientStubImpl::OnAudioSceneChange(const AudioScene &audioScene)
{
    std::lock_guard<std::mutex> lockCbMap(audioSceneChangedMutex_);
    for (const auto &callback : audioSceneChangedCallbackList_) {
        callback->OnAudioSceneChange(audioScene);
    }
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
    AUDIO_INFO_LOG("Add selfAppVolumeChangeCallback appUid : %{public}d", appUid);
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

void AudioPolicyClientStubImpl::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    std::lock_guard<std::mutex> lockCbMap(ringerModeMutex_);
    for (auto it = ringerModeCallbackList_.begin(); it != ringerModeCallbackList_.end(); ++it) {
        (*it)->OnRingerModeUpdated(ringerMode);
    }
}

void AudioPolicyClientStubImpl::OnAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent)
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
}

int32_t AudioPolicyClientStubImpl::AddAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &cb)
{
    AUDIO_INFO_LOG("AddAudioSessionCallback in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    audioSessionCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::SetDistribuitedOutputChangeCallback(
    const std::shared_ptr<AudioDistribuitedOutputChangeCallback> &cb)
{
    distribuitedOutputChangeCallback_.clear();
    distribuitedOutputChangeCallback_.push_back(cb);
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

void AudioPolicyClientStubImpl::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionDeactive in");
    std::lock_guard<std::mutex> lockCbMap(audioSessionMutex_);
    for (auto it = audioSessionCallbackList_.begin(); it != audioSessionCallbackList_.end(); ++it) {
        (*it)->OnAudioSessionDeactive(deactiveEvent);
    }
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

void AudioPolicyClientStubImpl::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    std::lock_guard<std::mutex> lockCbMap(micStateChangeMutex_);
    for (auto it = micStateChangeCallbackList_.begin(); it != micStateChangeCallbackList_.end(); ++it) {
        (*it)->OnMicStateUpdated(micStateChangeEvent);
    }
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

void AudioPolicyClientStubImpl::OnPreferredOutputDeviceUpdated(const AudioRendererInfo &rendererInfo,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lockCbMap(pOutputDeviceChangeMutex_);
    auto it = preferredOutputDeviceCallbackMap_.find(rendererInfo.streamUsage);
    CHECK_AND_RETURN_LOG(it != preferredOutputDeviceCallbackMap_.end(), "streamUsage not found");
    for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
        CHECK_AND_CONTINUE_LOG(iter != it->second.end() && (*iter) != nullptr, "iter is null");
        (*iter)->OnPreferredOutputDeviceUpdated(desc);
    }
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

void AudioPolicyClientStubImpl::OnPreferredInputDeviceUpdated(const AudioCapturerInfo &capturerInfo,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lockCbMap(pInputDeviceChangeMutex_);
    auto it = preferredInputDeviceCallbackMap_.find(capturerInfo.sourceType);
    CHECK_AND_RETURN_LOG(it != preferredInputDeviceCallbackMap_.end(), "sourceType not found");
    for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
        CHECK_AND_CONTINUE_LOG(iter != it->second.end() && (*iter) != nullptr, "iter is null");
        (*iter)->OnPreferredInputDeviceUpdated(desc);
    }
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

void AudioPolicyClientStubImpl::OnRendererDeviceChange(const uint32_t sessionId,
    const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioPolicyClientStubImpl::OnRendererDeviceChange");
    std::shared_ptr<DeviceChangeWithInfoCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
        if (deviceChangeWithInfoCallbackMap_.count(sessionId) == 0) {
            return;
        }
        callback = deviceChangeWithInfoCallbackMap_.at(sessionId).lock();
        if (callback == nullptr) {
            deviceChangeWithInfoCallbackMap_.erase(sessionId);
            return;
        }
    }
    if (callback != nullptr) {
        Trace traceCallback("callback->OnDeviceChangeWithInfo sessionid:" + std::to_string(sessionId)
            + " reason:" + std::to_string(static_cast<int>(reason)));
        callback->OnDeviceChangeWithInfo(sessionId, deviceInfo, reason);
    }
}

void AudioPolicyClientStubImpl::OnRendererStateChange(
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
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
}

void AudioPolicyClientStubImpl::OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Enter");
    std::shared_ptr<DeviceChangeWithInfoCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
        if (deviceChangeWithInfoCallbackMap_.count(sessionId) == 0) {
            AUDIO_ERR_LOG("No session id %{public}d", sessionId);
            return;
        }
        callback = deviceChangeWithInfoCallbackMap_.at(sessionId).lock();
    }
    if (callback != nullptr) {
        callback->OnRecreateStreamEvent(sessionId, streamFlag, reason);
    }
}

void AudioPolicyClientStubImpl::OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Enter");
    std::shared_ptr<DeviceChangeWithInfoCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockCbMap(deviceChangeWithInfoCallbackMutex_);
        if (deviceChangeWithInfoCallbackMap_.count(sessionId) == 0) {
            AUDIO_ERR_LOG("No session id %{public}d", sessionId);
            return;
        }
        callback = deviceChangeWithInfoCallbackMap_.at(sessionId).lock();
    }
    if (callback != nullptr) {
        callback->OnRecreateStreamEvent(sessionId, streamFlag, reason);
    }
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

void AudioPolicyClientStubImpl::OnCapturerStateChange(
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
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

void AudioPolicyClientStubImpl::OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingDataRequestedChangeMutex_);
    if (headTrackingDataRequestedChangeCallbackMap_.size() == 0) {
        return;
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

void AudioPolicyClientStubImpl::OnSpatializationEnabledChange(const bool &enabled)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    for (const auto &callback : spatializationEnabledChangeCallbackList_) {
        callback->OnSpatializationEnabledChange(enabled);
    }
}

void AudioPolicyClientStubImpl::OnSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeMutex_);
    for (const auto &callback : spatializationEnabledChangeCallbackList_) {
        callback->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    }
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

void AudioPolicyClientStubImpl::OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled)
{
    std::lock_guard<std::mutex> lockCbMap(spatializationEnabledChangeForCurrentDeviceMutex_);
    for (const auto &callback : spatializationEnabledChangeForCurrentDeviceCallbackList_) {
        callback->OnSpatializationEnabledChangeForCurrentDevice(enabled);
    }
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

void AudioPolicyClientStubImpl::OnHeadTrackingEnabledChange(const bool &enabled)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    for (const auto &callback : headTrackingEnabledChangeCallbackList_) {
        callback->OnHeadTrackingEnabledChange(enabled);
    }
}

void AudioPolicyClientStubImpl::OnHeadTrackingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    std::lock_guard<std::mutex> lockCbMap(headTrackingEnabledChangeMutex_);
    for (const auto &callback : headTrackingEnabledChangeCallbackList_) {
        callback->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    }
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

void AudioPolicyClientStubImpl::OnNnStateChange(const int32_t &nnState)
{
    std::lock_guard<std::mutex> lockCbMap(nnStateChangeMutex_);
    for (const auto &callback : nnStateChangeCallbackList_) {
        callback->OnNnStateChange(nnState);
    }
}
} // namespace AudioStandard
} // namespace OHOS