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

#ifndef LOG_TAG
#define LOG_TAG "AudioVolume"
#endif

#include <numeric>
#include "audio_volume.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "audio_utils_c.h"
#include "audio_stream_info.h"
#include "media_monitor_manager.h"
#include "audio_stream_monitor.h"
#include "audio_mute_factor_manager.h"
#include "volume_tools.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {
static const std::unordered_map<std::string, AudioStreamType> STREAM_TYPE_STRING_ENUM_MAP = {
    {"voice_call", STREAM_VOICE_CALL},
    {"voice_call_assistant", STREAM_VOICE_CALL_ASSISTANT},
    {"music", STREAM_MUSIC},
    {"ring", STREAM_RING},
    {"media", STREAM_MEDIA},
    {"voice_assistant", STREAM_VOICE_ASSISTANT},
    {"system", STREAM_SYSTEM},
    {"alarm", STREAM_ALARM},
    {"notification", STREAM_NOTIFICATION},
    {"bluetooth_sco", STREAM_BLUETOOTH_SCO},
    {"enforced_audible", STREAM_ENFORCED_AUDIBLE},
    {"dtmf", STREAM_DTMF},
    {"tts", STREAM_TTS},
    {"accessibility", STREAM_ACCESSIBILITY},
    {"recording", STREAM_RECORDING},
    {"movie", STREAM_MOVIE},
    {"game", STREAM_GAME},
    {"speech", STREAM_SPEECH},
    {"system_enforced", STREAM_SYSTEM_ENFORCED},
    {"ultrasonic", STREAM_ULTRASONIC},
    {"wakeup", STREAM_WAKEUP},
    {"voice_message", STREAM_VOICE_MESSAGE},
    {"navigation", STREAM_NAVIGATION},
#ifdef MULTI_ALARM_LEVEL
    {"announcement", STREAM_ANNOUNCEMENT},
    {"emergency", STREAM_EMERGENCY},
#endif
};

uint64_t DURATION_TIME_DEFAULT = 40;
uint64_t DURATION_TIME_SHORT = 10;
static const float DEFAULT_APP_VOLUME = 1.0f;
constexpr int32_t DEFAULT_APP_VOLUME_LEVEL = 100;
uint32_t VOIP_CALL_VOICE_SERVICE = 1001;
uint32_t DISTURB_STATE_VOLUME_MUTE = 0;
uint32_t DISTURB_STATE_VOLUME_UNMUTE = 1;
static constexpr float AUDIO_VOLUME_EPSILON = 0.0001;

AudioVolume *AudioVolume::GetInstance()
{
    static AudioVolume instance;
    return &instance;
}

AudioVolume::AudioVolume()
{
    AUDIO_INFO_LOG("AudioVolume construct");
}

AudioVolume::~AudioVolume()
{
}

int32_t AudioVolume::GetStreamAndPipeVolume(uint32_t sessionId, int32_t streamType, uint32_t pipeId,
    std::shared_ptr<StreamVolume> &streamVolume, std::shared_ptr<PipeVolume> &pipeVolume)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    CHECK_AND_RETURN_RET_LOG(it != streamVolume_.end(), -1, "sessionId %{public}u is not found.", sessionId);
    streamVolume = it->second;

    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(static_cast<AudioStreamType>(streamType));
    if (volumeType == STREAM_VOICE_ASSISTANT && !it->second->IsSystemApp()) {
        volumeType = STREAM_MUSIC;
    }

    std::string key = std::to_string(pipeId) + "|" + std::to_string(volumeType);
    auto itSV = pipeVolume_.find(key);
    CHECK_AND_RETURN_RET_LOG(itSV != pipeVolume_.end(), -1, "key %{public}s is not found.", key.c_str());
    pipeVolume = itSV->second;
    return 0;
}

float AudioVolume::GetVolume(const std::shared_ptr<StreamVolume>& streamVolume,
    const std::shared_ptr<PipeVolume>& pipeVolume, VolumeValues *volumes)
{
    if (streamVolume == nullptr || pipeVolume == nullptr) return 1.0F;

    Trace trace("[refactor] GetVolume " + std::to_string(streamVolume->GetTotalVolume()));

    volumes->volumeStream = streamVolume->GetTotalVolume();
    volumes->volumeHistory = streamVolume->GetHistoryVolume();
    auto appUid = streamVolume->GetAppUid();
    volumes->volumePipe = pipeVolume->GetTotalVolume();
    auto volumeLevel = pipeVolume->GetNoMuteVolumeLevel();

    float sysVolume = volumes->volumePipe;
    if (streamVolume->IsVirtualKeyboard()) {
        sysVolume = pipeVolume->GetIsMuted() ? 0.0f : 1.0f;
    }

    int32_t doNotDisturbStatusVolume = static_cast<int32_t>(
        GetDoNotDisturbStatusVolumeFromStream(*streamVolume, streamVolume->GetStreamType(), appUid));
    float mdmMuteFactor = AudioMuteFactorManager::GetInstance().GetMdmMuteStatus();
    volumes->volume = sysVolume * volumes->volumeStream * doNotDisturbStatusVolume * mdmMuteFactor;

    {
        std::shared_lock<std::shared_mutex> lock(streamVolume->volumeFactorsMutex_);
        volumes->volumeApp = streamVolume->appVolume_;
    }

    if (!IsSameVolume(streamVolume->GetMonitorVolume(), volumes->volume)) {
        streamVolume->SetMonitorVolumeAtomic(volumes->volume, volumeLevel);
        HILOG_COMM_INFO("volume,sessionId:%{public}u,volume:%{public}f,pipeId:%{public}d,"
            "volumePipe:%{public}f,volumeStream:%{public}f,volumeApp:%{public}f,isVKB:%{public}d,isMuted:%{public}s,"
            "doNotDisturbStatusVolume:%{public}d,mdmStatus:%{public}f", streamVolume->GetSessionId(),
            volumes->volume, pipeVolume->GetPipeId(), volumes->volumePipe, volumes->volumeStream, volumes->volumeApp,
            streamVolume->IsVirtualKeyboard(), pipeVolume->GetIsMuted() ? "T" : "F",
            doNotDisturbStatusVolume, mdmMuteFactor);
    }
    return volumes->volume;
}

void AudioVolume::ConstructEnforcedToneVolumeValues(VolumeValues *volumes)
{
    float fixedVolume = VolumeUtils::GetEnforcedToneVolumeFixed();
    volumes->volumePipe = fixedVolume;
    volumes->volumeStream = 1.0f;
    volumes->volumeApp = 1.0f;
    volumes->volume = fixedVolume;
    volumes->volumeHistory = fixedVolume;
}

/**
 * Since this function is called for each frame of data processing, any operations
 * added within the function should be done with caution to avoid power consumption
 * and performance issues.
 */
float AudioVolume::GetVolume(uint32_t sessionId, int32_t streamTypeIn, uint32_t pipeId,
    VolumeValues *volumes)
{
    // Acquires AudioVolume::volumeMutex_ lock to protect streamVolume_ and pipeVolume_
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    if (streamType == STREAM_SYSTEM_ENFORCED && VolumeUtils::IsEnforcedToneVolumeFixed()) {
        ConstructEnforcedToneVolumeValues(volumes);
        return volumes->volume;
    }
    GetVolumeValues(sessionId, streamType, pipeId, volumes);
    AudioStreamMonitor::GetInstance().UpdateMonitorVolume(sessionId, volumes->volume);
    return volumes->volume;
}

void AudioVolume::GetVolumeValues(uint32_t sessionId, AudioStreamType streamType, uint32_t pipeId,
    VolumeValues *volumes)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t volumeLevel = 0;
    volumes->volumeStream = 1.0f;
    int32_t appUid = -1;
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        volumes->volumeStream = it->second->GetTotalVolume();
        volumes->volumeHistory = it->second->GetHistoryVolume();
        volumes->durationMs = it->second->GetDurationMs();
        appUid = it->second->GetAppUid();
        if (volumeType == STREAM_VOICE_ASSISTANT && !it->second->IsSystemApp()) {
            volumeType = STREAM_MUSIC;
        }
        std::shared_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        volumes->volumeApp = it->second->appVolume_;
        factorsLock.unlock();
    } else {
        AUDIO_DEBUG_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
    //Some devices and volume types always use the maximum volume and do not maintain it in the pipeVolume
    volumes->volumePipe = 1.0f;
    std::string volumeKey = std::to_string(pipeId) + "|" + std::to_string(volumeType);
    auto itSV = pipeVolume_.find(volumeKey);
    if (itSV != pipeVolume_.end()) {
        volumes->volumePipe = itSV->second->GetTotalVolume();
        volumeLevel = itSV->second->GetNoMuteVolumeLevel();
    } else {
        AUDIO_ERR_LOG("no pipe volume, volumeType:%{public}d pipeId:%{public}d", volumeType, pipeId);
    }
    float sysVolume = volumes->volumePipe;
    if (it != streamVolume_.end() && it->second->IsVirtualKeyboard() && itSV != pipeVolume_.end()) {
        sysVolume = itSV->second->GetIsMuted() ? 0.0f : 1.0f;
    }
    int32_t doNotDisturbStatusVolume = static_cast<int32_t>(GetDoNotDisturbStatusVolumeInternal(streamType,
        appUid, sessionId));
    float mdmMuteFactor = AudioMuteFactorManager::GetInstance().GetMdmMuteStatus();
    volumes->volume = sysVolume * volumes->volumeStream * doNotDisturbStatusVolume * mdmMuteFactor;
    if (it != streamVolume_.end() && !IsSameVolume(it->second->GetMonitorVolume(), volumes->volume)) {
        it->second->SetMonitorVolumeAtomic(volumes->volume, volumeLevel);
        HILOG_COMM_INFO("volume,sessionId:%{public}u,volume:%{public}f,volumeType:%{public}d,pipeId:%{public}d,"
            "volumePipe:%{public}f,volumeStream:%{public}f,volumeApp:%{public}f,isVKB:%{public}d,isMuted:%{public}s,"
            "doNotDisturbStatusVolume:%{public}d,mdmStatus:%{public}f", sessionId, volumes->volume, volumeType,
            pipeId, volumes->volumePipe, volumes->volumeStream, volumes->volumeApp,
            it->second->IsVirtualKeyboard(),
            itSV != pipeVolume_.end() ? (itSV->second->GetIsMuted() ? "T" : "F") : "null",
            doNotDisturbStatusVolume, mdmMuteFactor);
    }
    Trace trace("AudioVolume::GetVolume " + std::to_string(volumes->volume));
}

uint32_t AudioVolume::GetDoNotDisturbStatusVolume(int32_t volumeType, int32_t appUid, uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    return GetDoNotDisturbStatusVolumeInternal(volumeType, appUid, sessionId);
}

uint32_t AudioVolume::GetDoNotDisturbStatusVolumeInternal(int32_t volumeType, int32_t appUid, uint32_t sessionId)
{
    if (!isDoNotDisturbStatus_.load(std::memory_order_acquire)) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }
    if (volumeType == STREAM_SYSTEM || volumeType == STREAM_DTMF) {
        return DISTURB_STATE_VOLUME_MUTE;
    }
    auto it = streamVolume_.find(sessionId);
    CHECK_AND_RETURN_RET_LOG(it != streamVolume_.end(), DISTURB_STATE_VOLUME_UNMUTE, "sessionId is null");
    if (it->second->IsSystemApp() || static_cast<uint32_t>(appUid) == VOIP_CALL_VOICE_SERVICE) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }
    AudioStreamType volumeMapType = VolumeUtils::GetVolumeTypeFromStreamType(static_cast<AudioStreamType>(volumeType));

    auto whiteList = std::atomic_load(&doNotDisturbStatusWhiteList_);
    if (whiteList && whiteList->count(static_cast<uint32_t>(appUid))) {
        // this stream of app is in whiteList, unMute
        return DISTURB_STATE_VOLUME_UNMUTE;
    } else {
        // this stream is STREAM_RING, mute
        if (volumeMapType != STREAM_RING && volumeMapType != STREAM_NOTIFICATION) {
            return DISTURB_STATE_VOLUME_UNMUTE;
        } else {
            return DISTURB_STATE_VOLUME_MUTE;
        }
    }
}

uint32_t AudioVolume::GetDoNotDisturbStatusVolumeFromStream(
    StreamVolume& stream, int32_t volumeType, int32_t appUid)
{
    if (!isDoNotDisturbStatus_.load(std::memory_order_acquire)) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }

    if (volumeType == STREAM_SYSTEM || volumeType == STREAM_DTMF) {
        return DISTURB_STATE_VOLUME_MUTE;
    }

    if (stream.IsSystemApp() || static_cast<uint32_t>(appUid) == VOIP_CALL_VOICE_SERVICE) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }

    auto whiteList = std::atomic_load(&doNotDisturbStatusWhiteList_);

    AudioStreamType volumeMapType = VolumeUtils::GetVolumeTypeFromStreamType(
        static_cast<AudioStreamType>(volumeType));

    if (whiteList && whiteList->count(static_cast<uint32_t>(appUid))) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }

    if (volumeMapType != STREAM_RING && volumeMapType != STREAM_NOTIFICATION) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    } else {
        return DISTURB_STATE_VOLUME_MUTE;
    }
}

void AudioVolume::SetDoNotDisturbStatusWhiteListVolume(std::vector<std::map<std::string, std::string>>
    doNotDisturbStatusWhiteList)
{
    auto newWhiteList = std::make_shared<std::unordered_set<uint32_t>>();

    for (const auto& obj : doNotDisturbStatusWhiteList) {
        for (const auto& [key, val] : obj) {
            newWhiteList->insert(static_cast<uint32_t>(atoi(key.c_str())));
        }
    }

    std::atomic_store(&doNotDisturbStatusWhiteList_, newWhiteList);

    AUDIO_INFO_LOG("DoNotDisturb whitelist updated, size: %{public}zu", newWhiteList->size());
}

void AudioVolume::SetDoNotDisturbStatus(bool isDoNotDisturb)
{
    isDoNotDisturbStatus_.store(isDoNotDisturb, std::memory_order_release);
    AUDIO_INFO_LOG("SetDoNotDisturbStatus %{public}d", isDoNotDisturb);
}

float AudioVolume::GetStreamVolume(uint32_t sessionId)
{
    Trace trace("AudioVolume::GetStreamVolume");
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it == streamVolume_.end()) {
        HILOG_COMM_ERROR("[GetStreamVolume]stream volume not exist, sessionId:%{public}u", sessionId);
        return 1.0f;
    }

    std::shared_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
    float volumeStream =
        it->second->isMuted_ || it->second->nonInterruptMute_ || it->second->isDualMuted_ ?
        0.0f : it->second->volume_ * it->second->duckFactor_ * it->second->lowPowerFactor_;
    factorsLock.unlock();

    if (!IsSameVolume(it->second->GetMonitorVolume(), volumeStream)) {
        it->second->SetMonitorVolumeAtomic(volumeStream, 0);
        AUDIO_INFO_LOG("volume, sessionId:%{public}u, stream volume:%{public}f", sessionId, volumeStream);
    }
    return volumeStream;
}

float AudioVolume::GetHistoryVolume(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        return it->second->GetHistoryVolume();
    }
    return 0.0f;
}

uint32_t AudioVolume::GetDurationMs(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        return it->second->GetDurationMs();
    }
    return 0;
}

void AudioVolume::SetHistoryVolume(uint32_t sessionId, float volume, uint32_t durationMs)
{
    AUDIO_DEBUG_LOG("history volume, sessionId:%{public}u, volume:%{public}f", sessionId, volume);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second->SetHistoryVolumeAtomic(volume, durationMs);
    }
}

void AudioVolume::AddStreamVolume(StreamVolumeParams &streamVolumeParams)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u", streamVolumeParams.sessionId);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    float totalSystemAppVolume = 1.0f;
    auto systemAppVolumeIt = systemAppVolume_.find(streamVolumeParams.uid);
    if (systemAppVolumeIt != systemAppVolume_.end()) {
        totalSystemAppVolume = systemAppVolumeIt->second.totalVolume_;
    } else {
        SystemAppVolume systemAppVolume(streamVolumeParams.uid, DEFAULT_APP_VOLUME, DEFAULT_APP_VOLUME_LEVEL, false);
        systemAppVolume.totalVolume_ = systemAppVolume.volume_;
        totalSystemAppVolume = systemAppVolume.totalVolume_;
        systemAppVolume_.emplace(streamVolumeParams.uid, systemAppVolume);
    }
    auto it = streamVolume_.find(streamVolumeParams.sessionId);
    if (it == streamVolume_.end()) {
        streamVolume_.emplace(streamVolumeParams.sessionId,
            std::make_shared<StreamVolume>(streamVolumeParams.sessionId, streamVolumeParams.streamType,
                streamVolumeParams.streamUsage, streamVolumeParams.uid, streamVolumeParams.pid,
                streamVolumeParams.isSystemApp, streamVolumeParams.mode, streamVolumeParams.isVKB));
    } else {
        HILOG_COMM_ERROR("[AddStreamVolume] stream volume already exist, sessionId:%{public}u",
            streamVolumeParams.sessionId);
    }
    it = streamVolume_.find(streamVolumeParams.sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        it->second->systemAppVolume_ = totalSystemAppVolume;
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    } else {
        HILOG_COMM_ERROR("[AddStreamVolume] stream volume not exist, sessionId:%{public}u",
            streamVolumeParams.sessionId);
    }
}

void AudioVolume::RemoveStreamVolume(uint32_t sessionId)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u", sessionId);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        streamVolume_.erase(sessionId);
    } else {
        HILOG_COMM_ERROR("[RemoveStreamVolume] stream volume already delete, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolume(uint32_t sessionId, float volume)
{
    HILOG_COMM_INFO("[SetStreamVolume] stream volume, sessionId:%{public}u, volume:%{public}f", sessionId, volume);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        it->second->volume_ = volume;
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    } else {
        HILOG_COMM_ERROR("[SetStreamVolume] stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolumeDuckFactor(uint32_t sessionId, float duckFactor, uint32_t durationMs)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, duckFactor:%{public}f, durationMs:%{public}d",
        sessionId, duckFactor, durationMs);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        if (!IsVolumeSame(it->second->duckFactor_, duckFactor, AUDIO_VOLUME_EPSILON)) {
            it->second->duckFactor_ = duckFactor;
            it->second->SetHistoryVolumeAtomic(it->second->GetHistoryVolume(), durationMs);
        }
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolumeLowPowerFactor(uint32_t sessionId, float lowPowerFactor)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, lowPowerFactor:%{public}f", sessionId, lowPowerFactor);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        it->second->lowPowerFactor_ = lowPowerFactor;
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string invocationTime,
    uint32_t code)
{
    AdjustStreamVolumeInfo adjustStreamVolumeInfo;
    adjustStreamVolumeInfo.volume = volume;
    adjustStreamVolumeInfo.sessionId = sessionId;
    adjustStreamVolumeInfo.invocationTime = invocationTime;
    switch (code) {
        case static_cast<uint32_t>(AdjustStreamVolume::STREAM_VOLUME_INFO):
            setStreamVolumeInfo_->Add(adjustStreamVolumeInfo);
            break;
        case static_cast<uint32_t>(AdjustStreamVolume::LOW_POWER_VOLUME_INFO):
            setLowPowerVolumeInfo_->Add(adjustStreamVolumeInfo);
            break;
        case static_cast<uint32_t>(AdjustStreamVolume::DUCK_VOLUME_INFO):
            setDuckVolumeInfo_->Add(adjustStreamVolumeInfo);
            break;
        default:
            break;
    }
}

std::vector<AdjustStreamVolumeInfo> AudioVolume::GetStreamVolumeInfo(AdjustStreamVolume volumeType)
{
    switch (volumeType) {
        case AdjustStreamVolume::STREAM_VOLUME_INFO:
            return setStreamVolumeInfo_->GetData();
        case AdjustStreamVolume::LOW_POWER_VOLUME_INFO:
            return setLowPowerVolumeInfo_->GetData();
        case AdjustStreamVolume::DUCK_VOLUME_INFO:
            return setDuckVolumeInfo_->GetData();
        default:
            return {};
    }
}

void AudioVolume::SetNonInterruptMute(uint32_t sessionId, bool muteFlag)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, muteFlag:%{public}d", sessionId, muteFlag);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        it->second->nonInterruptMute_ = muteFlag;
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    }
}

void AudioVolume::SetStreamVolumeMute(uint32_t sessionId, bool isMuted)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, isMuted:%{public}d", sessionId, isMuted);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        it->second->isMuted_ = isMuted;
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    }
}

void AudioVolume::SetDualStreamVolumeMute(uint32_t sessionId, bool isDualMuted)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, isDualMuted:%{public}d", sessionId, isDualMuted);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        std::unique_lock<std::shared_mutex> factorsLock(it->second->volumeFactorsMutex_);
        it->second->isDualMuted_ = isDualMuted;
        it->second->appVolume_ = GetAppVolumeInternal(it->second->GetAppUid(), it->second->GetVolumeMode());
        float newTotal = (it->second->isMuted_ || it->second->isAppRingMuted_ || it->second->nonInterruptMute_ ||
            it->second->isDualMuted_) ? 0.0f : it->second->volume_ * it->second->duckFactor_ *
            it->second->lowPowerFactor_ * it->second->appVolume_ * GetSystemAppVolumeEffective(*it->second);
        factorsLock.unlock();
        it->second->totalVolume_.store(newTotal, std::memory_order_release);
    }
}

float AudioVolume::GetAppVolume(int32_t appUid, AudioVolumeMode mode)
{
    AUDIO_DEBUG_LOG("Get app volume, appUid = %{public}d, mode = %{public}d", appUid, mode);
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    float appVolume = GetAppVolumeInternal(appUid, mode);
    AUDIO_DEBUG_LOG("appVolume = %{public}f", appVolume);
    return appVolume;
}

// MUST be called with volumeMutex_ held
inline float AudioVolume::GetAppVolumeInternal(int32_t appUid, AudioVolumeMode mode)
{
    float appVolume = 1.0f;
    auto iter = appVolume_.find(appUid);
    if (iter != appVolume_.end()) {
        appVolume = VolumeUtils::IsPCVolumeEnable() ? iter->second.totalVolume_ : (iter->second.isMuted_ ?
            iter->second.totalVolume_ :	(mode == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL ? 1.0 :
            iter->second.totalVolume_));
    }
    return appVolume;
}

void AudioVolume::SetAppVolumeMute(int32_t appUid, bool isMuted)
{
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    float totalAppVolume = 1.0f;
    auto it = appVolume_.find(appUid);
    if (it != appVolume_.end()) {
        it->second.isMuted_ = isMuted;
        it->second.totalVolume_ = it->second.isMuted_ ? 0.0f : it->second.volume_;
        totalAppVolume = it->second.totalVolume_;
    } else {
        AppVolume appVolume(appUid, DEFAULT_APP_VOLUME, defaultAppVolume_, isMuted);
        appVolume.totalVolume_ = isMuted ? 0.0f : appVolume.volume_;
        totalAppVolume = appVolume.totalVolume_;
        appVolume_.emplace(appUid, appVolume);
    }

    AUDIO_INFO_LOG("set volume mute, appUId:%{public}d, isMuted:%{public}d, appVolumeSize:%{public}zu",
        appUid, isMuted, appVolume_.size());
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        if (stream->GetAppUid() == appUid) {
            std::unique_lock<std::shared_mutex> factorsLock(stream->volumeFactorsMutex_);
            stream->appVolume_ = VolumeUtils::IsPCVolumeEnable() ? totalAppVolume : (isMuted ? totalAppVolume :
                (stream->GetVolumeMode() == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL ? 1.0 : totalAppVolume));
            float newTotal = (stream->isMuted_ || stream->isAppRingMuted_ || stream->nonInterruptMute_ ||
                stream->isDualMuted_) ? 0.0f : stream->volume_ * stream->duckFactor_ *
                stream->lowPowerFactor_ * stream->appVolume_ * GetSystemAppVolumeEffective(*stream);
            factorsLock.unlock();
            stream->totalVolume_.store(newTotal, std::memory_order_release);
        }
    }
}

bool AudioVolume::SetAppRingMuted(int32_t appUid, bool isMuted)
{
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    float totalAppVolume = 1.0f;
    auto it = appVolume_.find(appUid);
    if (it != appVolume_.end()) {
        it->second.totalVolume_ = it->second.isMuted_ ? 0.0f : it->second.volume_;
        totalAppVolume = it->second.totalVolume_;
    } else {
        AppVolume appVolume(appUid, DEFAULT_APP_VOLUME, defaultAppVolume_, false);
        appVolume.totalVolume_ = appVolume.volume_;
        totalAppVolume = appVolume.totalVolume_;
        appVolume_.emplace(appUid, appVolume);
    }

    AUDIO_INFO_LOG("appUid:%{public}d, isMuted:%{public}d", appUid, isMuted);
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        AUDIO_INFO_LOG("appUid: %{public}d, streamType: %{public}d", stream->GetAppUid(), stream->GetStreamType());
        if (stream->GetAppUid() == appUid && stream->GetStreamType() == static_cast<int32_t>(STREAM_RING)) {
            std::unique_lock<std::shared_mutex> factorsLock(stream->volumeFactorsMutex_);
            stream->isAppRingMuted_ = isMuted;
            stream->appVolume_ = totalAppVolume;
            float newTotal = (stream->isMuted_ || stream->isAppRingMuted_) ? 0.0f : stream->volume_ *
                stream->duckFactor_ * stream->lowPowerFactor_ * stream->appVolume_ *
                GetSystemAppVolumeEffective(*stream);
            factorsLock.unlock();
            stream->totalVolume_.store(newTotal, std::memory_order_release);
            AUDIO_INFO_LOG("stream total volume: %{public}f", stream->GetTotalVolume());
            return true;
        }
    }
    return false;
}


/**
 * Sets the application-requested volume factor (appVolume) for a specific app.
 *
 * The final app volume is determined by:
 *
 *     finalAppVolume = appVolume * systemAppVolume
 *
 * This function only modifies the app-controlled factor.
 */
void AudioVolume::SetAppVolume(AppVolume &appVolume)
{
    int32_t appUid = appVolume.GetAppUid();
    appVolume.totalVolume_ = appVolume.isMuted_ ? 0.0f : appVolume.volume_;
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = appVolume_.find(appUid);
    if (it != appVolume_.end()) {
        it->second.volume_ = appVolume.volume_;
        it->second.volumeLevel_ = appVolume.volumeLevel_;
        it->second.isMuted_ = appVolume.isMuted_;
        it->second.totalVolume_ = appVolume.totalVolume_;
    } else {
        appVolume_.emplace(appUid, appVolume);
    }

    HILOG_COMM_INFO("[SetAppVolume] app volume, appUId:%{public}d, "
        " volume:%{public}f, volumeLevel:%{public}d, isMuted:%{public}d, appVolumeSize:%{public}zu",
        appUid, appVolume.volume_, appVolume.volumeLevel_, appVolume.isMuted_,
        appVolume_.size());
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        if (stream->GetAppUid() == appUid) {
            std::unique_lock<std::shared_mutex> factorsLock(stream->volumeFactorsMutex_);
            stream->appVolume_ = VolumeUtils::IsPCVolumeEnable() ? appVolume.totalVolume_ : (appVolume.isMuted_ ?
                appVolume.totalVolume_ :(stream->GetVolumeMode() == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL ? 1.0 :
                    appVolume.totalVolume_));
            float newTotal = stream->isMuted_ ? 0.0f : stream->volume_ * stream->duckFactor_ *
                stream->lowPowerFactor_ * stream->appVolume_ * GetSystemAppVolumeEffective(*stream);
            factorsLock.unlock();
            stream->totalVolume_.store(newTotal, std::memory_order_release);
        }
    }
}

/**
 * Sets the system-managed volume factor (systemAppVolume) for a specific app.
 *
 * The final app volume is determined by:
 *
 *     finalAppVolume = appVolume * systemAppVolume
 *
 * Unlike SetAppVolume(), this function controls the system policy factor applied to the app.
 */
void AudioVolume::SetSystemAppVolume(SystemAppVolume &systemAppVolume)
{
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    int32_t appUid = systemAppVolume.GetAppUid();
    systemAppVolume.totalVolume_ = systemAppVolume.isMuted_ ? 0.0f : systemAppVolume.volume_;
    auto it = systemAppVolume_.find(appUid);
    if (it != systemAppVolume_.end()) {
        it->second.volume_ = systemAppVolume.volume_;
        it->second.volumeLevel_ = systemAppVolume.volumeLevel_;
        it->second.isMuted_ = systemAppVolume.isMuted_;
        it->second.totalVolume_ = systemAppVolume.totalVolume_;
    } else {
        systemAppVolume_.emplace(appUid, systemAppVolume);
    }

    AUDIO_INFO_LOG("appUid:%{public}d, volume:%{public}f, level:%{public}d, isMuted:%{public}d, size:%{public}zu",
        appUid, systemAppVolume.volume_, systemAppVolume.volumeLevel_, systemAppVolume.isMuted_,
        systemAppVolume_.size());
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        if (stream->GetAppUid() == appUid) {
            std::unique_lock<std::shared_mutex> factorsLock(stream->volumeFactorsMutex_);
            stream->systemAppVolume_ = systemAppVolume.totalVolume_;
            float newTotal = (stream->isMuted_ || stream->isAppRingMuted_ || stream->nonInterruptMute_ ||
                stream->isDualMuted_) ? 0.0f : stream->volume_ * stream->duckFactor_ *
                stream->lowPowerFactor_ * stream->appVolume_ * GetSystemAppVolumeEffective(*stream);
            factorsLock.unlock();
            stream->totalVolume_.store(newTotal, std::memory_order_release);
        }
    }
}

void AudioVolume::SetSystemAppVolumeMuted(int32_t appUid, bool isMuted)
{
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    float totalSystemAppVolume = 1.0f;
    auto it = systemAppVolume_.find(appUid);
    if (it != systemAppVolume_.end()) {
        it->second.isMuted_ = isMuted;
        it->second.totalVolume_ = it->second.isMuted_ ? 0.0f : it->second.volume_;
        totalSystemAppVolume = it->second.totalVolume_;
    } else {
        SystemAppVolume systemAppVolume(appUid, DEFAULT_APP_VOLUME, defaultAppVolume_, isMuted);
        systemAppVolume.totalVolume_ = isMuted ? 0.0f : systemAppVolume.volume_;
        totalSystemAppVolume = systemAppVolume.totalVolume_;
        systemAppVolume_.emplace(appUid, systemAppVolume);
    }

    AUDIO_INFO_LOG("appUid:%{public}d, isMuted:%{public}d, size:%{public}zu",
        appUid, isMuted, systemAppVolume_.size());
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        if (stream->GetAppUid() == appUid) {
            std::unique_lock<std::shared_mutex> factorsLock(stream->volumeFactorsMutex_);
            stream->systemAppVolume_ = totalSystemAppVolume;
            float newTotal = (stream->isMuted_ || stream->isAppRingMuted_ || stream->nonInterruptMute_ ||
                stream->isDualMuted_) ? 0.0f : stream->volume_ * stream->duckFactor_ *
                stream->lowPowerFactor_ * stream->appVolume_ * GetSystemAppVolumeEffective(*stream);
            factorsLock.unlock();
            stream->totalVolume_.store(newTotal, std::memory_order_release);
        }
    }
}

void AudioVolume::SetDefaultAppVolume(int32_t level)
{
    defaultAppVolume_ = level;
}

void AudioVolume::SetPipeVolume(PipeVolume &pipeVolume)
{
    AudioVolumeType volumeType = pipeVolume.GetVolumeType();
#ifdef MULTI_ALARM_LEVEL
    if (volumeType == STREAM_ANNOUNCEMENT || volumeType == STREAM_EMERGENCY) {
        AUDIO_WARNING_LOG("pipe volume, volumeType:%{public}d is not settable", volumeType);
        return;
    }
#endif
    std::string key = std::to_string(pipeVolume.GetPipeId()) + "|" + std::to_string(volumeType);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);

    auto it = pipeVolume_.find(key);
    if (it != pipeVolume_.end()) {
        it->second->SetVolume(pipeVolume.GetVolume());
        it->second->SetIsMuted(pipeVolume.GetIsMuted());
        it->second->SetNoMuteVolumeLevel(pipeVolume.GetNoMuteVolumeLevel());
        float totalVolume = pipeVolume.GetIsMuted() ? 0.0f : pipeVolume.GetVolume();
        it->second->SetTotalVolume(totalVolume);

        AUDIO_INFO_LOG("volumeType:%{public}d, pipeId:%{public}d,volume:%{public}f, noMuteVolumeLevel:%{public}d,"
            "isMuted:%{public}d, pipeVolumeSize:%{public}zu (updated)",  volumeType, pipeVolume.GetPipeId(),
            pipeVolume.GetVolume(), pipeVolume.GetNoMuteVolumeLevel(), pipeVolume.GetIsMuted(), pipeVolume_.size());
    } else {
        pipeVolume_[key] = std::make_shared<PipeVolume>(
            pipeVolume.GetPipeId(), volumeType, pipeVolume.GetVolume(),
            pipeVolume.GetNoMuteVolumeLevel(), pipeVolume.GetIsMuted());

        AUDIO_INFO_LOG("volumeType:%{public}d, pipeId:%{public}d,volume:%{public}f, noMuteVolumeLevel:%{public}d,"
            "isMuted:%{public}d, pipeVolumeSize:%{public}zu (created)",  volumeType, pipeVolume.GetPipeId(),
            pipeVolume.GetVolume(), pipeVolume.GetNoMuteVolumeLevel(), pipeVolume.GetIsMuted(), pipeVolume_.size());
    }
}

void AudioVolume::SetPipeVolumeMute(uint32_t pipeId, AudioVolumeType volumeType, bool isMuted)
{
#ifdef MULTI_ALARM_LEVEL
    if (volumeType == STREAM_ANNOUNCEMENT || volumeType == STREAM_EMERGENCY) {
        AUDIO_WARNING_LOG("volumeType:%{public}d could not be muted", volumeType);
        return;
    }
#endif
    std::string key = std::to_string(pipeId) + "|" + std::to_string(volumeType);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = pipeVolume_.find(key);
    if (it != pipeVolume_.end()) {
        it->second->SetIsMuted(isMuted);
        it->second->SetTotalVolume(isMuted ? 0.0f : it->second->GetVolume());
        AUDIO_INFO_LOG("pipeId:%{public}d, volumeType:%{public}d, isMuted:%{public}d",
            pipeId, volumeType, isMuted);
    } else {
        AUDIO_ERR_LOG("pipe volume type not exist, pipeId:%{public}d, volumeType:%{public}d,",
            pipeId, volumeType);
    }
}

void AudioVolume::RemovePipeVolume(uint32_t pipeId, AudioVolumeType volumeType)
{
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    std::string key = std::to_string(pipeId) + "|" + std::to_string(volumeType);
    auto it = pipeVolume_.find(key);
    if (it != pipeVolume_.end()) {
        pipeVolume_.erase(key);
    } else {
        AUDIO_ERR_LOG("pipe volume already delete, key:%{public}s", key.c_str());
    }
}

int32_t AudioVolume::ConvertStreamTypeStrToInt(const std::string &streamType)
{
    AudioStreamType stream = STREAM_MUSIC;
    if (STREAM_TYPE_STRING_ENUM_MAP.find(streamType) != STREAM_TYPE_STRING_ENUM_MAP.end()) {
        stream = STREAM_TYPE_STRING_ENUM_MAP.at(streamType);
    } else {
        AUDIO_WARNING_LOG("Invalid stream type [%{public}s]. Use default type", streamType.c_str());
    }
    return stream;
}

bool AudioVolume::IsSameVolume(float x, float y)
{
    return (std::abs((x) - (y)) <= std::abs(FLOAT_EPS));
}

void AudioVolume::Dump(std::string &dumpString)
{
    AUDIO_INFO_LOG("AudioVolume dump begin");
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    // dump pipe volume
    std::vector<std::shared_ptr<PipeVolume>> pipeVolumeList;
    for (auto &pipeVolume : pipeVolume_) {
        pipeVolumeList.push_back(pipeVolume.second);
    }
    std::sort(pipeVolumeList.begin(), pipeVolumeList.end(),
        [](std::shared_ptr<PipeVolume> &a, std::shared_ptr<PipeVolume> &b) {
        return a->GetVolumeType() < b->GetVolumeType();
    });
    AppendFormat(dumpString, "\n  - audio pipe volume size: %zu\n", pipeVolumeList.size());
    for (auto &pipeVolume : pipeVolumeList) {
        AppendFormat(dumpString, "  streamtype: %d ", pipeVolume->GetVolumeType());
        AppendFormat(dumpString, "  isMute: %s ", (pipeVolume->GetIsMuted() ? "true" : "false"));
        AppendFormat(dumpString, "  volFloat: %f ", pipeVolume->GetVolume());
        AppendFormat(dumpString, "  volInt: %d ", pipeVolume->GetNoMuteVolumeLevel());
        AppendFormat(dumpString, "  pipeId: %d \n", pipeVolume->GetPipeId());
    }

    // dump stream volume
    std::vector<std::shared_ptr<StreamVolume>> streamVolumeList;
    for (auto &streamVolume : streamVolume_) {
        streamVolumeList.push_back(streamVolume.second);
    }
    std::sort(streamVolumeList.begin(), streamVolumeList.end(),
        [](std::shared_ptr<StreamVolume> &a, std::shared_ptr<StreamVolume> &b) {
        return a->GetSessionId() < b->GetSessionId();
    });
    AppendFormat(dumpString, "\n  - audio stream volume size: %zu\n", streamVolumeList.size());
    for (auto &streamVolume : streamVolumeList) {
        AppendFormat(dumpString, "  sessionId: %u ", streamVolume->GetSessionId());
        AppendFormat(dumpString, "  streamType: %d ", streamVolume->GetStreamType());
        AppendFormat(dumpString, "  streamUsage: %d ", streamVolume->GetStreamUsage());
        AppendFormat(dumpString, "  appUid: %d ", streamVolume->GetAppUid());
        AppendFormat(dumpString, "  appPid: %d ", streamVolume->GetAppPid());
        AppendFormat(dumpString, "  volume: %f ", streamVolume->GetMonitorVolume());
        AppendFormat(dumpString, "  volumeLevel: %d ", streamVolume->GetMonitorVolumeLevel());
        std::shared_lock<std::shared_mutex> factorsLock(streamVolume->volumeFactorsMutex_);
        AppendFormat(dumpString, "  volFactor: %f ", streamVolume->volume_);
        AppendFormat(dumpString, "  duckFactor: %f ", streamVolume->duckFactor_);
        AppendFormat(dumpString, "  powerFactor: %f ", streamVolume->lowPowerFactor_);
        AppendFormat(dumpString, "  appVolume: %f \n", streamVolume->appVolume_);
        factorsLock.unlock();
    }
}

void AudioVolume::Monitor(uint32_t sessionId, bool isOutput)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto streamVolume = streamVolume_.find(sessionId);
    if (streamVolume != streamVolume_.end()) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::VOLUME_CHANGE,
            Media::MediaMonitor::BEHAVIOR_EVENT);
        bean->Add("ISOUTPUT", isOutput ? 1 : 0);
        bean->Add("STREAMID", static_cast<int32_t>(sessionId));
        bean->Add("APP_UID", streamVolume->second->GetAppUid());
        bean->Add("APP_PID", streamVolume->second->GetAppPid());
        bean->Add("STREAMTYPE", streamVolume->second->GetStreamType());
        bean->Add("STREAM_TYPE", streamVolume->second->GetStreamUsage());
        bean->Add("VOLUME", streamVolume->second->GetMonitorVolume());
        bean->Add("SYSVOLUME", streamVolume->second->GetMonitorVolumeLevel());
        std::shared_lock<std::shared_mutex> factorsLock(streamVolume->second->volumeFactorsMutex_);
        bean->Add("VOLUMEFACTOR", streamVolume->second->volume_);
        bean->Add("POWERVOLUMEFACTOR", streamVolume->second->lowPowerFactor_);
        factorsLock.unlock();
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    } else {
        AUDIO_DEBUG_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetFadeoutState(uint32_t streamIndex, uint32_t fadeoutState)
{
    std::unique_lock<std::shared_mutex> lock(fadeoutMutex_);
    fadeoutState_.insert_or_assign(streamIndex, fadeoutState);
}

uint32_t AudioVolume::GetFadeoutState(uint32_t streamIndex)
{
    std::shared_lock<std::shared_mutex> lock(fadeoutMutex_);
    auto it = fadeoutState_.find(streamIndex);
    if (it != fadeoutState_.end()) { return it->second; }
    AUDIO_WARNING_LOG("No such streamIndex in map!");
    return INVALID_STATE;
}

void AudioVolume::RemoveFadeoutState(uint32_t streamIndex)
{
    std::unique_lock<std::shared_mutex> lock(fadeoutMutex_);
    fadeoutState_.erase(streamIndex);
}

void AudioVolume::SetStopFadeoutState(uint32_t streamIndex, uint32_t fadeoutState)
{
    std::unique_lock<std::shared_mutex> lock(fadeoutMutex_);
    stopFadeoutState_.insert_or_assign(streamIndex, fadeoutState);
}

uint32_t AudioVolume::GetStopFadeoutState(uint32_t streamIndex)
{
    std::shared_lock<std::shared_mutex> lock(fadeoutMutex_);
    auto it = stopFadeoutState_.find(streamIndex);
    if (it != stopFadeoutState_.end()) {
        return it->second;
    }
    AUDIO_WARNING_LOG("No such streamIndex in map!");
    return INVALID_STATE;
}

void AudioVolume::RemoveStopFadeoutState(uint32_t streamIndex)
{
    std::unique_lock<std::shared_mutex> lock(fadeoutMutex_);
    stopFadeoutState_.erase(streamIndex);
}

void AudioVolume::SetScoActive(bool isActive)
{
    isScoActive_ = isActive;
}

void AudioVolume::SetCurrentActiveDevice(DeviceType currentActiveDevice)
{
    AUDIO_INFO_LOG("SetCurrentActiveDevice %{public}d", currentActiveDevice);
    currentActiveDevice_ = currentActiveDevice;
}

DeviceType AudioVolume::GetCurrentActiveDevice()
{
    AUDIO_INFO_LOG("GetCurrentActiveDevice %{public}d", currentActiveDevice_);
    return currentActiveDevice_;
}

void AudioVolume::SetOffloadType(uint32_t streamIndex, int32_t offloadType)
{
    std::unique_lock<std::shared_mutex> lock(fadeoutMutex_);
    offloadType_.insert_or_assign(streamIndex, offloadType);
}

int32_t AudioVolume::GetOffloadType(uint32_t streamIndex)
{
    std::shared_lock<std::shared_mutex> lock(fadeoutMutex_);
    auto it = offloadType_.find(streamIndex);
    if (it != offloadType_.end()) { return it->second; }
    AUDIO_WARNING_LOG("No such streamIndex in map!");
    return OFFLOAD_DEFAULT;
}

void AudioVolume::SetOffloadEnable(uint32_t streamIndex, int32_t offloadEnable)
{
    std::unique_lock<std::shared_mutex> lock(fadeoutMutex_);
    offloadEnable_.insert_or_assign(streamIndex, offloadEnable);
}

int32_t AudioVolume::GetOffloadEnable(uint32_t streamIndex)
{
    std::shared_lock<std::shared_mutex> lock(fadeoutMutex_);
    auto it = offloadEnable_.find(streamIndex);
    if (it != offloadEnable_.end()) { return it->second; }
    AUDIO_WARNING_LOG("No such streamIndex in map!");
    return 0;
}

float AudioVolume::GetSystemAppVolume(int32_t uid, AudioStreamType streamType)
{
    if (!ShouldApplySystemAppVolume(streamType)) {
        return 1.0f;
    }
    auto it = systemAppVolume_.find(uid);
    if (it != systemAppVolume_.end()) {
        return it->second.totalVolume_;
    } else {
        return 1.0f;
    }
}

float AudioVolume::GetSystemAppVolumeEffective(StreamVolume &stream)
{
    if (!ShouldApplySystemAppVolume(static_cast<AudioStreamType>(stream.GetStreamType()))) {
        return 1.0f;
    }
    return stream.systemAppVolume_;
}

// PC uses app volume by default; phone uses app volume based on switch and volume type
bool AudioVolume::ShouldApplySystemAppVolume(AudioStreamType streamType)
{
    if (VolumeUtils::IsPCVolumeEnable()) {
        return true;
    }

    if (!appIndividualVolumeEnabled_) {
        return false;
    }
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return volumeType == STREAM_MUSIC;
}

void AudioVolume::HandleAppIndividualVolumeSettingChanged(bool appIndividualVolumeEnabled)
{
    appIndividualVolumeEnabled_ = appIndividualVolumeEnabled;

    // Recalculate the volume of all streams.
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        std::unique_lock<std::shared_mutex> factorsLock(stream->volumeFactorsMutex_);
        float newTotal = (stream->isMuted_ || stream->isAppRingMuted_ || stream->nonInterruptMute_ ||
            stream->isDualMuted_) ? 0.0f : stream->volume_ * stream->duckFactor_ * stream->lowPowerFactor_ *
            stream->appVolume_ * GetSystemAppVolumeEffective(*stream);
        factorsLock.unlock();
        stream->totalVolume_.store(newTotal, std::memory_order_release);
    }
}

std::set<int32_t> AudioVolume::GetStreamVolumeAppUids()
{
    std::set<int32_t> appUidSet;

    for (auto &[sessionId, streamVolume] : streamVolume_) {
        appUidSet.insert(streamVolume->GetAppUid());
    }
    return appUidSet;
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif
using namespace OHOS::AudioStandard;

float GetCurVolume(uint32_t sessionId, const char *streamType, uint32_t pipeId,
    struct VolumeValues *volumes)
{
    CHECK_AND_RETURN_RET_LOG(streamType != nullptr, 1.0f, "streamType is nullptr");
    int32_t stream = AudioVolume::GetInstance()->ConvertStreamTypeStrToInt(streamType);
    return AudioVolume::GetInstance()->GetVolume(sessionId, stream, pipeId, volumes);
}

float GetStreamVolume(uint32_t sessionId)
{
    return AudioVolume::GetInstance()->GetStreamVolume(sessionId);
}

float GetPreVolume(uint32_t sessionId)
{
    return AudioVolume::GetInstance()->GetHistoryVolume(sessionId);
}

void SetPreVolume(uint32_t sessionId, float volume)
{
    AudioVolume::GetInstance()->SetHistoryVolume(sessionId, volume);
}

bool IsSameVolume(float x, float y)
{
    return AudioVolume::GetInstance()->IsSameVolume(x, y);
}

void MonitorVolume(uint32_t sessionId, bool isOutput)
{
    AudioVolume::GetInstance()->Monitor(sessionId, isOutput);
}

void SetFadeoutState(uint32_t streamIndex, uint32_t fadeoutState)
{
    AudioVolume::GetInstance()->SetFadeoutState(streamIndex, fadeoutState);
}

uint32_t GetFadeoutState(uint32_t streamIndex)
{
    return AudioVolume::GetInstance()->GetFadeoutState(streamIndex);
}

uint32_t GetStopFadeoutState(uint32_t streamIndex)
{
    return AudioVolume::GetInstance()->GetStopFadeoutState(streamIndex);
}

void RemoveStopFadeoutState(uint32_t streamIndex)
{
    AudioVolume::GetInstance()->RemoveStopFadeoutState(streamIndex);
}

int32_t GetSimpleBufferAvg(uint8_t *buffer, int32_t length)
{
    if (length <= 0) {
        return -1;
    }
    int32_t sum = std::accumulate(buffer, buffer + length, 0);
    return sum / length;
}

FadeStrategy GetFadeStrategy(uint64_t expectedPlaybackDurationMs)
{
    // 0 is default; duration > 40ms do default fade
    if (expectedPlaybackDurationMs == 0 || expectedPlaybackDurationMs > DURATION_TIME_DEFAULT) {
        return FADE_STRATEGY_DEFAULT;
    }

    // duration <= 10 ms no fade
    if (expectedPlaybackDurationMs <= DURATION_TIME_SHORT && expectedPlaybackDurationMs > 0) {
        return FADE_STRATEGY_NONE;
    }

    // duration > 10ms && duration <= 40ms do 5ms fade
    if (expectedPlaybackDurationMs <= DURATION_TIME_DEFAULT && expectedPlaybackDurationMs > DURATION_TIME_SHORT) {
        return FADE_STRATEGY_SHORTER;
    }

    return FADE_STRATEGY_DEFAULT;
}

void SetOffloadType(uint32_t streamIndex, int32_t offloadType)
{
    AudioVolume::GetInstance()->SetOffloadType(streamIndex, offloadType);
}

int32_t GetOffloadType(uint32_t streamIndex)
{
    return AudioVolume::GetInstance()->GetOffloadType(streamIndex);
}

void SetOffloadEnable(uint32_t streamIndex, int32_t offloadEnable)
{
    AudioVolume::GetInstance()->SetOffloadEnable(streamIndex, offloadEnable);
}

int32_t GetOffloadEnable(uint32_t streamIndex)
{
    return AudioVolume::GetInstance()->GetOffloadEnable(streamIndex);
}
#ifdef __cplusplus
}
#endif