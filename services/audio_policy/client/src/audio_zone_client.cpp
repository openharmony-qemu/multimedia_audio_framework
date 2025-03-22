/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioZoneClient"
#endif

#include "audio_zone_client.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioZoneClientStub::AudioZoneClientStub()
{}

AudioZoneClientStub::~AudioZoneClientStub()
{}

int32_t AudioZoneClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_ADD):
            HandleAudioZoneAdd(data, reply);
            break;
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_REMOVE):
            HandleAudioZoneRemove(data, reply);
            break;
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_CHANGE):
            HandleAudioZoneChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_INTERRUPT):
            HandleAudioZoneInterrupt(data, reply);
            break;
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_DEVICE_INTERRUPT):
            HandleAudioZoneDeviceInterrupt(data, reply);
            break;
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_SYSTEM_VOLUME_SET):
            HandleAudioZoneSetSystemVolume(data, reply);
            break;
        case static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_SYSTEM_VOLUME_GET):
            HandleAudioZoneGetSystemVolume(data, reply);
            break;
        default:
            break;
    }
    return SUCCESS;
}

void AudioZoneClientStub::HandleAudioZoneAdd(MessageParcel &data, MessageParcel &reply)
{
    AudioZoneDescriptor desc;
    desc.Unmarshalling(data);
    OnAudioZoneAdd(desc);
}

void AudioZoneClientStub::HandleAudioZoneRemove(MessageParcel &data, MessageParcel &reply)
{
    OnAudioZoneRemove(data.ReadInt32());
}

void AudioZoneClientStub::HandleAudioZoneChange(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    AudioZoneDescriptor desc;
    desc.Unmarshalling(data);
    AudioZoneChangeReason reason = static_cast<AudioZoneChangeReason>(data.ReadInt32());
    OnAudioZoneChange(zoneId, desc, reason);
}

void AudioZoneClientStub::HandleAudioZoneInterrupt(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    for (int i = 0; i < size; i++) {
        AudioInterrupt temp;
        AudioInterrupt::Unmarshalling(data, temp);
        AudioFocuState state = static_cast<AudioFocuState>(data.ReadInt32());
        interrupts.emplace_back(std::make_pair(temp, state));
    }
    AudioZoneInterruptReason reason = static_cast<AudioZoneInterruptReason>(data.ReadInt32());
    OnInterruptEvent(zoneId, interrupts, reason);
}

void AudioZoneClientStub::HandleAudioZoneDeviceInterrupt(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    int32_t deviceId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    for (int i = 0; i < size; i++) {
        AudioInterrupt temp;
        AudioInterrupt::Unmarshalling(data, temp);
        AudioFocuState state = static_cast<AudioFocuState>(data.ReadInt32());
        interrupts.emplace_back(std::make_pair(temp, state));
    }
    AudioZoneInterruptReason reason = static_cast<AudioZoneInterruptReason>(data.ReadInt32());
    OnInterruptEvent(zoneId, deviceId, interrupts, reason);
}

void AudioZoneClientStub::HandleAudioZoneSetSystemVolume(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t volumeLevel = data.ReadInt32();
    int32_t volumeFlag = data.ReadInt32();
    reply.WriteInt32(SetSystemVolume(zoneId, volumeType, volumeLevel, volumeFlag));
}

void AudioZoneClientStub::HandleAudioZoneGetSystemVolume(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    reply.WriteInt32(GetSystemVolume(zoneId, volumeType));
}

AudioZoneClient::AudioZoneClient()
{}

AudioZoneClient::~AudioZoneClient()
{}

int32_t AudioZoneClient::AddAudioZoneCallback(const std::shared_ptr<AudioZoneCallback> &callback)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    if (audioZoneCallback_ == nullptr) {
        int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneReport(true);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
            "EnableAudioZoneReport result:%{public}d", result);
    }
    audioZoneCallback_ = callback;
    return SUCCESS;
}

void AudioZoneClient::RemoveAudioZoneCallback()
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    CHECK_AND_RETURN_LOG(audioZoneCallback_ != nullptr, "audioZoneCallback is null.");
    AudioPolicyManager::GetInstance().EnableAudioZoneReport(false);
    audioZoneCallback_ = nullptr;
}

int32_t AudioZoneClient::AddAudioZoneChangeCallback(int32_t zoneId,
    const std::shared_ptr<AudioZoneChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lk(audioZoneChangeMutex_);
    if (audioZoneChangeCallbackMap_.find(zoneId) == audioZoneChangeCallbackMap_.end()) {
        int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneChangeReport(zoneId, true);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
            "EnableAudioZoneChangeReport result:%{public}d", result);
    }
    audioZoneChangeCallbackMap_[zoneId] = callback;
    return SUCCESS;
}

void AudioZoneClient::RemoveAudioZoneChangeCallback(int32_t zoneId)
{
    std::lock_guard<std::mutex> lk(audioZoneChangeMutex_);
    CHECK_AND_RETURN_LOG(audioZoneChangeCallbackMap_.find(zoneId) != audioZoneChangeCallbackMap_.end(),
        "audioZoneChangeCallbackMap_ not find zoneId.");
    AudioPolicyManager::GetInstance().EnableAudioZoneChangeReport(zoneId, false);
    audioZoneChangeCallbackMap_.erase(zoneId);
}

int32_t AudioZoneClient::AddAudioZoneVolumeProxy(int32_t zoneId,
    const std::shared_ptr<AudioZoneVolumeProxy> &proxy)
{
    std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
    if (audioZoneVolumeProxyMap_.find(zoneId) == audioZoneVolumeProxyMap_.end()) {
        int32_t result = AudioPolicyManager::GetInstance().EnableSystemVolumeProxy(zoneId, true);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
            "EnableSystemVolumeProxy result:%{public}d", result);
    }
    audioZoneVolumeProxyMap_[zoneId] = proxy;
    return SUCCESS;
}

void AudioZoneClient::RemoveAudioZoneVolumeProxy(int32_t zoneId)
{
    std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
    CHECK_AND_RETURN_LOG(audioZoneVolumeProxyMap_.find(zoneId)!= audioZoneVolumeProxyMap_.end(),
        "audioZoneVolumeProxyMap_ not find zoneId.");
    AudioPolicyManager::GetInstance().EnableSystemVolumeProxy(zoneId, false);
    audioZoneVolumeProxyMap_.erase(zoneId);
}

int32_t AudioZoneClient::AddAudioInterruptCallback(int32_t zoneId,
    const std::shared_ptr<AudioZoneInterruptCallback> &callback)
{
    return AddAudioInterruptCallback(zoneId, -1, callback);
}

int32_t AudioZoneClient::AddAudioInterruptCallback(int32_t zoneId, int32_t deviceId,
    const std::shared_ptr<AudioZoneInterruptCallback> &callback)
{
    int32_t key = GetInterruptKeyId(zoneId, deviceId);
    std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
    if (audioZoneInterruptCallbackMap_.find(key) == audioZoneInterruptCallbackMap_.end()) {
        int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceId, true);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
            "EnableAudioZoneInterruptReport result:%{public}d", result);
    }
    audioZoneInterruptCallbackMap_[key] = callback;
    return SUCCESS;
}

void AudioZoneClient::RemoveAudioInterruptCallback(int32_t zoneId)
{
    RemoveAudioInterruptCallback(zoneId, -1);
}

void AudioZoneClient::RemoveAudioInterruptCallback(int32_t zoneId, int32_t deviceId)
{
    int32_t key = GetInterruptKeyId(zoneId, deviceId);
    std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
    CHECK_AND_RETURN_LOG(audioZoneInterruptCallbackMap_.find(key)!= audioZoneInterruptCallbackMap_.end(),
        "audioZoneInterruptCallbackMap_ not find key.");
    AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceId, false);
    audioZoneInterruptCallbackMap_.erase(key);
}

void AudioZoneClient::Restore()
{
    {
        std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
        if (audioZoneCallback_ != nullptr) {
            int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneReport(true);
            AUDIO_INFO_LOG("EnableAudioZoneReport result:%{public}d", result);
        }
    }

    {
        std::lock_guard<std::mutex> lk(audioZoneChangeMutex_);
        for (const auto &it : audioZoneChangeCallbackMap_) {
            int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneChangeReport(it.first, true);
            AUDIO_INFO_LOG("EnableAudioZoneChangeReport result:%{public}d", result);
        }
    }

    {
        std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
        for (const auto &it : audioZoneVolumeProxyMap_) {
            int32_t result = AudioPolicyManager::GetInstance().EnableSystemVolumeProxy(it.first, true);
            AUDIO_INFO_LOG("EnableSystemVolumeProxy result:%{public}d", result);
        }
    }

    {
        std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
        for (const auto &it : audioZoneInterruptCallbackMap_) {
        /* 0xFFFFFFFF: mask */
            int32_t zoneId = static_cast<int32_t>(it.first & 0xFFFFFFFF);
            int32_t deviceId = static_cast<int32_t>((it.first >> BIT_32) & 0xFFFFFFFF);
            int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceId, true);
            AUDIO_INFO_LOG("EnableAudioZoneInterruptReport result:%{public}d", result);
        }
    }
}

int64_t AudioZoneClient::GetInterruptKeyId(int32_t zoneId, int32_t deviceId)
{
    int64_t key = zoneId;
    key = key << BIT_32;
    return key | deviceId;
}

void AudioZoneClient::OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    if (audioZoneCallback_ != nullptr) {
        audioZoneCallback_->OnAudioZoneAdd(zoneDescriptor);
    }
}

void AudioZoneClient::OnAudioZoneRemove(int32_t zoneId)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    if (audioZoneCallback_!= nullptr) {
        audioZoneCallback_->OnAudioZoneRemove(zoneId);
    }
}

void AudioZoneClient::OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
    AudioZoneChangeReason reason)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    if (audioZoneChangeCallbackMap_.find(zoneId)!= audioZoneChangeCallbackMap_.end()) {
        audioZoneChangeCallbackMap_[zoneId]->OnAudioZoneChange(zoneDescriptor, reason);
    }
}

void AudioZoneClient::OnInterruptEvent(int32_t zoneId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    AudioZoneInterruptReason reason)
{
    OnInterruptEvent(zoneId, -1, interrupts, reason);
}

void AudioZoneClient::OnInterruptEvent(int32_t zoneId, int32_t deviceId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    AudioZoneInterruptReason reason)
{
    int32_t key = GetInterruptKeyId(zoneId, deviceId);
    std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
    if (audioZoneInterruptCallbackMap_.find(key)!= audioZoneInterruptCallbackMap_.end()) {
        audioZoneInterruptCallbackMap_[key]->OnInterruptEvent(interrupts, reason);
    }
}

int32_t AudioZoneClient::SetSystemVolume(const int32_t zoneId, const AudioVolumeType volumeType,
    const int32_t volumeLevel, const int32_t volumeFlag)
{
    std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
    if (audioZoneVolumeProxyMap_.find(zoneId)!= audioZoneVolumeProxyMap_.end()) {
        audioZoneVolumeProxyMap_[zoneId]->SetSystemVolume(volumeType, volumeLevel);
        return SUCCESS;
    }
    return ERR_OPERATION_FAILED;
}

int32_t AudioZoneClient::GetSystemVolume(int32_t zoneId, AudioVolumeType volumeType)
{
    std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
    if (audioZoneVolumeProxyMap_.find(zoneId)!= audioZoneVolumeProxyMap_.end()) {
        return audioZoneVolumeProxyMap_[zoneId]->GetSystemVolume(volumeType);
    }
    return ERR_OPERATION_FAILED;
}
}  // namespace AudioStandard
}  // namespace OHOS

