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

#include "audio_volume_c.h"

namespace OHOS {
namespace AudioStandard {
class StreamVolume;
class SystemVolume;
enum FadePauseState {
    NO_FADE,
    DO_FADE,
    DONE_FADE,
    INVALID_STATE
};

class AudioVolume {
public:
    static AudioVolume *GetInstance();
    ~AudioVolume();

    float GetVolume(uint32_t sessionId, int32_t volumeType, const std::string &deviceClass,
        VolumeValues *volumes); // all volume
    float GetStreamVolume(uint32_t sessionId); // only stream volume

    // history volume
    float GetHistoryVolume(uint32_t sessionId);
    void SetHistoryVolume(uint32_t sessionId, float volume);

    // stream volume
    void AddStreamVolume(uint32_t sessionId, int32_t streamType, int32_t streamUsage, int32_t uid, int32_t pid);
    void RemoveStreamVolume(uint32_t sessionId);
    void SetStreamVolume(uint32_t sessionId, float volume);
    void SetStreamVolumeDuckFactor(uint32_t sessionId, float duckFactor);
    void SetStreamVolumeLowPowerFactor(uint32_t sessionId, float lowPowerFactor);
    void SetStreamVolumeMute(uint32_t sessionId, bool isMuted);
    void SetStreamVolumeFade(uint32_t sessionId, float fadeBegin, float fadeEnd);
    std::pair<float, float> GetStreamVolumeFade(uint32_t sessionId);

    // system volume
    void SetSystemVolume(SystemVolume &systemVolume);
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

private:
    AudioVolume();

private:
    std::unordered_map<uint32_t, StreamVolume> streamVolume_ {};
    std::unordered_map<std::string, SystemVolume> systemVolume_ {};
    std::unordered_map<uint32_t, float> historyVolume_ {};
    std::unordered_map<uint32_t, std::pair<float, int32_t>> monitorVolume_ {};
    std::shared_mutex volumeMutex_ {};
    std::shared_mutex systemMutex_ {};

    std::shared_mutex fadoutMutex_ {};
    std::unordered_map<uint32_t, uint32_t> fadeoutState_{};
};

class StreamVolume {
public:
    StreamVolume(uint32_t sessionId, int32_t streamType, int32_t streamUsage, int32_t uid, int32_t pid)
        : sessionId_(sessionId), streamType_(streamType), streamUsage_(streamUsage), appUid_(uid), appPid_(pid) {};
    ~StreamVolume() = default;
    uint32_t GetSessionId() {return sessionId_;};
    int32_t GetStreamType() {return streamType_;};
    int32_t GetStreamUsage() {return streamUsage_;};
    int32_t GetAppUid() {return appUid_;};
    int32_t GetAppPid() {return appPid_;};

public:
    float volume_ = 1.0f;
    float duckFactor_ = 1.0f;
    float lowPowerFactor_ = 1.0f;
    bool isMuted_ = false;
    float fadeBegin_ = 1.0f;
    float fadeEnd_ = 1.0f;

private:
    uint32_t sessionId_ = 0;
    int32_t streamType_ = 0;
    int32_t streamUsage_ = 0;
    int32_t appUid_ = 0;
    int32_t appPid_ = 0;
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
};
} // namespace AudioStandard
} // namespace OHOS
#endif
