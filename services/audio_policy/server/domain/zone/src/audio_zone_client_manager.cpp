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
#undef LOG_TAG
#define LOG_TAG "AudioZoneClientManager"

#include "audio_zone_client_manager.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
AudioZoneClientManager& AudioZoneClientManager::GetInstance()
{
    static AudioZoneClientManager manager(DelayedSingleton<AudioPolicyServerHandler>::GetInstance());
    return manager;
}

AudioZoneClientManager::AudioZoneClientManager(std::shared_ptr<AudioPolicyServerHandler> handler)
    : handler_(handler)
{
}

int32_t AudioZoneClientManager::RegisterAudioZoneClient(pid_t clientPid, sptr<IStandardAudioZoneClient> client)
{
    CHECK_AND_RETURN_RET_LOG(client != nullptr, ERROR, "client is null");
    AUDIO_INFO_LOG("register audio zone client %{public}d", clientPid);
    std::lock_guard<std::mutex> lock(clientMutex_);
    if (clients_.find(clientPid) != clients_.end()) {
        AUDIO_WARNING_LOG("register client duplicate %{public}d", clientPid);
    }
    clients_[clientPid] = client;
    return SUCCESS;
}

void AudioZoneClientManager::UnRegisterAudioZoneClient(pid_t clientPid)
{
    AUDIO_INFO_LOG("unregister audio zone client %{public}d", clientPid);
    std::lock_guard<std::mutex> lock(clientMutex_);
    if (clients_.find(clientPid) == clients_.end()) {
        AUDIO_WARNING_LOG("not found client %{public}d", clientPid);
    }
    clients_.erase(clientPid);
}

bool AudioZoneClientManager::IsRegisterAudioZoneClient(pid_t clientPid)
{
    std::lock_guard<std::mutex> lock(clientMutex_);
    return clients_.find(clientPid) != clients_.end();
}

void AudioZoneClientManager::DispatchEvent(std::shared_ptr<AudioZoneEvent> event)
{
    CHECK_AND_RETURN_LOG(event != nullptr, "event is null");

    if (event->descriptor != nullptr) {
        AUDIO_DEBUG_LOG("dispatch zone %{public}d event %{public}d to client %{public}d",
            event->descriptor->zoneId_, event->type, event->clientPid);
    } else {
        AUDIO_DEBUG_LOG("dispatch zone %{public}d event %{public}d to client %{public}d",
            event->zoneId, event->type, event->clientPid);
    }

    std::lock_guard<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    CHECK_AND_RETURN_LOG(clients_.find(event->clientPid) != clients_.end(),
        "client %{public}d not register", event->clientPid);

    switch (event->type) {
        case AudioZoneEventType::AUDIO_ZONE_ADD_EVENT:
            clients_[event->clientPid]->OnAudioZoneAdd(*(event->descriptor));
            break;
        case AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT:
            clients_[event->clientPid]->OnAudioZoneRemove(event->zoneId);
            break;
        case AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT:
            clients_[event->clientPid]->OnAudioZoneChange(event->zoneId,
                *(event->descriptor), static_cast<int32_t>(event->zoneChangeReason));
            break;
        case AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT:
            if (event->deviceTag.empty()) {
                clients_[event->clientPid]->OnInterruptEvent(event->zoneId,
                    ToIpcInterrupts(event->interrupts), static_cast<int32_t>(event->zoneInterruptReason));
            } else {
                clients_[event->clientPid]->OnInterruptEvent(event->zoneId,
                    event->deviceTag, ToIpcInterrupts(event->interrupts),
                    static_cast<int32_t>(event->zoneInterruptReason));
            }
            break;
        default:
            break;
    }
}

void AudioZoneClientManager::SendZoneAddEvent(pid_t clientPid, std::shared_ptr<AudioZoneDescriptor> descriptor)
{
    CHECK_AND_RETURN_LOG(descriptor != nullptr, "descriptor is null");
    std::lock_guard<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    CHECK_AND_RETURN_LOG(clients_.find(clientPid) != clients_.end(),
        "client %{public}d not register", clientPid);

    std::shared_ptr<AudioZoneEvent> event = std::make_shared<AudioZoneEvent>();
    CHECK_AND_RETURN_LOG(event != nullptr, "event is null");
    event->clientPid = clientPid;
    event->zoneId = descriptor->zoneId_;
    event->type = AudioZoneEventType::AUDIO_ZONE_ADD_EVENT;
    event->descriptor = descriptor;
    handler_->SendAudioZoneEvent(event);
    AUDIO_DEBUG_LOG("sned add audio zone %{public}d to client %{public}d",
        descriptor->zoneId_, clientPid);
}

void AudioZoneClientManager::SendZoneRemoveEvent(pid_t clientPid, int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    CHECK_AND_RETURN_LOG(clients_.find(clientPid) != clients_.end(),
        "client %{public}d not register", clientPid);

    std::shared_ptr<AudioZoneEvent> event = std::make_shared<AudioZoneEvent>();
    CHECK_AND_RETURN_LOG(event != nullptr, "event is null");
    event->clientPid = clientPid;
    event->zoneId = zoneId;
    event->type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
    handler_->SendAudioZoneEvent(event);
    AUDIO_DEBUG_LOG("sned remove audio zone %{public}d to client %{public}d",
        zoneId, clientPid);
}

void AudioZoneClientManager::SendZoneChangeEvent(pid_t clientPid, std::shared_ptr<AudioZoneDescriptor> descriptor,
    AudioZoneChangeReason reason)
{
    CHECK_AND_RETURN_LOG(descriptor != nullptr, "descriptor is null");
    std::lock_guard<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    CHECK_AND_RETURN_LOG(clients_.find(clientPid) != clients_.end(),
        "client %{public}d not register", clientPid);

    std::shared_ptr<AudioZoneEvent> event = std::make_shared<AudioZoneEvent>();
    CHECK_AND_RETURN_LOG(event != nullptr, "event is null");
    event->clientPid = clientPid;
    event->zoneId = descriptor->zoneId_;
    event->type = AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT;
    event->descriptor = descriptor;
    event->zoneChangeReason = reason;
    handler_->SendAudioZoneEvent(event);
    AUDIO_DEBUG_LOG("sned change audio zone %{public}d to client %{public}d",
        descriptor->zoneId_, clientPid);
}

void AudioZoneClientManager::SendZoneInterruptEvent(pid_t clientPid, int32_t zoneId, const std::string &deviceTag,
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts,
    AudioZoneInterruptReason reason)
{
    std::lock_guard<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    CHECK_AND_RETURN_LOG(clients_.find(clientPid) != clients_.end(),
        "client %{public}d not register", clientPid);

    std::shared_ptr<AudioZoneEvent> event = std::make_shared<AudioZoneEvent>();
    CHECK_AND_RETURN_LOG(event != nullptr, "event is null");
    event->clientPid = clientPid;
    event->zoneId = zoneId;
    event->deviceTag = deviceTag;
    event->type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
    event->interrupts = interrupts;
    event->zoneInterruptReason = reason;
    handler_->SendAudioZoneEvent(event);
    AUDIO_DEBUG_LOG("send audio zone %{public}d interrupt event to client %{public}d",
        zoneId, clientPid);
}

int32_t AudioZoneClientManager::SetSystemVolumeLevel(const pid_t clientPid, const int32_t zoneId,
    const AudioVolumeType volumeType, const int32_t volumeLevel, const int32_t volumeFlag)
{
    sptr<IStandardAudioZoneClient> client = nullptr;
    {
        std::lock_guard<std::mutex> lock(clientMutex_);
        CHECK_AND_RETURN_RET(clients_.find(clientPid) != clients_.end(), ERROR);
        client = clients_[clientPid];
    }
    AUDIO_DEBUG_LOG("set audio zone %{public}d volume %{public}d to client %{public}d",
        zoneId, volumeLevel, clientPid);
    return client->SetSystemVolume(zoneId, volumeType, volumeLevel, volumeFlag);
}

int32_t AudioZoneClientManager::GetSystemVolumeLevel(const pid_t clientPid, const int32_t zoneId,
    AudioVolumeType volumeType)
{
    sptr<IStandardAudioZoneClient> client = nullptr;
    {
        std::lock_guard<std::mutex> lock(clientMutex_);
        CHECK_AND_RETURN_RET(clients_.find(clientPid) != clients_.end(), ERROR);
        client = clients_[clientPid];
    }
    AUDIO_DEBUG_LOG("get audio zone %{public}d volume from client %{public}d",
        zoneId, clientPid);
    float outVolume = 0.0f;
    client->GetSystemVolume(zoneId, volumeType, outVolume);
    return static_cast<int32_t>(outVolume);
}
} // namespace AudioStandard
} // namespace OHOS