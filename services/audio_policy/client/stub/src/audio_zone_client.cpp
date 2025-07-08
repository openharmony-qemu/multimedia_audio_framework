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
static constexpr int32_t MAX_SIZE = 1024;
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
    CHECK_AND_RETURN_LOG(audioZoneVolumeProxyMap_.find(zoneId) != audioZoneVolumeProxyMap_.end(),
        "audioZoneVolumeProxyMap_ not find zoneId.");
    AudioPolicyManager::GetInstance().EnableSystemVolumeProxy(zoneId, false);
    audioZoneVolumeProxyMap_.erase(zoneId);
}

int32_t AudioZoneClient::AddAudioInterruptCallback(int32_t zoneId,
    const std::shared_ptr<AudioZoneInterruptCallback> &callback)
{
    return AddAudioInterruptCallback(zoneId, "", callback);
}

int32_t AudioZoneClient::AddAudioInterruptCallback(int32_t zoneId, const std::string &deviceTag,
    const std::shared_ptr<AudioZoneInterruptCallback> &callback)
{
    std::string key = GetInterruptKeyId(zoneId, deviceTag);
    std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
    if (audioZoneInterruptCallbackMap_.find(key) == audioZoneInterruptCallbackMap_.end()) {
        int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceTag, true);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
            "EnableAudioZoneInterruptReport result:%{public}d", result);
    }
    audioZoneInterruptCallbackMap_[key] = callback;
    return SUCCESS;
}

void AudioZoneClient::RemoveAudioInterruptCallback(int32_t zoneId)
{
    RemoveAudioInterruptCallback(zoneId, "");
}

void AudioZoneClient::RemoveAudioInterruptCallback(int32_t zoneId, const std::string &deviceTag)
{
    std::string key = GetInterruptKeyId(zoneId, deviceTag);
    std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
    CHECK_AND_RETURN_LOG(audioZoneInterruptCallbackMap_.find(key) != audioZoneInterruptCallbackMap_.end(),
        "audioZoneInterruptCallbackMap_ not find key.");
    AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceTag, false);
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
            std::size_t pos = it.first.find('&');
            CHECK_AND_RETURN_LOG(pos != std::string::npos, "error str:%{public}s", it.first.c_str());
            std::string zoneIdStr = it.first.substr(0, pos);
            std::string deviceTag = it.first.substr(pos + 1);
            int32_t zoneId = 0;
            auto [ptr, ec] = std::from_chars(zoneIdStr.data(), zoneIdStr.data() + zoneIdStr.size(), zoneId);
            (void)ptr;
            CHECK_AND_CONTINUE_LOG(ec != std::errc::invalid_argument, "%{public}s is not a number", zoneIdStr.c_str());
            CHECK_AND_CONTINUE_LOG(ec != std::errc::result_out_of_range,
                "%{public}s is out of range", zoneIdStr.c_str());
            int32_t result = AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceTag, true);
            AUDIO_INFO_LOG("EnableAudioZoneInterruptReport result:%{public}d", result);
        }
    }
}

std::string AudioZoneClient::GetInterruptKeyId(int32_t zoneId, const std::string &deviceTag)
{
    return std::to_string(zoneId) + "&" + deviceTag;
}

int32_t AudioZoneClient::OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    CHECK_AND_RETURN_RET_LOG(audioZoneCallback_ != nullptr, ERR_OPERATION_FAILED, "audioZoneCallback_ is null.");
    audioZoneCallback_->OnAudioZoneAdd(zoneDescriptor);
    return SUCCESS;
}

int32_t AudioZoneClient::OnAudioZoneRemove(int32_t zoneId)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    CHECK_AND_RETURN_RET_LOG(audioZoneCallback_ != nullptr, ERR_OPERATION_FAILED, "audioZoneCallback_ is null.");
    audioZoneCallback_->OnAudioZoneRemove(zoneId);
    return SUCCESS;
}

int32_t AudioZoneClient::OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
    int32_t reason)
{
    std::lock_guard<std::mutex> lk(audioZoneCallbackMutex_);
    CHECK_AND_RETURN_RET_LOG(audioZoneChangeCallbackMap_.find(zoneId) != audioZoneChangeCallbackMap_.end(),
        ERR_OPERATION_FAILED, "audioZoneChangeCallbackMap_ not find zoneId %{public}d.", zoneId);
    CHECK_AND_RETURN_RET_LOG(audioZoneChangeCallbackMap_[zoneId] != nullptr,
        ERR_OPERATION_FAILED, "audioZoneChangeCallback is nullptr, zoneId %{public}d.", zoneId);
    audioZoneChangeCallbackMap_[zoneId]->OnAudioZoneChange(zoneDescriptor,
        static_cast<AudioZoneChangeReason>(reason));
    return SUCCESS;
}

int32_t AudioZoneClient::OnInterruptEvent(int32_t zoneId,
    const std::vector<std::map<AudioInterrupt, int32_t>> &ipcInterrupts,
    int32_t reason)
{
    int32_t size = static_cast<int32_t>(ipcInterrupts.size());
    CHECK_AND_RETURN_RET_LOG(size > 0 && size < MAX_SIZE, ERR_INVALID_PARAM,
        "invalid interrupt size: %{public}d", size);
    OnInterruptEvent(zoneId, "", ipcInterrupts, reason);
    return SUCCESS;
}

int32_t AudioZoneClient::OnInterruptEvent(int32_t zoneId, const std::string &deviceTag,
    const std::vector<std::map<AudioInterrupt, int32_t>> &ipcInterrupts,
    int32_t reason)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    for (const auto& map : ipcInterrupts) {
        for (const auto& [key, value] : map) {
            interrupts.emplace_back(key, static_cast<AudioFocuState>(value));
        }
    }

    std::string key = GetInterruptKeyId(zoneId, deviceTag);
    std::lock_guard<std::mutex> lk(audioZoneInterruptMutex_);
    CHECK_AND_RETURN_RET_LOG(audioZoneInterruptCallbackMap_.find(key) != audioZoneInterruptCallbackMap_.end(),
        ERR_OPERATION_FAILED, "audioZoneInterruptCallbackMap_ not find key %{public}s.", key.c_str());
    CHECK_AND_RETURN_RET_LOG(audioZoneInterruptCallbackMap_[key] != nullptr,
        ERR_OPERATION_FAILED, "audioZoneInterruptCallback is nullptr, key %{public}s.", key.c_str());

    audioZoneInterruptCallbackMap_[key]->OnInterruptEvent(interrupts,
        static_cast<AudioZoneInterruptReason>(reason));
    return SUCCESS;
}

int32_t AudioZoneClient::SetSystemVolume(int32_t zoneId, int32_t volumeType,
    int32_t volumeLevel, int32_t volumeFlag)
{
    std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
    CHECK_AND_RETURN_RET_LOG(audioZoneVolumeProxyMap_.find(zoneId) != audioZoneVolumeProxyMap_.end(),
        ERR_OPERATION_FAILED, "audioZoneVolumeProxyMap_ not find zoneId %{public}d.", zoneId);
    CHECK_AND_RETURN_RET_LOG(audioZoneVolumeProxyMap_[zoneId] != nullptr,
        ERR_OPERATION_FAILED, "audioZoneVolumeProxy is nullptr, zoneId %{public}d.", zoneId);

    audioZoneVolumeProxyMap_[zoneId]->SetSystemVolume(static_cast<AudioVolumeType>(volumeType),
        volumeLevel);
    return SUCCESS;
}

int32_t AudioZoneClient::GetSystemVolume(int32_t zoneId, int32_t volumeType, float &outVolume)
{
    std::lock_guard<std::mutex> lk(audioZoneVolumeProxyMutex_);
    CHECK_AND_RETURN_RET_LOG(audioZoneVolumeProxyMap_.find(zoneId) != audioZoneVolumeProxyMap_.end(),
        ERR_OPERATION_FAILED, "audioZoneVolumeProxyMap_ not find zoneId %{public}d.", zoneId);
    CHECK_AND_RETURN_RET_LOG(audioZoneVolumeProxyMap_[zoneId] != nullptr,
        ERR_OPERATION_FAILED, "audioZoneVolumeProxy is nullptr, zoneId %{public}d.", zoneId);

    outVolume = audioZoneVolumeProxyMap_[zoneId]->GetSystemVolume(static_cast<AudioVolumeType>(volumeType));
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS

