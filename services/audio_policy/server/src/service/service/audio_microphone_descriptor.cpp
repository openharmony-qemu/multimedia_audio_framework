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
#ifndef LOG_TAG
#define LOG_TAG "AudioMicrophoneDescriptor"
#endif

#include "audio_microphone_descriptor.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"

#include "audio_config_manager.h"
#include "audio_server_proxy.h"


namespace OHOS {
namespace AudioStandard {

static int32_t startMicrophoneId = 1;

int32_t AudioMicrophoneDescriptor::SetMicrophoneMute(bool isMute)
{
    AUDIO_DEBUG_LOG("SetMicrophoneMute state[%{public}d]", isMute);
    int32_t ret = AudioServerProxy::GetInstance().SetMicrophoneMuteProxy(isMute | isMicrophoneMutePersistent_);
    if (ret == SUCCESS) {
        isMicrophoneMuteTemporary_ = isMute;
        AudioStreamCollector::GetAudioStreamCollector().UpdateCapturerInfoMuteStatus(0,
            isMicrophoneMuteTemporary_ | isMicrophoneMutePersistent_);
    }
    return ret;
}

int32_t AudioMicrophoneDescriptor::SetMicrophoneMutePersistent(const bool isMute)
{
    AUDIO_DEBUG_LOG("state[%{public}d]", isMute);
    isMicrophoneMutePersistent_ = isMute;
    bool flag = isMicrophoneMuteTemporary_ | isMicrophoneMutePersistent_;
    int32_t ret = AudioServerProxy::GetInstance().SetMicrophoneMuteProxy(flag);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("UpdateCapturerInfoMuteStatus when set mic mute state persistent.");
        AudioStreamCollector::GetAudioStreamCollector().UpdateCapturerInfoMuteStatus(0,
            isMicrophoneMuteTemporary_|isMicrophoneMutePersistent_);
    }
    ret = AudioPolicyManagerFactory::GetAudioPolicyManager().SetPersistMicMuteState(isMicrophoneMutePersistent_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to save the persistent microphone mute status in setting database.");
        return ERROR;
    }
    return ret;
}

bool AudioMicrophoneDescriptor::GetPersistentMicMuteState()
{
    return isMicrophoneMutePersistent_;
}

int32_t AudioMicrophoneDescriptor::InitPersistentMicrophoneMuteState(bool &isMute)
{
    int32_t ret = AudioPolicyManagerFactory::GetAudioPolicyManager().GetPersistMicMuteState(isMute);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetPersistMicMuteState failed.");
        return ret;
    }
    // Ensure persistent mic mute state takes effect when first startup
    isMicrophoneMutePersistent_ = isMute;
    ret = AudioServerProxy::GetInstance().SetMicrophoneMuteProxy(isMicrophoneMutePersistent_);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("UpdateCapturerInfoMuteStatus when audio service restart.");
        AudioStreamCollector::GetAudioStreamCollector().UpdateCapturerInfoMuteStatus(0, isMicrophoneMutePersistent_);
    }
    return ret;
}

bool AudioMicrophoneDescriptor::IsMicrophoneMute()
{
    return isMicrophoneMuteTemporary_ | isMicrophoneMutePersistent_;
}

bool AudioMicrophoneDescriptor::GetMicrophoneMuteTemporary()
{
    return isMicrophoneMuteTemporary_;
}

bool AudioMicrophoneDescriptor::GetMicrophoneMutePersistent()
{
    return isMicrophoneMutePersistent_;
}

void AudioMicrophoneDescriptor::AddMicrophoneDescriptor(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    if (deviceDescriptor->deviceRole_ == INPUT_DEVICE &&
        deviceDescriptor->deviceType_ != DEVICE_TYPE_FILE_SOURCE) {
        auto isPresent = [&deviceDescriptor](const sptr<MicrophoneDescriptor> &desc) {
            CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
            return desc->deviceType_ == deviceDescriptor->deviceType_;
        };

        auto iter = std::find_if(connectedMicrophones_.begin(), connectedMicrophones_.end(), isPresent);
        if (iter == connectedMicrophones_.end()) {
            sptr<MicrophoneDescriptor> micDesc = new (std::nothrow) MicrophoneDescriptor(startMicrophoneId++,
                deviceDescriptor->deviceType_);
            CHECK_AND_RETURN_LOG(micDesc != nullptr, "new MicrophoneDescriptor failed");
            connectedMicrophones_.push_back(micDesc);
        }
    }
}

void AudioMicrophoneDescriptor::RemoveMicrophoneDescriptor(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    auto isPresent = [&deviceDescriptor](const sptr<MicrophoneDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return desc->deviceType_ == deviceDescriptor->deviceType_;
    };

    auto iter = std::find_if(connectedMicrophones_.begin(), connectedMicrophones_.end(), isPresent);
    if (iter != connectedMicrophones_.end()) {
        connectedMicrophones_.erase(iter);
    }
}

void AudioMicrophoneDescriptor::AddAudioCapturerMicrophoneDescriptor(int32_t sessionId, DeviceType devType)
{
    if (devType == DEVICE_TYPE_NONE) {
        audioCaptureMicrophoneDescriptor_[sessionId] = new MicrophoneDescriptor(0, DEVICE_TYPE_INVALID);
        return;
    }
    auto isPresent = [&devType] (const sptr<MicrophoneDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid microphone descriptor");
        return (devType == desc->deviceType_);
    };

    auto itr = std::find_if(connectedMicrophones_.begin(), connectedMicrophones_.end(), isPresent);
    if (itr != connectedMicrophones_.end()) {
        audioCaptureMicrophoneDescriptor_[sessionId] = *itr;
    }
}

vector<sptr<MicrophoneDescriptor>> AudioMicrophoneDescriptor::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    vector<sptr<MicrophoneDescriptor>> descList = {};
    const auto desc = audioCaptureMicrophoneDescriptor_.find(sessionId);
    if (desc != audioCaptureMicrophoneDescriptor_.end()) {
        sptr<MicrophoneDescriptor> micDesc = new (std::nothrow) MicrophoneDescriptor(desc->second);
        if (micDesc == nullptr) {
            AUDIO_ERR_LOG("Create microphone device descriptor failed");
            return descList;
        }
        descList.push_back(micDesc);
    }
    return descList;
}

vector<sptr<MicrophoneDescriptor>> AudioMicrophoneDescriptor::GetAvailableMicrophones()
{
    return connectedMicrophones_;
}

void AudioMicrophoneDescriptor::UpdateAudioCapturerMicrophoneDescriptor(DeviceType devType)
{
    auto isPresent = [&devType] (const sptr<MicrophoneDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid microphone descriptor");
        return (devType == desc->deviceType_);
    };

    auto itr = std::find_if(connectedMicrophones_.begin(), connectedMicrophones_.end(), isPresent);
    if (itr != connectedMicrophones_.end()) {
        for (auto& desc : audioCaptureMicrophoneDescriptor_) {
            if (desc.second->deviceType_ != devType) {
                desc.second = *itr;
            }
        }
    }
}

void AudioMicrophoneDescriptor::RemoveAudioCapturerMicrophoneDescriptor(int32_t uid)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioStreamCollector::GetAudioStreamCollector().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);

    for (auto &info : audioCapturerChangeInfos) {
        if (info->clientUID != uid && info->createrUID != uid) {
            continue;
        }
        audioCaptureMicrophoneDescriptor_.erase(info->sessionId);
    }
}

void AudioMicrophoneDescriptor::RemoveAudioCapturerMicrophoneDescriptorBySessionID(int32_t sessionID)
{
    audioCaptureMicrophoneDescriptor_.erase(sessionID);
}

}
}