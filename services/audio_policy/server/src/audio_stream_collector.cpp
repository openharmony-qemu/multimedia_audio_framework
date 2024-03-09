/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "audio_stream_collector.h"

#include "audio_errors.h"
#include "audio_client_tracker_callback_proxy.h"
#include "ipc_skeleton.h"
#include "i_standard_client_tracker.h"
#include "hisysevent.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const map<pair<ContentType, StreamUsage>, AudioStreamType> AudioStreamCollector::streamTypeMap_ =
    AudioStreamCollector::CreateStreamMap();

map<pair<ContentType, StreamUsage>, AudioStreamType> AudioStreamCollector::CreateStreamMap()
{
    map<pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_MOVIE, STREAM_USAGE_MEDIA)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_GAME, STREAM_USAGE_MEDIA)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_MEDIA)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_ULTRASONIC, STREAM_USAGE_SYSTEM)] = STREAM_ULTRASONIC;

    // Old mapping relationships from content and usage to stream type
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_UNKNOWN)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_MEDIA)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_UNKNOWN)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_MEDIA)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;

    // Only use stream usage to choose stream type
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MESSAGE)] = STREAM_VOICE_MESSAGE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_GAME)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_AUDIOBOOK)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NAVIGATION)] = STREAM_NAVIGATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_DTMF)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ULTRASONIC)] = STREAM_ULTRASONIC;

    return streamMap;
}

AudioStreamCollector::AudioStreamCollector() : audioSystemMgr_
    (AudioSystemManager::GetInstance())
{
    audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AUDIO_INFO_LOG("AudioStreamCollector()");
}

AudioStreamCollector::~AudioStreamCollector()
{
    AUDIO_INFO_LOG("~AudioStreamCollector()");
}

int32_t AudioStreamCollector::AddRendererStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("AddRendererStream playback client id %{public}d session %{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.sessionId);

    rendererStatequeue_.insert({{streamChangeInfo.audioRendererChangeInfo.clientUID,
        streamChangeInfo.audioRendererChangeInfo.sessionId},
        streamChangeInfo.audioRendererChangeInfo.rendererState});

    unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_unique<AudioRendererChangeInfo>();
    if (!rendererChangeInfo) {
        AUDIO_ERR_LOG("AddRendererStream Memory Allocation Failed");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    rendererChangeInfo->createrUID = streamChangeInfo.audioRendererChangeInfo.createrUID;
    rendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
    rendererChangeInfo->sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    rendererChangeInfo->callerPid = streamChangeInfo.audioRendererChangeInfo.callerPid;
    rendererChangeInfo->tokenId = IPCSkeleton::GetCallingTokenID();
    rendererChangeInfo->rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    rendererChangeInfo->rendererInfo = streamChangeInfo.audioRendererChangeInfo.rendererInfo;
    rendererChangeInfo->outputDeviceInfo = streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo;
    rendererChangeInfo->channelCount = streamChangeInfo.audioRendererChangeInfo.channelCount;
    audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    AUDIO_DEBUG_LOG("audioRendererChangeInfos_: Added for client %{public}d session %{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.sessionId);

    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "audioPolicyServerHandler_ is nullptr, callback error");
    audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    return SUCCESS;
}

void AudioStreamCollector::GetRendererStreamInfo(AudioStreamChangeInfo &streamChangeInfo,
    AudioRendererChangeInfo &rendererInfo)
{
    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->clientUID == streamChangeInfo.audioRendererChangeInfo.clientUID &&
            (*it)->sessionId == streamChangeInfo.audioRendererChangeInfo.sessionId) {
            rendererInfo.outputDeviceInfo = (*it)->outputDeviceInfo;
            return;
        }
    }
}

void AudioStreamCollector::GetCapturerStreamInfo(AudioStreamChangeInfo &streamChangeInfo,
    AudioCapturerChangeInfo &capturerInfo)
{
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->clientUID == streamChangeInfo.audioCapturerChangeInfo.clientUID &&
            (*it)->sessionId == streamChangeInfo.audioCapturerChangeInfo.sessionId) {
            capturerInfo.inputDeviceInfo = (*it)->inputDeviceInfo;
            return;
        }
    }
}

int32_t AudioStreamCollector::AddCapturerStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("AddCapturerStream recording client id %{public}d session %{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID, streamChangeInfo.audioCapturerChangeInfo.sessionId);

    capturerStatequeue_.insert({{streamChangeInfo.audioCapturerChangeInfo.clientUID,
        streamChangeInfo.audioCapturerChangeInfo.sessionId},
        streamChangeInfo.audioCapturerChangeInfo.capturerState});

    unique_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_unique<AudioCapturerChangeInfo>();
    if (!capturerChangeInfo) {
        AUDIO_ERR_LOG("AddCapturerStream Memory Allocation Failed");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    capturerChangeInfo->createrUID = streamChangeInfo.audioCapturerChangeInfo.createrUID;
    capturerChangeInfo->clientUID = streamChangeInfo.audioCapturerChangeInfo.clientUID;
    capturerChangeInfo->sessionId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    capturerChangeInfo->callerPid = streamChangeInfo.audioCapturerChangeInfo.callerPid;
    capturerChangeInfo->muted = streamChangeInfo.audioCapturerChangeInfo.muted;

    capturerChangeInfo->capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    capturerChangeInfo->capturerInfo = streamChangeInfo.audioCapturerChangeInfo.capturerInfo;
    capturerChangeInfo->inputDeviceInfo = streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo;
    audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));

    AUDIO_DEBUG_LOG("audioCapturerChangeInfos_: Added for client %{public}d session %{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID, streamChangeInfo.audioCapturerChangeInfo.sessionId);

    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "audioPolicyServerHandler_ is nullptr, callback error");
    audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos_);
    return SUCCESS;
}

int32_t AudioStreamCollector::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object)
{
    AUDIO_DEBUG_LOG("RegisterTracker mode %{public}d", mode);

    int32_t clientId;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    if (mode == AUDIO_MODE_PLAYBACK) {
        AddRendererStream(streamChangeInfo);
        clientId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    } else {
        // mode = AUDIO_MODE_RECORD
        AddCapturerStream(streamChangeInfo);
        clientId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    }

    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr,
        ERR_INVALID_PARAM, "AudioStreamCollector: client tracker obj cast failed");
    std::shared_ptr<AudioClientTracker> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        ERR_INVALID_PARAM, "AudioStreamCollector: failed to create tracker cb obj");
    clientTracker_[clientId] = callback;
    WriterStreamChangeSysEvent(mode, streamChangeInfo);
    return SUCCESS;
}

void AudioStreamCollector::SetRendererStreamParam(AudioStreamChangeInfo &streamChangeInfo,
    unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo)
{
    rendererChangeInfo->createrUID = streamChangeInfo.audioRendererChangeInfo.createrUID;
    rendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
    rendererChangeInfo->sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    rendererChangeInfo->callerPid = streamChangeInfo.audioRendererChangeInfo.callerPid;
    rendererChangeInfo->clientPid = streamChangeInfo.audioRendererChangeInfo.clientPid;
    rendererChangeInfo->tokenId = IPCSkeleton::GetCallingTokenID();
    rendererChangeInfo->rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    rendererChangeInfo->rendererInfo = streamChangeInfo.audioRendererChangeInfo.rendererInfo;
    rendererChangeInfo->outputDeviceInfo = streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo;
}

void AudioStreamCollector::SetCapturerStreamParam(AudioStreamChangeInfo &streamChangeInfo,
    unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo)
{
    capturerChangeInfo->createrUID = streamChangeInfo.audioCapturerChangeInfo.createrUID;
    capturerChangeInfo->clientUID = streamChangeInfo.audioCapturerChangeInfo.clientUID;
    capturerChangeInfo->sessionId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    capturerChangeInfo->callerPid = streamChangeInfo.audioCapturerChangeInfo.callerPid;
    capturerChangeInfo->clientPid = streamChangeInfo.audioCapturerChangeInfo.clientPid;
    capturerChangeInfo->muted = streamChangeInfo.audioCapturerChangeInfo.muted;
    capturerChangeInfo->capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    capturerChangeInfo->capturerInfo = streamChangeInfo.audioCapturerChangeInfo.capturerInfo;
    capturerChangeInfo->inputDeviceInfo = streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo;
}

int32_t AudioStreamCollector::UpdateRendererStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("UpdateRendererStream client %{public}d state %{public}d session %{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.rendererState,
        streamChangeInfo.audioRendererChangeInfo.sessionId);
    if (rendererStatequeue_.find(make_pair(streamChangeInfo.audioRendererChangeInfo.clientUID,
        streamChangeInfo.audioRendererChangeInfo.sessionId)) != rendererStatequeue_.end()) {
        if (streamChangeInfo.audioRendererChangeInfo.rendererState ==
            rendererStatequeue_[make_pair(streamChangeInfo.audioRendererChangeInfo.clientUID,
                streamChangeInfo.audioRendererChangeInfo.sessionId)]) {
            // Renderer state not changed
            return SUCCESS;
        }
    } else {
        AUDIO_INFO_LOG("UpdateRendererStream client %{public}d not found in rendererStatequeue_",
            streamChangeInfo.audioRendererChangeInfo.clientUID);
    }

    // Update the renderer info in audioRendererChangeInfos_
    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfo = **it;
        if (audioRendererChangeInfo.clientUID == streamChangeInfo.audioRendererChangeInfo.clientUID &&
            audioRendererChangeInfo.sessionId == streamChangeInfo.audioRendererChangeInfo.sessionId) {
            rendererStatequeue_[make_pair(audioRendererChangeInfo.clientUID, audioRendererChangeInfo.sessionId)] =
                streamChangeInfo.audioRendererChangeInfo.rendererState;
            AUDIO_DEBUG_LOG("UpdateRendererStream: update client %{public}d session %{public}d",
                audioRendererChangeInfo.clientUID, audioRendererChangeInfo.sessionId);

            unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_unique<AudioRendererChangeInfo>();
            CHECK_AND_RETURN_RET_LOG(rendererChangeInfo != nullptr,
                ERR_MEMORY_ALLOC_FAILED, "RendererChangeInfo Memory Allocation Failed");
            SetRendererStreamParam(streamChangeInfo, rendererChangeInfo);
            rendererChangeInfo->channelCount = (*it)->channelCount;
            if (rendererChangeInfo->rendererState != RENDERER_RUNNING) {
                rendererChangeInfo->outputDeviceInfo = (*it)->outputDeviceInfo;
            }
            *it = move(rendererChangeInfo);

            if (audioPolicyServerHandler_ != nullptr) {
                audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
            }

            if (streamChangeInfo.audioRendererChangeInfo.rendererState == RENDERER_RELEASED) {
                audioRendererChangeInfos_.erase(it);
                rendererStatequeue_.erase(make_pair(audioRendererChangeInfo.clientUID,
                    audioRendererChangeInfo.sessionId));
                clientTracker_.erase(audioRendererChangeInfo.sessionId);
            }
            return SUCCESS;
        }
    }
    AUDIO_INFO_LOG("UpdateRendererStream: Not found clientUid:%{public}d sessionId:%{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.clientUID);
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateCapturerStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("UpdateCapturerStream client %{public}d state %{public}d session %{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID, streamChangeInfo.audioCapturerChangeInfo.capturerState,
        streamChangeInfo.audioCapturerChangeInfo.sessionId);

    if (capturerStatequeue_.find(make_pair(streamChangeInfo.audioCapturerChangeInfo.clientUID,
        streamChangeInfo.audioCapturerChangeInfo.sessionId)) != capturerStatequeue_.end()) {
        if (streamChangeInfo.audioCapturerChangeInfo.capturerState ==
            capturerStatequeue_[make_pair(streamChangeInfo.audioCapturerChangeInfo.clientUID,
                streamChangeInfo.audioCapturerChangeInfo.sessionId)]) {
            // Capturer state not changed
            return SUCCESS;
        }
    }

    // Update the capturer info in audioCapturerChangeInfos_
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        AudioCapturerChangeInfo audioCapturerChangeInfo = **it;
        if (audioCapturerChangeInfo.clientUID == streamChangeInfo.audioCapturerChangeInfo.clientUID &&
            audioCapturerChangeInfo.sessionId == streamChangeInfo.audioCapturerChangeInfo.sessionId) {
            capturerStatequeue_[make_pair(audioCapturerChangeInfo.clientUID, audioCapturerChangeInfo.sessionId)] =
                streamChangeInfo.audioCapturerChangeInfo.capturerState;

            AUDIO_DEBUG_LOG("Session is updated for client %{public}d session %{public}d",
                streamChangeInfo.audioCapturerChangeInfo.clientUID,
                streamChangeInfo.audioCapturerChangeInfo.sessionId);

            unique_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_unique<AudioCapturerChangeInfo>();
            CHECK_AND_RETURN_RET_LOG(capturerChangeInfo != nullptr,
                ERR_MEMORY_ALLOC_FAILED, "CapturerChangeInfo Memory Allocation Failed");
            SetCapturerStreamParam(streamChangeInfo, capturerChangeInfo);
            if (streamChangeInfo.audioCapturerChangeInfo.capturerState != CAPTURER_RUNNING) {
                capturerChangeInfo->inputDeviceInfo = (*it)->inputDeviceInfo;
            }
            *it = move(capturerChangeInfo);
            if (audioPolicyServerHandler_ != nullptr) {
                audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos_);
            }
            if (streamChangeInfo.audioCapturerChangeInfo.capturerState ==  CAPTURER_RELEASED) {
                audioCapturerChangeInfos_.erase(it);
                capturerStatequeue_.erase(make_pair(audioCapturerChangeInfo.clientUID,
                    audioCapturerChangeInfo.sessionId));
                clientTracker_.erase(audioCapturerChangeInfo.sessionId);
            }
            return SUCCESS;
        }
    }
    AUDIO_DEBUG_LOG("UpdateCapturerStream: clientUI not in audioCapturerChangeInfos_::%{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID);
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateRendererDeviceInfo(DeviceInfo &outputDeviceInfo)
{
    bool deviceInfoUpdated = false;

    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->outputDeviceInfo.deviceType != outputDeviceInfo.deviceType) {
            AUDIO_DEBUG_LOG("UpdateRendererDeviceInfo: old device: %{public}d new device: %{public}d",
                (*it)->outputDeviceInfo.deviceType, outputDeviceInfo.deviceType);
            (*it)->outputDeviceInfo = outputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateCapturerDeviceInfo(DeviceInfo &inputDeviceInfo)
{
    bool deviceInfoUpdated = false;

    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->inputDeviceInfo.deviceType != inputDeviceInfo.deviceType) {
            AUDIO_DEBUG_LOG("UpdateCapturerDeviceInfo: old device: %{public}d new device: %{public}d",
                (*it)->inputDeviceInfo.deviceType, inputDeviceInfo.deviceType);
            (*it)->inputDeviceInfo = inputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateRendererDeviceInfo(int32_t clientUID, int32_t sessionId,
    DeviceInfo &outputDeviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool deviceInfoUpdated = false;

    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->clientUID == clientUID && (*it)->sessionId == sessionId) {
            AUDIO_DEBUG_LOG("uid %{public}d sessionId %{public}d update device: old %{public}d, new %{public}d",
                clientUID, sessionId, (*it)->outputDeviceInfo.deviceType, outputDeviceInfo.deviceType);
            (*it)->outputDeviceInfo = outputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    }
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateCapturerDeviceInfo(int32_t clientUID, int32_t sessionId,
    DeviceInfo &inputDeviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool deviceInfoUpdated = false;

    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->clientUID == clientUID && (*it)->sessionId == sessionId) {
            AUDIO_DEBUG_LOG("uid %{public}d sessionId %{public}d update device: old %{public}d, new %{public}d",
                (*it)->clientUID, (*it)->sessionId, (*it)->inputDeviceInfo.deviceType, inputDeviceInfo.deviceType);
            (*it)->inputDeviceInfo = inputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateTracker(const AudioMode &mode, DeviceInfo &deviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    if (mode == AUDIO_MODE_PLAYBACK) {
        UpdateRendererDeviceInfo(deviceInfo);
    } else {
        UpdateCapturerDeviceInfo(deviceInfo);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    // update the stream change info
    if (mode == AUDIO_MODE_PLAYBACK) {
        UpdateRendererStream(streamChangeInfo);
    } else {
    // mode = AUDIO_MODE_RECORD
        UpdateCapturerStream(streamChangeInfo);
    }
    WriterStreamChangeSysEvent(mode, streamChangeInfo);
    return SUCCESS;
}

AudioStreamType AudioStreamCollector::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(std::make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    }

    if (streamType == STREAM_MEDIA) {
        streamType = STREAM_MUSIC;
    }

    return streamType;
}

AudioStreamType AudioStreamCollector::GetStreamType(int32_t sessionId)
{
    AudioStreamType streamType = STREAM_MUSIC;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->sessionId == sessionId) {
            streamType = GetStreamType(changeInfo->rendererInfo.contentType, changeInfo->rendererInfo.streamUsage);
        }
    }
    return streamType;
}

int32_t AudioStreamCollector::GetChannelCount(int32_t sessionId)
{
    int32_t channelCount = 0;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::unique_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it != audioRendererChangeInfos_.end()) {
        channelCount = (*it)->channelCount;
    }
    return channelCount;
}

int32_t AudioStreamCollector::GetCurrentRendererChangeInfos(
    vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        rendererChangeInfos.push_back(make_unique<AudioRendererChangeInfo>(*changeInfo));
    }
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos returned");

    return SUCCESS;
}

int32_t AudioStreamCollector::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioCapturerChangeInfos_) {
        capturerChangeInfos.push_back(make_unique<AudioCapturerChangeInfo>(*changeInfo));
        AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos returned");
    }

    return SUCCESS;
}

void AudioStreamCollector::RegisteredTrackerClientDied(int32_t uid)
{
    AUDIO_INFO_LOG("TrackerClientDied:client:%{public}d Died", uid);

    // Send the release state event notification for all streams of died client to registered app
    int32_t sessionID = -1;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    vector<std::unique_ptr<AudioRendererChangeInfo>>::iterator audioRendererBegin = audioRendererChangeInfos_.begin();
    while (audioRendererBegin != audioRendererChangeInfos_.end()) {
        const auto &audioRendererChangeInfo = *audioRendererBegin;
        if (audioRendererChangeInfo == nullptr ||
            (audioRendererChangeInfo->clientUID != uid && audioRendererChangeInfo->createrUID != uid)) {
            audioRendererBegin++;
            continue;
        }
        sessionID = audioRendererChangeInfo->sessionId;
        audioRendererChangeInfo->rendererState = RENDERER_RELEASED;
        WriteRenderStreamReleaseSysEvent(audioRendererChangeInfo);
        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
        }
        rendererStatequeue_.erase(make_pair(audioRendererChangeInfo->clientUID,
            audioRendererChangeInfo->sessionId));
        vector<std::unique_ptr<AudioRendererChangeInfo>>::iterator temp = audioRendererBegin;
        audioRendererBegin = audioRendererChangeInfos_.erase(temp);
        if ((sessionID != -1) && clientTracker_.erase(sessionID)) {
            AUDIO_DEBUG_LOG("TrackerClientDied:client %{public}d cleared", sessionID);
        }
    }

    sessionID = -1;
    vector<std::unique_ptr<AudioCapturerChangeInfo>>::iterator audioCapturerBegin = audioCapturerChangeInfos_.begin();
    while (audioCapturerBegin != audioCapturerChangeInfos_.end()) {
        const auto &audioCapturerChangeInfo = *audioCapturerBegin;
        if (audioCapturerChangeInfo == nullptr || audioCapturerChangeInfo->clientUID != uid) {
            audioCapturerBegin++;
            continue;
        }
        sessionID = audioCapturerChangeInfo->sessionId;
        audioCapturerChangeInfo->capturerState = CAPTURER_RELEASED;
        WriteCaptureStreamReleaseSysEvent(audioCapturerChangeInfo);
        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos_);
        }
        capturerStatequeue_.erase(make_pair(audioCapturerChangeInfo->clientUID,
            audioCapturerChangeInfo->sessionId));
        vector<std::unique_ptr<AudioCapturerChangeInfo>>::iterator temp = audioCapturerBegin;
        audioCapturerBegin = audioCapturerChangeInfos_.erase(temp);
        if ((sessionID != -1) && clientTracker_.erase(sessionID)) {
            AUDIO_DEBUG_LOG("TrackerClientDied:client %{public}d cleared", sessionID);
        }
    }
}

bool AudioStreamCollector::GetAndCompareStreamType(AudioStreamType requiredType, AudioRendererInfo rendererInfo)
{
    AudioStreamType defaultStreamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(rendererInfo.contentType, rendererInfo.streamUsage));
    if (pos != streamTypeMap_.end()) {
        defaultStreamType = pos->second;
    }
    return defaultStreamType == requiredType;
}

int32_t AudioStreamCollector::GetUid(int32_t sessionId)
{
    int32_t defaultUid = -1;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::unique_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it != audioRendererChangeInfos_.end()) {
        defaultUid = (*it)->createrUID;
    }
    return defaultUid;
}

int32_t AudioStreamCollector::UpdateStreamState(int32_t clientUid,
    StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->clientUID == clientUid &&
            GetAndCompareStreamType(streamSetStateEventInternal.audioStreamType, changeInfo->rendererInfo)) {
            AUDIO_INFO_LOG("UpdateStreamState Found matching uid and type");
            std::shared_ptr<AudioClientTracker> callback = clientTracker_[changeInfo->sessionId];
            if (callback == nullptr) {
                AUDIO_ERR_LOG("UpdateStreamState callback failed sId:%{public}d",
                    changeInfo->sessionId);
                continue;
            }
            if (streamSetStateEventInternal.streamSetState == StreamSetState::STREAM_PAUSE) {
                callback->PausedStreamImpl(streamSetStateEventInternal);
            } else if (streamSetStateEventInternal.streamSetState == StreamSetState::STREAM_RESUME) {
                callback->ResumeStreamImpl(streamSetStateEventInternal);
            }
        }
    }

    return SUCCESS;
}

bool AudioStreamCollector::IsStreamActive(AudioStreamType volumeType)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool result = false;
    for (auto &changeInfo: audioRendererChangeInfos_) {
        if (changeInfo->rendererState != RENDERER_RUNNING) {
            continue;
        }
        AudioStreamType rendererVolumeType = GetVolumeTypeFromContentUsage((changeInfo->rendererInfo).contentType,
            (changeInfo->rendererInfo).streamUsage);
        if (rendererVolumeType == volumeType) {
            // An active stream has been found, return true directly.
            return true;
        }
    }
    return result;
}

int32_t AudioStreamCollector::GetRunningStream(AudioStreamType certainType, int32_t certainChannelCount)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    int32_t runningStream = -1;
    if ((certainType == STREAM_DEFAULT) && (certainChannelCount == 0)) {
        for (auto &changeInfo : audioRendererChangeInfos_) {
            if (changeInfo->rendererState == RENDERER_RUNNING) {
                runningStream = changeInfo->sessionId;
                break;
            }
        }
    } else if (certainChannelCount == 0) {
        for (auto &changeInfo : audioRendererChangeInfos_) {
            if ((changeInfo->rendererState == RENDERER_RUNNING) &&
                    (certainType == GetStreamType(changeInfo->rendererInfo.contentType,
                    changeInfo->rendererInfo.streamUsage))) {
                runningStream = changeInfo->sessionId;
                break;
            }
        }
    } else {
        for (auto &changeInfo : audioRendererChangeInfos_) {
            if ((changeInfo->rendererState == RENDERER_RUNNING) &&
                    (certainType == GetStreamType(changeInfo->rendererInfo.contentType,
                    changeInfo->rendererInfo.streamUsage)) && (certainChannelCount == changeInfo->channelCount)) {
                runningStream = changeInfo->sessionId;
                break;
            }
        }
    }
    return runningStream;
}

AudioStreamType AudioStreamCollector::GetVolumeTypeFromContentUsage(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    }
    switch (streamType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_MESSAGE:
            return STREAM_VOICE_CALL;
        case STREAM_RING:
        case STREAM_SYSTEM:
        case STREAM_NOTIFICATION:
        case STREAM_SYSTEM_ENFORCED:
        case STREAM_DTMF:
            return STREAM_RING;
        case STREAM_MUSIC:
        case STREAM_MEDIA:
        case STREAM_MOVIE:
        case STREAM_GAME:
        case STREAM_SPEECH:
        case STREAM_NAVIGATION:
            return STREAM_MUSIC;
        case STREAM_VOICE_ASSISTANT:
            return STREAM_VOICE_ASSISTANT;
        case STREAM_ALARM:
            return STREAM_ALARM;
        case STREAM_ACCESSIBILITY:
            return STREAM_ACCESSIBILITY;
        case STREAM_ULTRASONIC:
            return STREAM_ULTRASONIC;
        default:
            return STREAM_MUSIC;
    }
}

AudioStreamType AudioStreamCollector::GetStreamTypeFromSourceType(SourceType sourceType)
{
    switch (sourceType) {
        case SOURCE_TYPE_MIC:
            return STREAM_MUSIC;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            return STREAM_VOICE_CALL;
        case SOURCE_TYPE_ULTRASONIC:
            return STREAM_ULTRASONIC;
        case SOURCE_TYPE_WAKEUP:
            return STREAM_WAKEUP;
        default:
            return STREAM_MUSIC;
    }
}

int32_t AudioStreamCollector::SetLowPowerVolume(int32_t streamId, float volume)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
        ERR_INVALID_PARAM, "SetLowPowerVolume streamId invalid.");
    std::shared_ptr<AudioClientTracker> callback = clientTracker_[streamId];
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        ERR_INVALID_PARAM, "SetLowPowerVolume callback failed");
    callback->SetLowPowerVolumeImpl(volume);
    return SUCCESS;
}

float AudioStreamCollector::GetLowPowerVolume(int32_t streamId)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    float ret = 1.0; // invalue volume
    CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
        ret, "GetLowPowerVolume streamId invalid.");
    float volume;
    std::shared_ptr<AudioClientTracker> callback = clientTracker_[streamId];
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        ret, "GetLowPowerVolume callback failed");
    callback->GetLowPowerVolumeImpl(volume);
    return volume;
}

int32_t AudioStreamCollector::SetOffloadMode(int32_t streamId, int32_t state, bool isAppBack)
{
    std::shared_ptr<AudioClientTracker> callback;
    {
        std::lock_guard<std::mutex> lock(streamsInfoMutex_);
        CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
            ERR_INVALID_PARAM, "streamId (%{public}d) invalid.", streamId);
        callback = clientTracker_[streamId];
        CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback failed");
    }
    callback->SetOffloadModeImpl(state, isAppBack);
    return SUCCESS;
}

int32_t AudioStreamCollector::UnsetOffloadMode(int32_t streamId)
{
    std::shared_ptr<AudioClientTracker> callback;
    {
        std::lock_guard<std::mutex> lock(streamsInfoMutex_);
        CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
            ERR_INVALID_PARAM, "streamId (%{public}d) invalid.", streamId);
        callback = clientTracker_[streamId];
        CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback failed");
    }
    callback->UnsetOffloadModeImpl();
    return SUCCESS;
}

float AudioStreamCollector::GetSingleStreamVolume(int32_t streamId)
{
    std::shared_ptr<AudioClientTracker> callback;
    {
        std::lock_guard<std::mutex> lock(streamsInfoMutex_);
        float ret = 1.0; // invalue volume
        CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
            ret, "GetSingleStreamVolume streamId invalid.");
        callback = clientTracker_[streamId];
        CHECK_AND_RETURN_RET_LOG(callback != nullptr,
            ret, "GetSingleStreamVolume callback failed");
    }
    float volume;
    callback->GetSingleStreamVolumeImpl(volume);
    return volume;
}

int32_t AudioStreamCollector::UpdateCapturerInfoMuteStatus(int32_t uid, bool muteStatus)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool capturerInfoUpdated = false;
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->clientUID == uid || uid == 0) {
            (*it)->muted = muteStatus;
            capturerInfoUpdated = true;
        }
    }

    if (capturerInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos_);
    }

    return SUCCESS;
}

void AudioStreamCollector::WriterStreamChangeSysEvent(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    bool isOutput = true;
    AudioStreamType streamType;
    uint64_t transactionId = 0;

    if (mode == AUDIO_MODE_PLAYBACK) {
        streamType = GetVolumeTypeFromContentUsage(streamChangeInfo.audioRendererChangeInfo.rendererInfo.contentType,
            streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage);
        transactionId = audioSystemMgr_->GetTransactionId(
            streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.deviceType, OUTPUT_DEVICE);

        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "STREAM_CHANGE",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ISOUTPUT", isOutput ? 1 : 0,
            "STREAMID", streamChangeInfo.audioRendererChangeInfo.sessionId,
            "UID", streamChangeInfo.audioRendererChangeInfo.clientUID,
            "PID", streamChangeInfo.audioRendererChangeInfo.clientPid,
            "TRANSACTIONID", transactionId,
            "STREAMTYPE", streamType,
            "STATE", streamChangeInfo.audioRendererChangeInfo.rendererState,
            "DEVICETYPE", streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.deviceType);
    } else {
        isOutput = false;
        streamType = GetStreamTypeFromSourceType(streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType);
        transactionId = audioSystemMgr_->GetTransactionId(
            streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.deviceType, INPUT_DEVICE);

        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "STREAM_CHANGE",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ISOUTPUT", isOutput ? 1 : 0,
            "STREAMID", streamChangeInfo.audioCapturerChangeInfo.sessionId,
            "UID", streamChangeInfo.audioCapturerChangeInfo.clientUID,
            "PID", streamChangeInfo.audioCapturerChangeInfo.clientPid,
            "TRANSACTIONID", transactionId,
            "STREAMTYPE", streamType,
            "STATE", streamChangeInfo.audioCapturerChangeInfo.capturerState,
            "DEVICETYPE", streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.deviceType);
    }
}


void AudioStreamCollector::WriteRenderStreamReleaseSysEvent(
    const std::unique_ptr<AudioRendererChangeInfo> &audioRendererChangeInfo)
{
    AudioStreamType streamType = GetVolumeTypeFromContentUsage(audioRendererChangeInfo->rendererInfo.contentType,
        audioRendererChangeInfo->rendererInfo.streamUsage);
    uint64_t transactionId = audioSystemMgr_->GetTransactionId(
        audioRendererChangeInfo->outputDeviceInfo.deviceType, OUTPUT_DEVICE);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "STREAM_CHANGE",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ISOUTPUT", 1,
        "STREAMID", audioRendererChangeInfo->sessionId,
        "UID", audioRendererChangeInfo->clientUID,
        "PID", audioRendererChangeInfo->clientPid,
        "TRANSACTIONID", transactionId,
        "STREAMTYPE", streamType,
        "STATE", audioRendererChangeInfo->rendererState,
        "DEVICETYPE", audioRendererChangeInfo->outputDeviceInfo.deviceType);
}

void AudioStreamCollector::WriteCaptureStreamReleaseSysEvent(
    const std::unique_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfo)
{
    AudioStreamType streamType = GetStreamTypeFromSourceType(audioCapturerChangeInfo->capturerInfo.sourceType);
    uint64_t transactionId = audioSystemMgr_->GetTransactionId(
        audioCapturerChangeInfo->inputDeviceInfo.deviceType, INPUT_DEVICE);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "STREAM_CHANGE",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ISOUTPUT", 0,
        "STREAMID", audioCapturerChangeInfo->sessionId,
        "UID", audioCapturerChangeInfo->clientUID,
        "PID", audioCapturerChangeInfo->clientPid,
        "TRANSACTIONID", transactionId,
        "STREAMTYPE", streamType,
        "STATE", audioCapturerChangeInfo->capturerState,
        "DEVICETYPE", audioCapturerChangeInfo->inputDeviceInfo.deviceType);
}
} // namespace AudioStandard
} // namespace OHOS
