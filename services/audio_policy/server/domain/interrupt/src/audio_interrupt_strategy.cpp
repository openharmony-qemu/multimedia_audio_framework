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
#define LOG_TAG "AudioInterruptStrategy"
#endif

#include "audio_interrupt_strategy.h"
#include "audio_log.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
AudioInterruptStrategy::AudioInterruptStrategy()
    : sessionService_(OHOS::Singleton<AudioSessionService>::GetInstance())
{}

bool AudioInterruptStrategy::IsMicSource(SourceType sourceType)
{
    return (sourceType == SOURCE_TYPE_VOICE_CALL ||
            sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION ||
            sourceType == SOURCE_TYPE_VOICE_COMMUNICATION);
}

void AudioInterruptStrategy::UpdateFocusStrategy(const std::string &bundleName,
    AudioFocusEntry &focusEntry, bool isExistMediaStream, bool isIncomingMediaStream)
{
    bool ret = false;
    if (queryBundleNameListCallback_ != nullptr) {
        queryBundleNameListCallback_->OnQueryBundleNameIsInList(bundleName, "audio_param", ret);
    }
    if (isExistMediaStream && isIncomingMediaStream && ret &&
        focusEntry.hintType == INTERRUPT_HINT_STOP) {
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        AUDIO_INFO_LOG("%{public}s update audio focus strategy", bundleName.c_str());
    }
}

void AudioInterruptStrategy::UpdateMapFocusStrategy(const std::string &bundleName,
    AudioFocusEntry &focusEntry, bool isExistMediaStream, SourceType incomingSourceType)
{
    bool isBundleNameExist = false;
    if (queryBundleNameListCallback_ != nullptr) {
        queryBundleNameListCallback_->OnQueryBundleNameIsInList((bundleName + ".audiointerrupt"), "audio_param",
            isBundleNameExist);
    }
    if (isExistMediaStream && incomingSourceType == SOURCE_TYPE_MIC && isBundleNameExist &&
        focusEntry.hintType == INTERRUPT_HINT_PAUSE) {
        focusEntry.hintType = INTERRUPT_HINT_STOP;
        AUDIO_INFO_LOG("%{public}s update Map audio focus strategy", bundleName.c_str());
    }
}

void AudioInterruptStrategy::UpdateMicFocusStrategy(const AudioFocusType &existAudioFocusType,
    const AudioFocusType &incomingAudioFocusType, const std::string &currentBundleName,
    const std::string &incomingBundleName, AudioFocusEntry &focusEntry)
{
    if (queryBundleNameListCallback_ == nullptr) {
        AUDIO_INFO_LOG("Not a recording stream access");
        return;
    }
    bool isCurrentBundleNameExist = false;
    bool isIncomingBundleNameExist = false;
    queryBundleNameListCallback_->OnQueryBundleNameIsInList(currentBundleName, "audio_micfocus_list",
        isCurrentBundleNameExist);
    queryBundleNameListCallback_->OnQueryBundleNameIsInList(incomingBundleName, "audio_micfocus_list",
        isIncomingBundleNameExist);
    AudioStreamType existStreamType = existAudioFocusType.streamType;
    AudioStreamType incomingStreamType = incomingAudioFocusType.streamType;
    SourceType existSourceType = existAudioFocusType.sourceType;
    SourceType incomingSourceType = incomingAudioFocusType.sourceType;
    AUDIO_INFO_LOG("%{public}s update mic focus strategy, focusEntry.hintType: %{public}d,"
        " focusEntry.actionOn: %{public}d"
        " existSourceType: %{public}d  incomingSourceType: %{public}d"
        " existStreamType: %{public}d incomingStreamType: %{public}d"
        " isCurrentBundleNameExist: %{public}d  isIncomingBundleNameExist: %{public}d",
        currentBundleName.c_str(), focusEntry.hintType, focusEntry.actionOn, existSourceType, incomingSourceType,
        existStreamType, incomingStreamType,
        isCurrentBundleNameExist, isIncomingBundleNameExist);
    if (existSourceType == SOURCE_TYPE_MIC &&
        (IsMicSource(incomingSourceType) || incomingStreamType == STREAM_VOICE_CALL) &&
        isCurrentBundleNameExist) {
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        focusEntry.actionOn = CURRENT;
        AUDIO_INFO_LOG("current %{public}s update mic focus strategy", currentBundleName.c_str());
        return;
    }
    if ((IsMicSource(existSourceType) || existStreamType == STREAM_VOICE_CALL) &&
        incomingSourceType == SOURCE_TYPE_MIC &&
        isIncomingBundleNameExist) {
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        focusEntry.actionOn = INCOMING;
        AUDIO_INFO_LOG("incoming %{public}s update mic focus strategy", incomingBundleName.c_str());
        return;
    }
}

void AudioInterruptStrategy::UpdateMicFocusByUid(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    int32_t uid = incomingInterrupt.uid;
    std::string bundleName = AudioInterruptUtils::GetAudioInterruptBundleName(incomingInterrupt);
    std::string currentBundleName = AudioInterruptUtils::GetAudioInterruptBundleName(currentInterrupt);
    AudioFocusType existAudioFocusType = currentInterrupt.audioFocusType;
    AudioFocusType incomingAudioFocusType = incomingInterrupt.audioFocusType;
    if (uid == static_cast<int32_t>(AUDIO_ID)) {
        AUDIO_INFO_LOG("lake app:%{public}s access", std::to_string(uid).c_str());
        UpdateMicFocusStrategy(existAudioFocusType, incomingAudioFocusType, std::to_string(uid),
            bundleName, focusEntry);
    } else {
        UpdateMicFocusStrategy(existAudioFocusType, incomingAudioFocusType, currentBundleName,
            bundleName, focusEntry);
    }
}

void AudioInterruptStrategy::UpdateWindowFocusStrategy(const int32_t &currentPid, const int32_t &incomingPid,
    const AudioStreamType &existStreamType, const AudioStreamType &incomingStreamType, AudioFocusEntry &focusEntry)
{
#ifdef HAS_WINDOW_MANAGER
    if (currentPid == incomingPid) {
        return;
    }
    if ((existStreamType != STREAM_MUSIC &&
        existStreamType != STREAM_MOVIE && existStreamType != STREAM_SPEECH) ||
        (incomingStreamType != STREAM_MUSIC && incomingStreamType != STREAM_MOVIE &&
        incomingStreamType != STREAM_SPEECH)) {
        return;
    }

    bool isCurTargetWindowState = WindowUtils::CheckWindowState(currentPid);
    bool isIncomingTargetWindowState = WindowUtils::CheckWindowState(incomingPid);
    if (isCurTargetWindowState && isIncomingTargetWindowState) {
        AUDIO_INFO_LOG("The media windowStates concurrent");
        focusEntry.hintType = INTERRUPT_HINT_NONE;
        focusEntry.actionOn = INCOMING;
    }
#endif
}

void AudioInterruptStrategy::UpdateMuteAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    if (currentInterrupt.strategy == InterruptStrategy::DEFAULT &&
        incomingInterrupt.strategy == InterruptStrategy::DEFAULT &&
        !(currentInterrupt.behaviorFlags_.muteWhenInterruptFlag && focusEntry.actionOn == CURRENT) &&
        !(incomingInterrupt.behaviorFlags_.muteWhenInterruptFlag && focusEntry.actionOn == INCOMING)) {
        return;
    }

    if ((focusEntry.hintType != INTERRUPT_HINT_STOP &&
        focusEntry.hintType != INTERRUPT_HINT_PAUSE) ||
        incomingInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP) {
        AUDIO_INFO_LOG("streamId: %{public}u, keep current hintType=%{public}d",
            currentInterrupt.streamId, focusEntry.hintType);
        return;
    }

    if (currentInterrupt.strategy == InterruptStrategy::MUTE) {
        focusEntry.actionOn = CURRENT;
    }
    if (incomingInterrupt.strategy == InterruptStrategy::MUTE) {
        focusEntry.actionOn = INCOMING;
    }

    AUDIO_INFO_LOG("currentStreamId:%{public}u, incomingStreamId:%{public}u, action:%{public}u",
        currentInterrupt.streamId, incomingInterrupt.streamId, focusEntry.actionOn);
    focusEntry.isReject = false;
    focusEntry.hintType = INTERRUPT_HINT_MUTE;
}

void AudioInterruptStrategy::UpdateInterPhoneStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    if (AudioInterruptUtils::IsInterPhoneInterrupt(currentInterrupt) &&
        AudioInterruptUtils::IsVoipInterrupt(incomingInterrupt)) {
        focusEntry.actionOn = CURRENT;
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        AUDIO_INFO_LOG("currentStreamId:%{public}u, incomingStreamId:%{public}u, action:%{public}u",
            currentInterrupt.streamId, incomingInterrupt.streamId, focusEntry.actionOn);
    } else if (AudioInterruptUtils::IsInterPhoneInterrupt(incomingInterrupt) &&
        AudioInterruptUtils::IsVoipInterrupt(currentInterrupt)) {
        focusEntry.actionOn = INCOMING;
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        AUDIO_INFO_LOG("currentStreamId:%{public}u, incomingStreamId:%{public}u, action:%{public}u",
            currentInterrupt.streamId, incomingInterrupt.streamId, focusEntry.actionOn);
    }
}

void AudioInterruptStrategy::UpdateAIBaseFocusStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    if (((currentInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION) ||
        (currentInterrupt.audioFocusType.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION)) &&
        incomingInterrupt.behaviorFlags_.aibaseMuteVoip) {
        AUDIO_INFO_LOG("Input method MIC mutes VoIP stream due to AI-based focus strategy");
        focusEntry.actionOn = CURRENT;
        focusEntry.hintType = INTERRUPT_HINT_MUTE;
        focusEntry.isReject = false;
    }
}

void AudioInterruptStrategy::UpdateAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    int32_t uid = incomingInterrupt.uid;
    int32_t currentPid = currentInterrupt.pid;
    int32_t currentUid = currentInterrupt.uid;
    int32_t incomingPid = incomingInterrupt.pid;
    AudioFocusType incomingAudioFocusType = incomingInterrupt.audioFocusType;
    AudioFocusType existAudioFocusType = currentInterrupt.audioFocusType;
    std::string bundleName = AudioInterruptUtils::GetAudioInterruptBundleName(incomingInterrupt);
    std::string currentBundleName = AudioInterruptUtils::GetAudioInterruptBundleName(currentInterrupt);
    CHECK_AND_RETURN_LOG(!bundleName.empty(), "bundleName is empty");
    AudioStreamType existStreamType = existAudioFocusType.streamType;
    AudioStreamType incomingStreamType = incomingAudioFocusType.streamType;
    SourceType existSourceType = existAudioFocusType.sourceType;
    SourceType incomingSourceType = incomingAudioFocusType.sourceType;
    UpdateFocusStrategy(bundleName, focusEntry, AudioInterruptUtils::IsMediaStream(existStreamType),
        AudioInterruptUtils::IsMediaStream(incomingStreamType));
    UpdateMapFocusStrategy(bundleName, focusEntry, AudioInterruptUtils::IsMediaStream(existStreamType),
        incomingSourceType);
    UpdateMediaWithMixStrategy(currentInterrupt, incomingInterrupt, focusEntry);
    UpdateMediaWithDuckStrategy(currentInterrupt, incomingInterrupt, focusEntry);
    UpdateMicFocusByUid(currentInterrupt, incomingInterrupt, focusEntry);
    UpdateWindowFocusStrategy(currentPid, incomingPid, existStreamType, incomingStreamType, focusEntry);
    UpdateMuteAudioFocusStrategy(currentInterrupt, incomingInterrupt, focusEntry);
    UpdateInterPhoneStrategy(currentInterrupt, incomingInterrupt, focusEntry);
    UpdateAIBaseFocusStrategy(currentInterrupt, incomingInterrupt, focusEntry);
}

void AudioInterruptStrategy::UpdateMediaWithDuckStrategy(const AudioInterrupt &activeInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    if (!AudioInterruptUtils::IsMediaStream(activeInterrupt.audioFocusType.streamType) ||
        !AudioInterruptUtils::IsMediaStream(incomingInterrupt.audioFocusType.streamType)) {
        return;
    }
    AudioConcurrencyMode concurrencyMode = AudioConcurrencyMode::INVALID;
    if (sessionService_.IsAudioSessionActivated(incomingInterrupt.pid)) {
        concurrencyMode = sessionService_.GetSessionStrategy(incomingInterrupt.pid);
    }
    if (concurrencyMode == AudioConcurrencyMode::DUCK_OTHERS && sessionService_.IsSystemApp(incomingInterrupt.pid)) {
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
        focusEntry.actionOn = CURRENT;
        return;
    }
 
    concurrencyMode = AudioConcurrencyMode::INVALID;
    if (sessionService_.IsAudioSessionActivated(activeInterrupt.pid)) {
        concurrencyMode = sessionService_.GetSessionStrategy(activeInterrupt.pid);
    }
    if (concurrencyMode == AudioConcurrencyMode::DUCK_OTHERS && sessionService_.IsSystemApp(activeInterrupt.pid)) {
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
        focusEntry.actionOn = INCOMING;
        return;
    }
}
 
bool AudioInterruptStrategy::IsMediaWithMixStrategy(const AudioInterrupt &currentInterrupt)
{
    CHECK_AND_RETURN_RET_LOG(AudioInterruptUtils::IsMediaStream(currentInterrupt.audioFocusType.streamType), false,
        "current stream not media stream");
 
    AudioConcurrencyMode concurrencyMode = AudioConcurrencyMode::INVALID;
    if (sessionService_.IsAudioSessionActivated(currentInterrupt.pid)) {
        concurrencyMode = sessionService_.GetSessionStrategy(currentInterrupt.pid);
    }
    if (currentInterrupt.independentStrategy.concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS ||
        concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS) {
        return true;
    }
    return false;
}
 
bool AudioInterruptStrategy::IsCanMixwithMedia(const AudioInterrupt &currentInterrupt)
{
    AudioStreamType incomingStreamType = currentInterrupt.audioFocusType.streamType;
    SourceType incomingSourceType = currentInterrupt.audioFocusType.sourceType;
    bool isMixStream = (incomingStreamType == STREAM_ALARM);
    bool isMixSource = (incomingSourceType == SOURCE_TYPE_MIC || incomingSourceType == SOURCE_TYPE_CAMCORDER ||
        incomingSourceType == SOURCE_TYPE_VOICE_MESSAGE || incomingSourceType == SOURCE_TYPE_LIVE);
    return (isMixStream || isMixSource);
}

void AudioInterruptStrategy::UpdateMediaWithMixStrategy(const AudioInterrupt &activeInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    if ((IsMediaWithMixStrategy(incomingInterrupt) && IsCanMixwithMedia(activeInterrupt)) ||
        (IsMediaWithMixStrategy(activeInterrupt) && IsCanMixwithMedia(incomingInterrupt))) {
        focusEntry.hintType = INTERRUPT_HINT_NONE;
        focusEntry.actionOn = INCOMING;
        AUDIO_INFO_LOG("Two streams can mix because of the active session,"
            "activeStreamId:%{public}u, incomingStreamId:%{public}u, action:%{public}u",
            activeInterrupt.streamId, incomingInterrupt.streamId, focusEntry.actionOn);
        return;
    }
}

int32_t AudioInterruptStrategy::SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object)
{
    AUDIO_INFO_LOG("Set query bundle name list callback");
    queryBundleNameListCallback_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    if (queryBundleNameListCallback_ == nullptr) {
        AUDIO_ERR_LOG("query bundle name list callback is null");
        return ERR_CALLBACK_NOT_REGISTERED;
    }
    return SUCCESS;
}

void AudioInterruptStrategy::OnQueryBundleNameIsInList(const std::string &bundleName,
    const std::string &params, bool& ret)
{
    if (queryBundleNameListCallback_ != nullptr) {
        queryBundleNameListCallback_->OnQueryBundleNameIsInList(bundleName, params, ret);
    }
}

} // namespace AudioStandard
} // namespace OHOS
