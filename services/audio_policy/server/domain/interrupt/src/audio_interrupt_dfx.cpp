/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioInterruptDfx"
#endif

#include "audio_info.h"
#include "audio_interrupt_dfx.h"
#include "audio_interrupt_utils.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "dfx_msg_manager.h"
#include "media_monitor_manager.h"
#include "app_bundle_manager.h"

namespace OHOS {
namespace AudioStandard {

const int64_t LONG_PLAYBACK_TIME_NS = 3LL * 60 * 1000 * 1000 * 1000;

AudioInterruptDfx::AudioInterruptDfx()
    : sessionService_(OHOS::Singleton<AudioSessionService>::GetInstance())
{}

void AudioInterruptDfx::WriteAudioInterruptErrorEvent(const AudioFocusErrorEvent &interruptError)
{
    AUDIO_INFO_LOG("WriteAudioInterruptErrorEvent begin");
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::INTERRUPT_ERROR,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("APP_NAME", interruptError.appName);
    bean->Add("ERROR_INFO", interruptError.errorInfo);
    bean->Add("INTERRUPT_HINTTYPE", static_cast<int32_t>(interruptError.hintType));
    bean->Add("RENDERER_INFO", interruptError.rendererInfo);
    bean->Add("SESSION_INFO", interruptError.audiosessionInfo);
    bean->Add("WINDOW_STATE", interruptError.isAppInForeground);
    bean->Add("RENDERER_PLAY_TIMES", interruptError.rendererPlayTimes);
    bean->Add("CURR_APP_NAME", interruptError.interruptedAppName);
    bean->Add("CURR_RENDERER_INFO", interruptError.interruptedRendererInfo);
    bean->Add("CURR_SESSION_INFO", interruptError.interruptedAudiosessionInfo);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioInterruptDfx::ActivateAudioSessionErrorEvent(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList, const int32_t callerPid)
{
    AudioFocusErrorEvent interruptError;
    for (const auto& item : audioFocusInfoList) {
        if (item.first.pid == callerPid && item.first.audioFocusType.sourceType == SOURCE_TYPE_INVALID) {
            interruptError.rendererInfo += "streamType: " + std::to_string(item.first.audioFocusType.streamType) +
                " sourceType: " + std::to_string(item.first.audioFocusType.sourceType) + ";";
            if (interruptError.appName.empty()) {
                interruptError.appName = AudioInterruptUtils::GetAudioInterruptBundleName(item.first);
            }
        }
    }
    if (!interruptError.rendererInfo.empty()) {
        interruptError.isAppInForeground = AudioInterruptUtils::GetAppState(callerPid);
        interruptError.errorInfo = "StartStreamAbnormal";
        interruptError.audiosessionInfo = "Scene:" +
            std::to_string(sessionService_.GenerateFakeAudioInterrupt(callerPid).audioFocusType.streamType);
        interruptError.audiosessionInfo += " concurrencyMode:" +
            std::to_string(static_cast<int32_t>(sessionService_.GetSessionStrategy(callerPid)));
        WriteAudioInterruptErrorEvent(interruptError);
    }
}

void AudioInterruptDfx::DeactivateAudioSessionErrorEvent(
    const std::vector<AudioInterrupt> &streamsInSession, const int32_t callerPid)
{
    AudioFocusErrorEvent interruptError;
    for (const auto& item : streamsInSession) {
        if (item.pid == callerPid) {
            interruptError.rendererInfo += "streamType: " + std::to_string(item.audioFocusType.streamType) +
                " sourceType: " + std::to_string(item.audioFocusType.sourceType) + ";";
            if (interruptError.appName.empty()) {
                interruptError.appName = AudioInterruptUtils::GetAudioInterruptBundleName(item);
            }
        }
    }
    if (!interruptError.rendererInfo.empty()) {
        interruptError.isAppInForeground = AudioInterruptUtils::GetAppState(callerPid);
        interruptError.errorInfo = "StopStreamAbnormal";
        interruptError.audiosessionInfo = "Scene:" +
            std::to_string(sessionService_.GenerateFakeAudioInterrupt(callerPid).audioFocusType.streamType);
        interruptError.audiosessionInfo += " concurrencyMode:" +
            std::to_string(static_cast<int32_t>(sessionService_.GetSessionStrategy(callerPid)));
        WriteAudioInterruptErrorEvent(interruptError);
    }
}

bool AudioInterruptDfx::IsInterruptErrorEvent(AudioStreamType sceneStreamType, AudioStreamType incomingStreamType)
{
    if (sceneStreamType == STREAM_MUSIC &&
        (incomingStreamType == STREAM_VOICE_COMMUNICATION ||
        incomingStreamType == STREAM_RING || incomingStreamType == STREAM_GAME)) {
        AUDIO_INFO_LOG("IsInterruptErrorEvent STREAM_MUSIC");
        return true;
    }
    if (sceneStreamType == STREAM_VOICE_COMMUNICATION && incomingStreamType == STREAM_GAME) {
        AUDIO_INFO_LOG("IsInterruptErrorEvent STREAM_VOICE_COMMUNICATION");
        return true;
    }
    return false;
}

void AudioInterruptDfx::AddInterruptErrorEvent(const AudioInterrupt &audioInterrupt, const int32_t callerPid)
{
    AudioFocusErrorEvent interruptError;
    AudioStreamType sceneStreamType = sessionService_.GenerateFakeAudioInterrupt(callerPid).audioFocusType.streamType;
    HILOG_COMM_INFO("sceneStreamType: %{public}d  audioInterrupt.audioFocusType.streamType: %{public}d",
        sceneStreamType, audioInterrupt.audioFocusType.streamType);
    if (IsInterruptErrorEvent(sceneStreamType, audioInterrupt.audioFocusType.streamType)) {
        interruptError.appName = AudioInterruptUtils::GetAudioInterruptBundleName(audioInterrupt);
        interruptError.isAppInForeground = AudioInterruptUtils::GetAppState(callerPid);
        interruptError.rendererInfo += "streamType: " + std::to_string(audioInterrupt.audioFocusType.streamType) +
            " sourceType: " + std::to_string(audioInterrupt.audioFocusType.sourceType);
        interruptError.errorInfo = "SceneAbnormal";
        interruptError.audiosessionInfo = "Scene:" +
            std::to_string(sessionService_.GenerateFakeAudioInterrupt(callerPid).audioFocusType.streamType);
        interruptError.audiosessionInfo += " concurrencyMode:" +
            std::to_string(static_cast<int32_t>(sessionService_.GetSessionStrategy(callerPid)));
        WriteAudioInterruptErrorEvent(interruptError);
    }
}

std::string AudioInterruptDfx::GetInterruptRendererInfo(const AudioInterrupt &audioInterrupt)
{
    std::string rendererInfoStr = "";
    AppendFormat(rendererInfoStr, "streamType : %d, streamid : %d", audioInterrupt.audioFocusType.streamType,
        audioInterrupt.streamId);
    return rendererInfoStr;
}

std::string AudioInterruptDfx::GetInterruptSessionInfo(const AudioInterrupt &audioInterrupt)
{
    std::string sesssionInfoStr = "";
    AudioSessionService &sessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    if (!sessionService.IsAudioSessionActivated(audioInterrupt.pid)) {
        return sesssionInfoStr;
    }
    if (sessionService.IsAudioSessionFocusMode(audioInterrupt.pid)) {
        AppendFormat(sesssionInfoStr, "audioSessionVersion: v2, concurrencyMode : %d, Scene : %d",
            sessionService.GetSessionStrategy(audioInterrupt.pid),
            sessionService.GenerateFakeAudioInterrupt(audioInterrupt.pid).audioFocusType.streamType);
    } else {
        AppendFormat(sesssionInfoStr, "audioSessionVersion: v1, concurrencyMode : %d",
            sessionService.GetSessionStrategy(audioInterrupt.pid));
    }
    return sesssionInfoStr;
}

void AudioInterruptDfx::RecordInterruptEvent(const InterruptEventInternal &interruptEvent,
    const AudioInterrupt &activeInterrupt, const AudioInterrupt &incomingInterrupt)
{
    RecordMediaInterruptEvent(interruptEvent, activeInterrupt, incomingInterrupt);
}

bool AudioInterruptDfx::IsCheckDfxStream(AudioStreamType audioStreamType)
{
    if (audioStreamType == STREAM_MUSIC || audioStreamType == STREAM_MOVIE ||
        audioStreamType == STREAM_SPEECH || audioStreamType == STREAM_NOTIFICATION ||
        audioStreamType == STREAM_VOICE_MESSAGE || audioStreamType == STREAM_NAVIGATION) {
        return true;
    }
    return false;
}

void AudioInterruptDfx::RecordMediaInterruptEvent(const InterruptEventInternal &interruptEvent,
    const AudioInterrupt &activeInterrupt, const AudioInterrupt &incomingInterrupt)
{
    AudioStreamType activeStreamType = activeInterrupt.audioFocusType.streamType;
    AudioStreamType incomingStreamType = incomingInterrupt.audioFocusType.streamType;
    if (!IsCheckDfxStream(activeStreamType) || !IsCheckDfxStream(incomingStreamType)) {
        return;
    }
    if (interruptEvent.hintType != INTERRUPT_HINT_PAUSE && interruptEvent.hintType != INTERRUPT_HINT_STOP) {
        return;
    }
    if (activeInterrupt.pid == incomingInterrupt.pid) {
        return;
    }
    if (sessionService_.IsAudioSessionFocusMode(incomingInterrupt.pid)) {
        return;
    }
    AudioFocusErrorEvent interruptErrorEvent;
    interruptErrorEvent.appName = AudioInterruptUtils::GetAudioInterruptBundleName(incomingInterrupt);
    interruptErrorEvent.hintType = interruptEvent.hintType;
    interruptErrorEvent.rendererInfo = GetInterruptRendererInfo(incomingInterrupt);
    interruptErrorEvent.audiosessionInfo = GetInterruptSessionInfo(incomingInterrupt);
    interruptErrorEvent.isAppInForeground =  AudioInterruptUtils::GetAppState(incomingInterrupt.pid);
    interruptErrorEvent.interruptedAppName = AudioInterruptUtils::GetAudioInterruptBundleName(activeInterrupt);
    interruptErrorEvent.interruptedRendererInfo = GetInterruptRendererInfo(activeInterrupt);
    interruptErrorEvent.interruptedAudiosessionInfo = GetInterruptSessionInfo(activeInterrupt);

    std::lock_guard<std::mutex> lock(cachedFocusMutex_);
    if (audioInterruptCheckInfo_.count(activeInterrupt.streamId) > 0) {
        audioInterruptCheckInfo_.erase(activeInterrupt.streamId);
    }
    auto streamId = incomingInterrupt.streamId;
    if (audioInterruptCheckInfo_.count(streamId) > 0) {
        audioInterruptCheckInfo_[streamId].errorEvents.push_back(interruptErrorEvent);
    } else {
        AudioInterruptCheckInfo interruptCheckInfo;
        interruptCheckInfo.isMuteStream = true;
        interruptCheckInfo.startTime = std::chrono::steady_clock::now();
        interruptCheckInfo.errorEvents.push_back(interruptErrorEvent);
        audioInterruptCheckInfo_[streamId] = interruptCheckInfo;
    }
}

void AudioInterruptDfx::NotifyStreamSilentChange(uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(cachedFocusMutex_);
    if (audioInterruptCheckInfo_.count(streamId) > 0) {
        audioInterruptCheckInfo_[streamId].isMuteStream = false;
    }
}

void AudioInterruptDfx::CheckMuteInterruptEvent(uint32_t streamId)
{
    if (audioInterruptCheckInfo_.count(streamId) > 0) {
        auto interruptCheckInfo = audioInterruptCheckInfo_[streamId];
        if (interruptCheckInfo.isMuteStream) {
            AUDIO_ERR_LOG("Is MuteStream %{public}d", streamId);
            for (auto interruptError : interruptCheckInfo.errorEvents) {
                interruptError.errorInfo = "MuteInterruptEvent";
                WriteAudioInterruptErrorEvent(interruptError);
            }
        }
    }
}

void AudioInterruptDfx::CheckShortInterruptEvent(uint32_t streamId)
{
    if (audioInterruptCheckInfo_.count(streamId) > 0) {
        auto interruptCheckInfo = audioInterruptCheckInfo_[streamId];
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - interruptCheckInfo.startTime < std::chrono::seconds(1)) {
            AUDIO_ERR_LOG("Is ShortStream %{public}d", streamId);
            for (auto interruptError : interruptCheckInfo.errorEvents) {
                interruptError.errorInfo = "ShortInterruptEvent";
                WriteAudioInterruptErrorEvent(interruptError);
            }
        }
    }
}

void AudioInterruptDfx::CheckInterruptEvent(uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(cachedFocusMutex_);
    CheckShortInterruptEvent(streamId);
    CheckMuteInterruptEvent(streamId);
    audioInterruptCheckInfo_.erase(streamId);
}

void AudioInterruptDfx::DelayCheckMuteStreamInterrupt(uint32_t streamId)
{
    if (audioInterruptCheckInfo_.count(streamId) <= 0) {
        return;
    }
    auto audioInterruptDfx = shared_from_this();
    auto checkTask = [audioInterruptDfx, streamId] {
        if (audioInterruptDfx == nullptr) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
        audioInterruptDfx->CheckInterruptEvent(streamId);
    };

    std::thread(checkTask).detach();
    AUDIO_INFO_LOG("Started Check MuteStream %{public}d with 10s delay", streamId);
}

void AudioInterruptDfx::HandleInterruptCallbackEvent(const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN(audioInterrupt.isAudioSessionInterrupt == false);
    if (audioInterrupt.audioFocusType.streamType == STREAM_MUSIC ||
        audioInterrupt.audioFocusType.streamType == STREAM_MOVIE ||
        audioInterrupt.audioFocusType.streamType == STREAM_SPEECH) {
        if (!audioInterrupt.registerCallbackFlag) {
            int64_t stamp = ClockTime::GetCurNano() - audioInterrupt.startTimeNs;
            if (stamp > LONG_PLAYBACK_TIME_NS && PermissionUtil::IsHap()) {
                WriteInterruptOrSessionCallbackErrorEvent(audioInterrupt.audioFocusType.streamType, audioInterrupt.uid,
                    TYPE_MEDIA_PLAYBACK_NOT_LISTENING_INTERRUPT_CHANGE);
            }
        }
        if (audioInterrupt.registerCallbackFlag && audioInterrupt.activatedRegisterCallbackFlag) {
            int64_t stamp = ClockTime::GetCurNano() - audioInterrupt.startTimeNs;
            if (stamp > LONG_PLAYBACK_TIME_NS && PermissionUtil::IsHap()) {
                WriteInterruptOrSessionCallbackErrorEvent(audioInterrupt.audioFocusType.streamType, audioInterrupt.uid,
                    TYPE_MEDIA_PLAYBACK_ACTIVATED_LISTENING_INTERRUPT_CHANGE);
            }
        }
    }
}

void AudioInterruptDfx::HandleStateChangeCallbackEvent(int32_t usage, int32_t uid, int32_t type)
{
    WriteInterruptOrSessionCallbackErrorEvent(usage, uid, type);
}

void AudioInterruptDfx::WriteInterruptOrSessionCallbackErrorEvent(int32_t usage, int32_t uid, int32_t type)
{
    std::string appName = AppBundleManager::GetBundleNameFromUid(uid);
    AUDIO_ERR_LOG("APP_NAME:%{public}s, STREAM_TYPE:%{public}d, TYPE:%{public}d", appName.c_str(), usage, type);
    AudioFocusErrorEvent interruptError;
    interruptError.appName = appName;
    interruptError.rendererInfo += "streamType: " + std::to_string(usage) +
        " type: " + std::to_string(type);
    interruptError.errorInfo = "MisuseOfInterfaces";
    WriteAudioInterruptErrorEvent(interruptError);
}
} // namespace AudioStandard
} // namespace OHOS
