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
#include "window_utils.h"

namespace OHOS {
namespace AudioStandard {

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

} // namespace AudioStandard
} // namespace OHOS
