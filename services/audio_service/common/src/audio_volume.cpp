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

uint64_t DURATION_TIME_DEFAULT = 40;
uint64_t DURATION_TIME_SHORT = 10;
static const float DEFAULT_APP_VOLUME = 1.0f;
uint32_t VOIP_CALL_VOICE_SERVICE = 5523;
uint32_t DISTURB_STATE_VOLUME_MUTE = 0;
uint32_t DISTURB_STATE_VOLUME_UNMUTE = 1;

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
    appVolume_.clear();
    streamVolume_.clear();
    systemVolume_.clear();
    doNotDisturbStatusWhiteListVolume_.clear();
}

// Note: Time-consuming logic operations cannot be performed on GetVolume.
float AudioVolume::GetVolume(uint32_t sessionId, int32_t streamType, const std::string &deviceClass,
    VolumeValues *volumes)
{
    Trace trace("AudioVolume::GetVolume");
    // read or write volume must be called AudioVolume::volumeMutex_
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(static_cast<AudioStreamType>(streamType));
    int32_t volumeLevel = 0;
    int32_t appUid = -1;
    volumes->volumeStream = 1.0f;
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        volumes->volumeStream = it->second.totalVolume_;
        volumes->volumeHistory = it->second.historyVolume_;
        volumes->volumeApp = it->second.appVolume_;
        appUid = it->second.GetAppUid();
        if (volumeType == STREAM_VOICE_ASSISTANT && !it->second.IsSystemApp()) {
            volumeType = STREAM_MUSIC;
        }
    } else {
        AUDIO_DEBUG_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
    volumes->volumeSystem = 1.0f;
    std::string key = std::to_string(volumeType) + deviceClass;
    auto itSV = systemVolume_.find(key);
    if (itSV != systemVolume_.end()) {
        volumes->volumeSystem = itSV->second.totalVolume_;
        volumeLevel = itSV->second.volumeLevel_;
    } else {
        AUDIO_ERR_LOG("no system volume, volumeType:%{public}d deviceClass%{public}s", volumeType, deviceClass.c_str());
    }
    float sysVolume = volumes->volumeSystem;
    if (it != streamVolume_.end() && it->second.IsVirtualKeyboard() && itSV != systemVolume_.end()) {
        sysVolume = itSV->second.isMuted_ ? 0.0f : 1.0f;
    }
    int32_t doNotDisturbStatusVolume = static_cast<int32_t>(GetDoNotDisturbStatusVolume(streamType, appUid, sessionId));
    float mdmMuteStatus = AudioMuteFactorManager::GetInstance().GetMdmMuteStatus() ? 0.0f : 1.0f;
    volumes->volume = sysVolume * volumes->volumeStream * doNotDisturbStatusVolume * mdmMuteStatus;
    if (it != streamVolume_.end() && !IsSameVolume(it->second.monitorVolume_, volumes->volume)) {
        it->second.monitorVolume_ = volumes->volume;
        it->second.monitorVolumeLevel_ = volumeLevel;
        AUDIO_INFO_LOG("volume,sessionId:%{public}u,volume:%{public}f,volumeType:%{public}d,devClass:%{public}s,"
            "volumeSystem:%{public}f,volumeStream:%{public}f,volumeApp:%{public}f,isVKB:%{public}d,isMuted:%{public}s,"
            "doNotDisturbStatusVolume:%{public}d,mdmStatus:%{public}f", sessionId, volumes->volume, volumeType,
            deviceClass.c_str(), volumes->volumeSystem, volumes->volumeStream, volumes->volumeApp,
            it->second.IsVirtualKeyboard(), itSV != systemVolume_.end() ? (itSV->second.isMuted_ ? "T" : "F") : "null",
            doNotDisturbStatusVolume, mdmMuteStatus);
    }
    AudioStreamMonitor::GetInstance().UpdateMonitorVolume(sessionId, volumes->volume);
    return volumes->volume;
}

uint32_t AudioVolume::GetDoNotDisturbStatusVolume(int32_t volumeType, int32_t appUid, uint32_t sessionId)
{
    if (!isDoNotDisturbStatus_) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }
    if (volumeType == STREAM_SYSTEM || volumeType == STREAM_DTMF) {
        return DISTURB_STATE_VOLUME_MUTE;
    }
    auto it = streamVolume_.find(sessionId);
    CHECK_AND_RETURN_RET_LOG(it != streamVolume_.end(), DISTURB_STATE_VOLUME_UNMUTE, "sessionId is null");
    if (it->second.IsSystemApp() || static_cast<uint32_t>(appUid) == VOIP_CALL_VOICE_SERVICE) {
        return DISTURB_STATE_VOLUME_UNMUTE;
    }
    AudioStreamType volumeMapType = VolumeUtils::GetVolumeTypeFromStreamType(static_cast<AudioStreamType>(volumeType));
    if (doNotDisturbStatusWhiteListVolume_[static_cast<uint32_t>(appUid)] == 1) {
        // this stream of app is in whiteList, unMute
        return DISTURB_STATE_VOLUME_UNMUTE;
    } else {
        // this stream is STREAM_RING, mute
        if (volumeMapType != STREAM_RING) {
            return DISTURB_STATE_VOLUME_UNMUTE;
        } else {
            return DISTURB_STATE_VOLUME_MUTE;
        }
    }
}

void AudioVolume::SetDoNotDisturbStatusWhiteListVolume(std::vector<std::map<std::string, std::string>>
    doNotDisturbStatusWhiteList)
{
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    doNotDisturbStatusWhiteListVolume_.clear();
    for (const auto& obj : doNotDisturbStatusWhiteList) {
        for (const auto& [key, val] : obj) {
            doNotDisturbStatusWhiteListVolume_[atoi(key.c_str())] = 1;
        }
    }
}

void AudioVolume::SetDoNotDisturbStatus(bool isDoNotDisturb)
{
    isDoNotDisturbStatus_ = isDoNotDisturb;
}

float AudioVolume::GetStreamVolume(uint32_t sessionId)
{
    Trace trace("AudioVolume::GetStreamVolume");
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    float volumeStream = 1.0f;
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        // only stream volume factor
        volumeStream =
            it->second.isMuted_ ? 0.0f : it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_;
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
    if (it != streamVolume_.end() && !IsSameVolume(it->second.monitorVolume_, volumeStream)) {
        it->second.monitorVolume_ = volumeStream;
        AUDIO_INFO_LOG("volume, sessionId:%{public}u, stream volume:%{public}f", sessionId, volumeStream);
    }
    return volumeStream;
}

float AudioVolume::GetHistoryVolume(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        return it->second.historyVolume_;
    }
    return 0.0f;
}

void AudioVolume::SetHistoryVolume(uint32_t sessionId, float volume)
{
    AUDIO_DEBUG_LOG("history volume, sessionId:%{public}u, volume:%{public}f", sessionId, volume);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.historyVolume_ = volume;
    }
}

void AudioVolume::AddStreamVolume(StreamVolumeParams &streamVolumeParams)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u", streamVolumeParams.sessionId);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(streamVolumeParams.sessionId);
    if (it == streamVolume_.end()) {
        streamVolume_.emplace(streamVolumeParams.sessionId,
            StreamVolume(streamVolumeParams.sessionId, streamVolumeParams.streamType, streamVolumeParams.streamUsage,
                streamVolumeParams.uid, streamVolumeParams.pid, streamVolumeParams.isSystemApp, streamVolumeParams.mode,
                streamVolumeParams.isVKB));
    } else {
        AUDIO_ERR_LOG("stream volume already exist, sessionId:%{public}u", streamVolumeParams.sessionId);
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
}

void AudioVolume::SetStreamVolume(uint32_t sessionId, float volume)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, volume:%{public}f", sessionId, volume);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.volume_ = volume;
        it->second.appVolume_ = GetAppVolumeInternal(it->second.GetAppUid(), it->second.GetVolumeMode());
        it->second.totalVolume_ = (it->second.isMuted_ || it->second.isAppRingMuted_) ? 0.0f :
            it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_ * it->second.appVolume_;
    } else {
        AUDIO_ERR_LOG("stream volume not exist, sessionId:%{public}u", sessionId);
    }
}

void AudioVolume::SetStreamVolumeDuckFactor(uint32_t sessionId, float duckFactor)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, duckFactor:%{public}f", sessionId, duckFactor);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.duckFactor_ = duckFactor;
        it->second.appVolume_ = GetAppVolumeInternal(it->second.GetAppUid(), it->second.GetVolumeMode());
        it->second.totalVolume_ = (it->second.isMuted_ || it->second.isAppRingMuted_) ? 0.0f :
            it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_ * it->second.appVolume_;
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
        it->second.lowPowerFactor_ = lowPowerFactor;
        it->second.appVolume_ = GetAppVolumeInternal(it->second.GetAppUid(), it->second.GetVolumeMode());
        it->second.totalVolume_ = (it->second.isMuted_ || it->second.isAppRingMuted_) ? 0.0f :
            it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_ * it->second.appVolume_;
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

void AudioVolume::SetStreamVolumeMute(uint32_t sessionId, bool isMuted)
{
    AUDIO_INFO_LOG("stream volume, sessionId:%{public}u, isMuted:%{public}d", sessionId, isMuted);
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = streamVolume_.find(sessionId);
    if (it != streamVolume_.end()) {
        it->second.isMuted_ = isMuted;
        it->second.appVolume_ = GetAppVolumeInternal(it->second.GetAppUid(), it->second.GetVolumeMode());
        it->second.totalVolume_ = (it->second.isMuted_ || it->second.isAppRingMuted_) ? 0.0f :
            it->second.volume_ * it->second.duckFactor_ * it->second.lowPowerFactor_ * it->second.appVolume_;
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

inline float AudioVolume::GetAppVolumeInternal(int32_t appUid, AudioVolumeMode mode)
{
    float appVolume = 1.0f;
    auto iter = appVolume_.find(appUid);
    if (iter != appVolume_.end()) {
        appVolume = iter->second.isMuted_ ? iter->second.totalVolume_ :
            (mode == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL ? 1.0 : iter->second.totalVolume_);
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

    AUDIO_INFO_LOG("set volume mute, appUId:%{public}d, isMuted:%{public}d, systemVolumeSize:%{public}zu",
        appUid, isMuted, appVolume_.size());
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        if (stream.GetAppUid() == appUid) {
            stream.appVolume_ = isMuted ? totalAppVolume :
                (stream.GetVolumeMode() == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL ? 1.0 : totalAppVolume);
            stream.totalVolume_ = stream.isMuted_ ? 0.0f :
                stream.volume_ * stream.duckFactor_ * stream.lowPowerFactor_ * stream.appVolume_;
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
        AUDIO_INFO_LOG("appUid: %{public}d, streamType: %{public}d", stream.GetAppUid(), stream.GetStreamType());
        if (stream.GetAppUid() == appUid && stream.GetStreamType() == static_cast<int32_t>(STREAM_RING)) {
            stream.isAppRingMuted_ = isMuted;
            stream.appVolume_ = totalAppVolume;
            stream.totalVolume_ = (stream.isMuted_ || stream.isAppRingMuted_) ? 0.0f :
                stream.volume_ * stream.duckFactor_ * stream.lowPowerFactor_ * stream.appVolume_;
            AUDIO_INFO_LOG("stream total volume: %{public}f", stream.totalVolume_);
            return true;
        }
    }
    return false;
}

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

    AUDIO_INFO_LOG("system volume, appUId:%{public}d, "
        " volume:%{public}f, volumeLevel:%{public}d, isMuted:%{public}d, systemVolumeSize:%{public}zu",
        appUid, appVolume.volume_, appVolume.volumeLevel_, appVolume.isMuted_,
        appVolume_.size());
    for (auto &streamVolume : streamVolume_) {
        auto &stream = streamVolume.second;
        if (stream.GetAppUid() == appUid) {
            stream.appVolume_ = appVolume.isMuted_ ? appVolume.totalVolume_ :
                (stream.GetVolumeMode() == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL ? 1.0 : appVolume.totalVolume_);
            stream.totalVolume_ = stream.isMuted_ ? 0.0f :
                stream.volume_ * stream.duckFactor_ * stream.lowPowerFactor_ * stream.appVolume_;
        }
    }
}

void AudioVolume::SetDefaultAppVolume(int32_t level)
{
    defaultAppVolume_ = level;
}

void AudioVolume::SetSystemVolume(SystemVolume &systemVolume)
{
    auto volumeType = systemVolume.GetVolumeType();
    auto deviceClass = systemVolume.GetDeviceClass();
    systemVolume.totalVolume_ = systemVolume.isMuted_ ? 0.0f : systemVolume.volume_;
    std::string key = std::to_string(volumeType) + deviceClass;
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = systemVolume_.find(key);
    if (it != systemVolume_.end()) {
        it->second.volume_ = systemVolume.volume_;
        it->second.volumeLevel_ = systemVolume.volumeLevel_;
        it->second.isMuted_ = systemVolume.isMuted_;
        it->second.totalVolume_ = systemVolume.totalVolume_;
    } else {
        systemVolume_.emplace(key, systemVolume);
    }

    AUDIO_INFO_LOG("system volume, volumeType:%{public}d, deviceClass:%{public}s,"
        " volume:%{public}f, volumeLevel:%{public}d, isMuted:%{public}d, systemVolumeSize:%{public}zu",
        volumeType, deviceClass.c_str(), systemVolume.volume_, systemVolume.volumeLevel_, systemVolume.isMuted_,
        systemVolume_.size());
}

void AudioVolume::SetSystemVolume(int32_t volumeType, const std::string &deviceClass,
    float volume, int32_t volumeLevel)
{
    std::string key = std::to_string(volumeType) + deviceClass;
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = systemVolume_.find(key);
    if (it != systemVolume_.end()) {
        it->second.volume_ = volume;
        it->second.volumeLevel_ = volumeLevel;
        it->second.totalVolume_ = it->second.isMuted_ ? 0.0f : it->second.volume_;
    } else {
        SystemVolume systemVolume(volumeType, deviceClass, volume, volumeLevel, false);
        systemVolume.totalVolume_ = systemVolume.isMuted_ ? 0.0f : systemVolume.volume_;
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
    std::unique_lock<std::shared_mutex> lock(volumeMutex_);
    auto it = systemVolume_.find(key);
    if (it != systemVolume_.end()) {
        it->second.isMuted_ = isMuted;
        it->second.totalVolume_ = it->second.isMuted_ ? 0.0f : it->second.volume_;
    } else {
        SystemVolume systemVolume(volumeType, deviceClass, 0.0f, 0, isMuted);
        systemVolume.totalVolume_ = systemVolume.isMuted_ ? 0.0f : systemVolume.volume_;
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
    AppendFormat(dumpString, "\n  - audio stream volume size: %zu\n", streamVolumeList.size());
    for (auto &streamVolume : streamVolumeList) {
        AppendFormat(dumpString, "  sessionId: %u ", streamVolume.GetSessionId());
        AppendFormat(dumpString, "  streamType: %d ", streamVolume.GetStreamType());
        AppendFormat(dumpString, "  streamUsage: %d ", streamVolume.GetStreamUsage());
        AppendFormat(dumpString, "  appUid: %d ", streamVolume.GetAppUid());
        AppendFormat(dumpString, "  appPid: %d ", streamVolume.GetAppPid());
        AppendFormat(dumpString, "  volume: %f ", streamVolume.monitorVolume_);
        AppendFormat(dumpString, "  volumeLevel: %d ", streamVolume.monitorVolumeLevel_);
        AppendFormat(dumpString, "  volFactor: %f ", streamVolume.volume_);
        AppendFormat(dumpString, "  duckFactor: %f ", streamVolume.duckFactor_);
        AppendFormat(dumpString, "  powerFactor: %f ", streamVolume.lowPowerFactor_);
        AppendFormat(dumpString, "  appVolume: %f \n", streamVolume.appVolume_);
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
        bean->Add("APP_UID", streamVolume->second.GetAppUid());
        bean->Add("APP_PID", streamVolume->second.GetAppPid());
        bean->Add("STREAMTYPE", streamVolume->second.GetStreamType());
        bean->Add("STREAM_TYPE", streamVolume->second.GetStreamUsage());
        bean->Add("VOLUME", streamVolume->second.monitorVolume_);
        bean->Add("SYSVOLUME", streamVolume->second.monitorVolumeLevel_);
        bean->Add("VOLUMEFACTOR", streamVolume->second.volume_);
        bean->Add("POWERVOLUMEFACTOR", streamVolume->second.lowPowerFactor_);
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

void AudioVolume::SetVgsVolumeSupported(bool isVgsSupported)
{
    isVgsVolumeSupported_ = isVgsSupported;
}

inline bool AudioVolume::IsVgsVolumeSupported() const
{
    // solve bluetooth sco connneted then connect typec headset, the volume of typec headset can not be adjusted.
    return isVgsVolumeSupported_ && isScoActive_;
}

void AudioVolume::SetScoActive(bool isActive)
{
    isScoActive_ = isActive;
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
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif
using namespace OHOS::AudioStandard;

float GetCurVolume(uint32_t sessionId, const char *streamType, const char *deviceClass,
    struct VolumeValues *volumes)
{
    CHECK_AND_RETURN_RET_LOG(streamType != nullptr, 1.0f, "streamType is nullptr");
    CHECK_AND_RETURN_RET_LOG(deviceClass != nullptr, 1.0f, "deviceClass is nullptr");
    int32_t stream = AudioVolume::GetInstance()->ConvertStreamTypeStrToInt(streamType);
    return AudioVolume::GetInstance()->GetVolume(sessionId, stream, deviceClass, volumes);
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