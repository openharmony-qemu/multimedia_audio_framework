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

#ifndef AUDIO_VOLUME_H
#define AUDIO_VOLUME_H

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include "audio_stream_info.h"
#include "audio_volume_c.h"
#include "audio_utils.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class StreamVolume;
class SystemVolume;
class AppVolume;
enum FadePauseState {
    NO_FADE,
    DO_FADE,
    DONE_FADE,
    INVALID_STATE
};
const int32_t MAX_STREAM_CACHE_AMOUNT = 10;

class AudioVolume {
public:
    static AudioVolume *GetInstance();
    ~AudioVolume();

    float GetVolume(uint32_t sessionId, int32_t streamType, const std::string &deviceClass,
        VolumeValues *volumes); // all volume
    float GetStreamVolume(uint32_t sessionId); // only stream volume factor
    float GetAppVolume(int32_t appUid, AudioVolumeMode mode);
    // history volume
    float GetHistoryVolume(uint32_t sessionId);
    void SetHistoryVolume(uint32_t sessionId, float volume);

    // stream volume
    void AddStreamVolume(StreamVolumeParams &streamVolumeParams);
    void RemoveStreamVolume(uint32_t sessionId);
    void SetStreamVolume(uint32_t sessionId, float volume);
    void SetStreamVolumeDuckFactor(uint32_t sessionId, float duckFactor);
    void SetStreamVolumeLowPowerFactor(uint32_t sessionId, float lowPowerFactor);
    void SetStreamVolumeMute(uint32_t sessionId, bool isMuted);

    // system volume
    void SetSystemVolume(SystemVolume &systemVolume);
    void SetAppVolume(AppVolume &appVolume);
    void SetAppVolumeMute(int32_t appUid, bool muted);
    bool SetAppRingMuted(int32_t appUid, bool isMuted);
    void SetSystemVolume(int32_t volumeType, const std::string &deviceClass, float volume, int32_t volumeLevel);
    void SetSystemVolumeMute(int32_t volumeType, const std::string &deviceClass, bool isMuted);

    // stream type convert
    int32_t ConvertStreamTypeStrToInt(const std::string &streamType);
    bool IsSameVolume(float x, float y);
    void Dump(std::string &dumpString);
    void Monitor(uint32_t sessionId, bool isOutput);

    void SetFadeoutState(uint32_t streamIndex, uint32_t fadeoutState);
    uint32_t GetFadeoutState(uint32_t streamIndex);
    void RemoveFadeoutState(uint32_t streamIndex);

    void SetStopFadeoutState(uint32_t streamIndex, uint32_t fadeoutState);
    uint32_t GetStopFadeoutState(uint32_t streamIndex);
    void RemoveStopFadeoutState(uint32_t streamIndex);
    
    void SetDefaultAppVolume(int32_t level);
    void SetVgsVolumeSupported(bool isVgsSupported);
    std::vector<AdjustStreamVolumeInfo> GetStreamVolumeInfo(AdjustStreamVolume volumeType);
    void SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string invocationTime, uint32_t code);
    void SetScoActive(bool isActive);
    DeviceType GetCurrentActiveDevice();
    void SetDoNotDisturbStatusVolume(uint32_t sessionId, float volume);
    uint32_t GetDoNotDisturbStatusVolume(int32_t volumeType, int32_t appUid, uint32_t sessionId);
    void SetDoNotDisturbStatusWhiteListVolume(std::vector<std::map<std::string, std::string>>
        doNotDisturbStatusWhiteList);
    void SetDoNotDisturbStatus(bool isDoNotDisturb);
    void SetOffloadType(uint32_t streamIndex, int32_t offloadType);
    int32_t GetOffloadType(uint32_t streamIndex);
    void SetOffloadEnable(uint32_t streamIndex, int32_t offloadEnable);
    int32_t GetOffloadEnable(uint32_t streamIndex);
private:
    AudioVolume();
    float GetAppVolumeInternal(int32_t appUid, AudioVolumeMode mode);
    bool IsVgsVolumeSupported() const;
private:
    std::unordered_map<uint32_t, StreamVolume> streamVolume_ {};
    std::unordered_map<std::string, SystemVolume> systemVolume_ {};
    std::unordered_map<int32_t, AppVolume> appVolume_ {};
    std::shared_mutex volumeMutex_ {};
    bool isVgsVolumeSupported_ = false;
    std::shared_mutex fadeoutMutex_ {};
    std::unordered_map<uint32_t, uint32_t> fadeoutState_{};
    std::unordered_map<uint32_t, uint32_t> stopFadeoutState_{};
    std::unordered_map<uint32_t, int32_t> offloadType_{};
    std::unordered_map<uint32_t, int32_t> offloadEnable_{};
    int32_t defaultAppVolume_ = 0;
    DeviceType currentActiveDevice_ = DEVICE_TYPE_NONE;

    std::shared_ptr<FixedSizeList<AdjustStreamVolumeInfo>> setStreamVolumeInfo_ =
        std::make_shared<FixedSizeList<AdjustStreamVolumeInfo>>(MAX_STREAM_CACHE_AMOUNT);
    std::shared_ptr<FixedSizeList<AdjustStreamVolumeInfo>> setLowPowerVolumeInfo_ =
        std::make_shared<FixedSizeList<AdjustStreamVolumeInfo>>(MAX_STREAM_CACHE_AMOUNT);
    std::shared_ptr<FixedSizeList<AdjustStreamVolumeInfo>> setDuckVolumeInfo_ =
        std::make_shared<FixedSizeList<AdjustStreamVolumeInfo>>(MAX_STREAM_CACHE_AMOUNT);

    bool isScoActive_ = false;
    std::unordered_map<uint32_t, float> doNotDisturbStatusWhiteListVolume_ {};
    bool isDoNotDisturbStatus_ = false;
};

class StreamVolume {
public:
    StreamVolume(uint32_t sessionId, int32_t streamType, int32_t streamUsage, int32_t uid, int32_t pid,
        bool isSystemApp, int32_t mode, bool isVKB) : sessionId_(sessionId), streamType_(streamType),
        streamUsage_(streamUsage), appUid_(uid), appPid_(pid), isSystemApp_(isSystemApp),
        isVirtualKeyboard_(isVKB) {volumeMode_ = static_cast<AudioVolumeMode>(mode);};
    ~StreamVolume() = default;
    uint32_t GetSessionId() {return sessionId_;};
    int32_t GetStreamType() {return streamType_;};
    int32_t GetStreamUsage() {return streamUsage_;};
    int32_t GetAppUid() {return appUid_;};
    int32_t GetAppPid() {return appPid_;};
    bool IsSystemApp() {return isSystemApp_;};
    AudioVolumeMode GetVolumeMode() {return volumeMode_;};
    bool IsVirtualKeyboard() {return isVirtualKeyboard_;};
public:
    float volume_ = 1.0f;
    float duckFactor_ = 1.0f;
    float lowPowerFactor_ = 1.0f;
    bool isMuted_ = false;

    // Indicates whether the stream is muted by SetAppRingMuted API.
    // This flag is only applicable to ring stream.
    bool isAppRingMuted_ = false;

    float appVolume_ = 1.0f;
    float totalVolume_ = 1.0f; // volume_ * duckFactor_ * lowPowerFactor_ * appVolume_

    float historyVolume_ = 0.0f; // used all volume
    float monitorVolume_ = 0.0f; // monitor all volume change
    int32_t monitorVolumeLevel_ = 0; // monitor system volume level change

private:
    uint32_t sessionId_ = 0;
    int32_t streamType_ = 0;
    int32_t streamUsage_ = 0;
    int32_t appUid_ = 0;
    int32_t appPid_ = 0;
    bool isSystemApp_ = false;
    AudioVolumeMode volumeMode_ = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    bool isVirtualKeyboard_ = false;
};

class SystemVolume {
public:
    SystemVolume(int32_t volumeType, std::string deviceClass, float volume, int32_t volumeLevel, bool isMuted)
        : volumeType_(volumeType), deviceClass_(deviceClass), volume_(volume),
        volumeLevel_(volumeLevel), isMuted_(isMuted) {};
    ~SystemVolume() = default;
    int32_t GetVolumeType() {return volumeType_;};
    std::string GetDeviceClass() {return deviceClass_;};

private:
    int32_t volumeType_ = 0;
    std::string deviceClass_ = "";

public:
    float volume_ = 0.0f;
    int32_t volumeLevel_ = 0;
    bool isMuted_ = false;
    float totalVolume_ = 0.0f;
};

class AppVolume {
public:
    AppVolume(int32_t appUid, float volume, int32_t volumeLevel, bool isMuted)
        : appUid_(appUid), volume_(volume),
        volumeLevel_(volumeLevel), isMuted_(isMuted) {};
    ~AppVolume() = default;
    int32_t GetAppUid() {return appUid_;};

private:
    int32_t appUid_ = 0;

public:
    float volume_ = 1.0f;
    int32_t volumeLevel_ = 0;
    bool isMuted_ = false;
    float totalVolume_ = 1.0f;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
