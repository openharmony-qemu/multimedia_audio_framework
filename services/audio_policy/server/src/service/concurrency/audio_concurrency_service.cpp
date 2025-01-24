/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_concurrency_service.h"
#include "audio_concurrency_state_listener_proxy.h"

namespace OHOS {
namespace AudioStandard {
void AudioConcurrencyService::Init()
{
    AUDIO_INFO_LOG("AudioConcurrencyService Init");
    std::unique_ptr<AudioConcurrencyParser> parser = std::make_unique<AudioConcurrencyParser>();
    CHECK_AND_RETURN_LOG(parser != nullptr, "Create audioConcurrency parser failed!");
    CHECK_AND_RETURN_LOG(!parser->LoadConfig(concurrencyCfgMap_), "Load audioConcurrency cfgMap failed!");
}

void AudioConcurrencyService::DispatchConcurrencyEventWithSessionId(uint32_t sessionID)
{
    CHECK_AND_RETURN_LOG(sessionID >= MIN_SESSIONID && sessionID <= MAX_SESSIONID,
        "EntryPoint Taint Mark:arg sessionID: %{public}u is tained", sessionID);
    std::lock_guard<std::mutex> lock(cbMapMutex_);
    AUDIO_DEBUG_LOG("DispatchConcurrencyEventWithSessionId %{public}d", sessionID);
    CHECK_AND_RETURN_LOG(concurrencyClients_.find(sessionID) != concurrencyClients_.end(),
        "session %{public}u not exist", sessionID);
    concurrencyClients_[sessionID]->OnConcedeStream();
}

AudioConcurrencyService::AudioConcurrencyDeathRecipient::AudioConcurrencyDeathRecipient(
    const std::shared_ptr<AudioConcurrencyService> &service, uint32_t sessionID)
    : service_(service), sessionID_(sessionID)
{
}

void AudioConcurrencyService::AudioConcurrencyDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    std::shared_ptr<AudioConcurrencyService> service = service_.lock();
    if (service != nullptr) {
        service->UnsetAudioConcurrencyCallback(sessionID_);
    }
}

AudioConcurrencyService::AudioConcurrencyClient::AudioConcurrencyClient(
    const std::shared_ptr<AudioConcurrencyCallback> &callback, const sptr<IRemoteObject> &object,
    const sptr<AudioConcurrencyDeathRecipient> &deathRecipient, uint32_t sessionID)
    : callback_(callback), object_(object), deathRecipient_(deathRecipient), sessionID_(sessionID)
{
    AUDIO_DEBUG_LOG("callback ctor, sessionID %{public}u", sessionID_);
}

AudioConcurrencyService::AudioConcurrencyClient::~AudioConcurrencyClient()
{
    AUDIO_DEBUG_LOG("callback dtor, sessionID %{public}u", sessionID_);
    if (object_ != nullptr) {
        object_->RemoveDeathRecipient(deathRecipient_);
    }
}

int32_t AudioConcurrencyService::SetAudioConcurrencyCallback(const uint32_t sessionID,
    const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(cbMapMutex_);

    // maybe add check session id validation here

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "AudioConcurrencytCb object is nullptr");

    sptr<IStandardConcurrencyStateListener> listener = iface_cast<IStandardConcurrencyStateListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "AudioConcurrencytCb obj cast failed");

    std::shared_ptr<AudioConcurrencyCallback> callback = std::make_shared<AudioConcurrencyListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "AudioConcurrencyService create cb failed");

    if (concurrencyClients_.find(sessionID) == concurrencyClients_.end()) {
        sptr<AudioConcurrencyDeathRecipient> deathRecipient = new
            AudioConcurrencyDeathRecipient(shared_from_this(), sessionID);
        object->AddDeathRecipient(deathRecipient);
        std::shared_ptr<AudioConcurrencyClient> client =
            std::make_shared<AudioConcurrencyClient>(callback, object, deathRecipient, sessionID);
        concurrencyClients_[sessionID] = client;
    } else {
        AUDIO_ERR_LOG("session %{public}u already exist", sessionID);
        return ERR_INVALID_PARAM;
    }

    return SUCCESS;
}

int32_t AudioConcurrencyService::UnsetAudioConcurrencyCallback(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(cbMapMutex_);

    if (concurrencyClients_.erase(sessionID) == 0) {
        AUDIO_ERR_LOG("session %{public}u not present", sessionID);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

void AudioConcurrencyService::SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler)
{
    handler_ = handler;
}

void AudioConcurrencyService::AudioConcurrencyClient::OnConcedeStream()
{
    if (callback_ != nullptr) {
        callback_->OnConcedeStream();
    }
}

int32_t AudioConcurrencyService::ActivateAudioConcurrency(AudioPipeType incomingPipeType,
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    if (concurrencyCfgMap_.empty()) {
        return SUCCESS;
    }
    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        if ((*it)->rendererInfo.pipeType == incomingPipeType && (incomingPipeType == PIPE_TYPE_OFFLOAD ||
            incomingPipeType == PIPE_TYPE_MULTICHANNEL)) {
            continue;
        }
        CHECK_AND_RETURN_RET_LOG(concurrencyCfgMap_[std::make_pair((*it)->rendererInfo.pipeType, incomingPipeType)] !=
            CONCEDE_INCOMING, ERR_CONCEDE_INCOMING_STREAM, "existing session %{public}d, "
            "pipe %{public}d, concede incoming pipe %{public}d", (*it)->sessionId, (*it)->rendererInfo.pipeType,
            incomingPipeType);
    }
    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        CHECK_AND_RETURN_RET_LOG(concurrencyCfgMap_[std::make_pair((*it)->capturerInfo.pipeType, incomingPipeType)] !=
            CONCEDE_INCOMING, ERR_CONCEDE_INCOMING_STREAM, "existing session %{public}d, "
            "pipe %{public}d, concede incoming pipe %{public}d", (*it)->sessionId, (*it)->capturerInfo.pipeType,
            incomingPipeType);
    }
    bool concedeIncomingVoipCap = false;
    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        if ((*it)->rendererInfo.pipeType == incomingPipeType && (incomingPipeType == PIPE_TYPE_OFFLOAD ||
            incomingPipeType == PIPE_TYPE_MULTICHANNEL)) {
            continue;
        }
        ConcurrencyAction action = concurrencyCfgMap_[std::make_pair((*it)->rendererInfo.pipeType, incomingPipeType)];
        if (action == CONCEDE_EXISTING && handler_ != nullptr) {
            handler_->SendConcurrencyEventWithSessionIDCallback((*it)->sessionId);
        }
    }
    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        ConcurrencyAction action = concurrencyCfgMap_[std::make_pair((*it)->capturerInfo.pipeType, incomingPipeType)];
        if (action == CONCEDE_EXISTING && handler_ != nullptr) {
            if (incomingPipeType == PIPE_TYPE_CALL_IN && (*it)->capturerInfo.pipeType == PIPE_TYPE_CALL_IN) {
                concedeIncomingVoipCap = true;
            }
            handler_->SendConcurrencyEventWithSessionIDCallback((*it)->sessionId);
        }
    }
    CHECK_AND_RETURN_RET_LOG(!concedeIncomingVoipCap, ERR_CONCEDE_INCOMING_STREAM,
        "Existing call in concede incoming call in");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS