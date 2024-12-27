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

#include "audio_volume.h"
#include "audio_volume_c.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "audio_stream_info.h"
#include "media_monitor_manager.h"
#include "event_bean.h"

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
    {"navigation", STREAM_NAVIGATION}
};

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
    streamVolume_.clear();
    systemVolume_.clear();
    historyVolume_.clear();
    monitorVolume_.clear();
}

float AudioVolume::GetVolume(uint32_t sessionId, int32_t volumeType, const std::string &deviceClass)
{
    Trace trace("AudioVolume::GetVolume sessionId:" + std::to_string(sessionId));
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    float volumeStream = 1.0f;
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        volumeStream =
            it->second.isMuted_ ? 0.0f : it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_;
        AUDIO_DEBUG_LOG("stream volume, sessionId:%{public}u, volume:%{public}f, duck:%{public}f, lowPower:%{public}f,"
            " isMuted:%{public}d, streamVolumeSize:%{public}zu",
            sessionId, it->second.volume_, it->second.duckFactor_, it->second.lowPowerFactor_, it->second.isMuted_,
            streamVolume_.size());
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u, streamVolumeSize:%{public}zu",
            sessionId, streamVolume_.size());
    }

    std::shared_lock<std::shared_mutex> lockSystem(systemMutex_);
    int32_t volumeLevel = 0;
    float volumeSystem = 1.0f;
    std::string key = std::to_string(volumeType) + deviceClass;
    auto itSV = systemVolume_.find(key);
    if (itSV != systemVolume_.end()) {
        volumeLevel = itSV->second.volumeLevel_;
        volumeSystem = itSV->second.isMuted_ ? 0.0f : itSV->second.volume_;
        AUDIO_DEBUG_LOG("system volume, volumeType:%{public}d, deviceClass:%{public}s,"
            " volume:%{public}f, isMuted:%{public}d, systemVolumeSize:%{public}zu",
            volumeType, deviceClass.c_str(), itSV->second.volume_, itSV->second.isMuted_, systemVolume_.size());
    } else {
        AUDIO_ERR_LOG("system volume not exist, volumeType:%{public}d, deviceClass:%{public}s,"
            " systemVolumeSize:%{public}zu", volumeType, deviceClass.c_str(), systemVolume_.size());
    }
    float volumeFloat = volumeStream * volumeSystem;
    if (monitorVolume_.find(sessionId) != monitorVolume_.end()) {
        if (monitorVolume_[sessionId].first != volumeFloat) {
            AUDIO_INFO_LOG("volume, sessionId:%{public}u, volume:%{public}f, volumeType:%{public}d,"
                " deviceClass:%{public}s, stream volume:%{public}f, system volume:%{public}f",
                sessionId, volumeFloat, volumeType, deviceClass.c_str(), volumeStream, volumeSystem);
        }
        monitorVolume_[sessionId] = {volumeFloat, volumeLevel};
    }
    return volumeFloat;
}

float AudioVolume::GetStreamVolume(uint32_t sessionId)
{
    Trace trace("AudioVolume::GetStreamVolume sessionId:" + std::to_string(sessionId));
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    float volumeStream = 1.0f;
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        volumeStream =
            it->second.isMuted_ ? 0.0f : it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_;
        AUDIO_DEBUG_LOG("stream volume, sessionId:%{public}u, volume:%{public}f, duck:%{public}f, lowPower:%{public}f,"
            " isMuted:%{public}d, streamVolumeSize:%{public}zu",
            sessionId, it->second.volume_, it->second.duckFactor_, it->second.lowPowerFactor_, it->second.isMuted_,
            streamVolume_.size());
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u, streamVolumeSize:%{public}zu",
            sessionId, streamVolume_.size());
    }
    if (monitorVolume_.find(sessionId) != monitorVolume_.end()) {
        if (monitorVolume_[sessionId].first != volumeStream) {
            AUDIO_INFO_LOG("volume, sessionId:%{public}u, stream volume:%{public}f", sessionId, volumeStream);
        }
        monitorVolume_[sessionId] = {volumeStream, 15}; // 15 level only stream volume
    }
    return volumeStream;
}

float AudioVolume::GetHistoryVolume(uint32_t sessionId)
{
    Trace trace("AudioVolume::GetHistoryVolume sessionId:" + std::to_string(sessionId));
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = historyVolume_.find(sessionId);
    if (it != historyVolume_.end()) {
        return it->second;
    }
    return 0.0f;
}

void AudioVolume::SetHistoryVolume(uint32_t sessionId, float volume)
{
    AUDIO_INFO_LOG("history volume, sessionId:%{public}u, volume:%{public}f", sessionId, volume);
    Trace trace("AudioVolume::SetHistoryVolume sessionId:" + std::to_string(sessionId));
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = historyVolume_.find(sessionId);
    if (it != historyVolume_.end()) {
        it->second = volume;
    }
}

void AudioVolume::AddStreamVolume(uint32_t sessionId, int32_t streamType, int32_t streamUsage,
    int32_t uid, int32_t pid)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u", sessionId);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it == streamVolume_.end()) {
        streamVolume_.emplace(sessionId, StreamVolume(sessionId, streamType, streamUsage, uid, pid));
        historyVolume_.emplace(sessionId, 0.0f);
        monitorVolume_.emplace(sessionId, std::make_pair(0.0f, 0));
    } else {
        AUDIO_ERR_LOG("stream volume already exist, sessionId:%{public}u", sessionId);
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
        AUDIO_ERR_LOG("stream volume already delete, sessionId:%{public}u", sessionId);
    }
    auto itHistory = historyVolume_.find(sessionId);
    if (itHistory != historyVolume_.end()) {
        historyVolume_.erase(sessionId);
    }
    auto itMonitor = monitorVolume_.find(sessionId);
    if (itMonitor != monitorVolume_.end()) {
        monitorVolume_.erase(sessionId);
    }
}

void AudioVolume::SetStreamVolume(uint32_t sessionId, float volume)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, volume:%{public}f", sessionId, volume);
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.volume_ = volume;
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolumeDuckFactor(uint32_t sessionId, float duckFactor)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, duckFactor:%{public}f", sessionId, duckFactor);
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.duckFactor_ = duckFactor;
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolumeLowPowerFactor(uint32_t sessionId, float lowPowerFactor)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, lowPowerFactor:%{public}f", sessionId, lowPowerFactor);
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.lowPowerFactor_ = lowPowerFactor;
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolumeMute(uint32_t sessionId, bool isMuted)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, isMuted:%{public}d", sessionId, isMuted);
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.isMuted_ = isMuted;
    }
}

void AudioVolume::SetStreamVolumeFade(uint32_t sessionId, float fadeBegin, float fadeEnd)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, fadeBegin:%{public}f, fadeEnd:%{public}f",
        sessionId, fadeBegin, fadeEnd);
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.fadeBegin_ = fadeBegin;
        it->second.fadeEnd_ = fadeEnd;
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

std::pair<float, float> AudioVolume::GetStreamVolumeFade(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        return {it->second.fadeBegin_, it->second.fadeEnd_};
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
    return {1.0f, 1.0f};
}

void AudioVolume::SetSystemVolume(SystemVolume &systemVolume)
{
    auto volumeType = systemVolume.GetVolumeType();
    auto deviceClass = systemVolume.GetDeviceClass();
    std::string key = std::to_string(volumeType) + deviceClass;
    bool haveSystemVolume = true;
    {
        std::shared_lock<std::shared_mutex> lock(systemMutex_);
        auto it = systemVolume_.find(key);
        if (it != systemVolume_.end()) {
            it->second.volume_ = systemVolume.volume_;
            it->second.volumeLevel_ = systemVolume.volumeLevel_;
            it->second.isMuted_ = systemVolume.isMuted_;
        } else {
            haveSystemVolume = false;
        }
    }
    if (!haveSystemVolume) {
        std::unique_lock<std::shared_mutex> lock(systemMutex_);
        systemVolume_.emplace(key, systemVolume);
    }
    AUDIO_INFO_LOG("system volume, volumeType:%{public}d, deviceClass:%{public}s,"
        " volume:%{public}f, volumeLevel:%{public}d, isMuted:%{public}d, systemVolumeSize:%{public}zu",
        volumeType, deviceClass.c_str(), systemVolume.volume_, systemVolume.volumeLevel_, systemVolume.isMuted_,
        systemVolume_.size());
}

void AudioVolume::SetSystemVolume(int32_t volumeType, const std::string &deviceClass, float volume, int32_t volumeLevel)
{
    std::string key = std::to_string(volumeType) + deviceClass;
    bool haveSystemVolume = true;
    {
        std::shared_lock<std::shared_mutex> lock(systemMutex_);
        auto it = systemVolume_.find(key);
        if (it != systemVolume_.end()) {
            it->second.volume_ = volume;
            it->second.volumeLevel_ = volumeLevel;
        } else {
            haveSystemVolume = false;
        }
    }
    if (!haveSystemVolume) {
        std::unique_lock<std::shared_mutex> lock(systemMutex_);
        SystemVolume systemVolume(volumeType, deviceClass, volume, volumeLevel, false);
        systemVolume_.emplace(key, systemVolume);
    }
    AUDIO_INFO_LOG("system volume, volumeType:%{public}d, deviceClass:%{public}s,"
        " volume:%{public}f, volumeLevel:%{public}d, systemVolumeSize:%{public}zu",
        volumeType, deviceClass.c_str(), volume, volumeLevel, systemVolume_.size());
}

void AudioVolume::SetSystemVolumeMute(int32_t volumeType, const std::string &deviceClass, bool isMuted)
{
    AUDIO_INFO_LOG("system volume, volumeType:%{public}d, deviceClass:%{public}s, isMuted:%{public}d",
        volumeType, deviceClass.c_str(), isMuted);
    std::string key = std::to_string(volumeType) + deviceClass;
    bool haveSystemVolume = true;
    {
        std::shared_lock<std::shared_mutex> lock(systemMutex_);
        auto it = systemVolume_.find(key);
        if (it != systemVolume_.end()) {
            it->second.isMuted_ = isMuted;
        } else {
            haveSystemVolume = false;
        }
    }
    if (!haveSystemVolume) {
        std::unique_lock<std::shared_mutex> lock(systemMutex_);
        SystemVolume systemVolume(volumeType, deviceClass, 0.0f, 0, isMuted);
        systemVolume_.emplace(key, systemVolume);
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
    // dump system volume
    std::vector<SystemVolume> systemVolumeList;
    for (auto &systemVolume : systemVolume_) {
        systemVolumeList.push_back(systemVolume.second);
    }
    std::sort(systemVolumeList.begin(), systemVolumeList.end(), [](SystemVolume &a, SystemVolume &b) {
        return a.GetVolumeType() < b.GetVolumeType();
    });
    AppendFormat(dumpString, "\n  - audio system volume size: %zu\n", systemVolumeList.size());
    for (auto &systemVolume : systemVolumeList) {
        AppendFormat(dumpString, "  streamtype: %d ", systemVolume.GetVolumeType());
        AppendFormat(dumpString, "  isMute: %s ", (systemVolume.isMuted_ ? "true" : "false"));
        AppendFormat(dumpString, "  volFloat: %f ", systemVolume.volume_);
        AppendFormat(dumpString, "  volInt: %d ", systemVolume.volumeLevel_);
        AppendFormat(dumpString, "  device class: %s \n", systemVolume.GetDeviceClass().c_str());
    }

    // dump stream volume
    std::vector<StreamVolume> streamVolumeList;
    for (auto &streamVolume : streamVolume_) {
        streamVolumeList.push_back(streamVolume.second);
    }
    std::sort(streamVolumeList.begin(), streamVolumeList.end(), [](StreamVolume &a, StreamVolume &b) {
        return a.GetSessionId() < b.GetSessionId();
    });
    AppendFormat(dumpString, "\n  - audio stream volume size: %zu, his volume size: %zu, mon volume size: %zu\n",
        streamVolumeList.size(), historyVolume_.size(), monitorVolume_.size());
    for (auto &streamVolume : streamVolumeList) {
        auto monVol = monitorVolume_.find(streamVolume.GetSessionId());
        AppendFormat(dumpString, "  sessionId: %u ", streamVolume.GetSessionId());
        AppendFormat(dumpString, "  streamType: %d ", streamVolume.GetStreamType());
        AppendFormat(dumpString, "  streamUsage: %d ", streamVolume.GetStreamUsage());
        AppendFormat(dumpString, "  appUid: %d ", streamVolume.GetAppUid());
        AppendFormat(dumpString, "  appPid: %d ", streamVolume.GetAppPid());
        AppendFormat(dumpString, "  volume: %f ", monVol != monitorVolume_.end() ? monVol->second.first : 0.0f);
        AppendFormat(dumpString, "  volumeLevel: %d ",  monVol != monitorVolume_.end() ? monVol->second.second : 0);
        AppendFormat(dumpString, "  volFactor: %f ", streamVolume.volume_);
        AppendFormat(dumpString, "  duckFactor: %f ", streamVolume.duckFactor_);
        AppendFormat(dumpString, "  powerFactor: %f ", streamVolume.lowPowerFactor_);
        AppendFormat(dumpString, "  fadeBegin: %f ", streamVolume.fadeBegin_);
        AppendFormat(dumpString, "  fadeEnd: %f \n", streamVolume.fadeEnd_);
    }
}

void AudioVolume::Monitor(uint32_t sessionId, bool isOutput)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto streamVolume = streamVolume_.find(sessionId);
    if (streamVolume != streamVolume_.end()) {
        auto monVol = monitorVolume_.find(sessionId);
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::VOLUME_CHANGE,
            Media::MediaMonitor::BEHAVIOR_EVENT);
        bean->Add("ISOUTPUT", isOutput ? 1 : 0);
        bean->Add("STREAMID", static_cast<int32_t>(sessionId));
        bean->Add("APP_UID", streamVolume->second.GetAppUid());
        bean->Add("APP_PID", streamVolume->second.GetAppPid());
        bean->Add("STREAMTYPE", streamVolume->second.GetStreamType());
        bean->Add("STREAM_TYPE", streamVolume->second.GetStreamUsage());
        bean->Add("VOLUME", monVol != monitorVolume_.end() ? monVol->second.first : 0.0f);
        bean->Add("SYSVOLUME", monVol != monitorVolume_.end() ? monVol->second.second : 0);
        bean->Add("VOLUMEFACTOR", streamVolume->second.volume_);
        bean->Add("POWERVOLUMEFACTOR", streamVolume->second.lowPowerFactor_);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetFadeoutState(uint32_t streamIndex, uint32_t fadeoutState)
{
    std::unique_lock<std::shared_mutex> lock(fadoutMutex_);
    fadeoutState_.insert_or_assign(streamIndex, fadeoutState);
}

uint32_t AudioVolume::GetFadeoutState(uint32_t streamIndex)
{
    std::shared_lock<std::shared_mutex> lock(fadoutMutex_);
    auto it = fadeoutState_.find(streamIndex);
    if (it != fadeoutState_.end()) { return it->second; }
    AUDIO_WARNING_LOG("No such streamIndex in map!");
    return INVALID_STATE;
}

void AudioVolume::RemoveFadeoutState(uint32_t streamIndex)
{
    std::unique_lock<std::shared_mutex> lock(fadoutMutex_);
    fadeoutState_.erase(streamIndex);
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif
using namespace OHOS::AudioStandard;

float GetCurVolume(uint32_t sessionId, const char *streamType, const char *deviceClass)
{
    CHECK_AND_RETURN_RET_LOG(streamType != nullptr, 1.0f, "streamType is nullptr");
    CHECK_AND_RETURN_RET_LOG(deviceClass != nullptr, 1.0f, "deviceClass is nullptr");
    std::string tmpStreamType = streamType;
    // Set voice call assistant stream type to full volume
    if (tmpStreamType == "voice_call_assistant") {
        return 1.0f;
    }
    int32_t stream = AudioVolume::GetInstance()->ConvertStreamTypeStrToInt(streamType);
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(static_cast<AudioStreamType>(stream));
    return AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, deviceClass);
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

void GetStreamVolumeFade(uint32_t sessionId, float *fadeBegin, float *fadeEnd)
{
    auto fade = AudioVolume::GetInstance()->GetStreamVolumeFade(sessionId);
    *fadeBegin = fade.first;
    *fadeEnd = fade.second;
}

void SetStreamVolumeFade(uint32_t sessionId, float fadeBegin, float fadeEnd)
{
    AudioVolume::GetInstance()->SetStreamVolumeFade(sessionId, fadeBegin, fadeEnd);
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
#ifdef __cplusplus
}
#endif