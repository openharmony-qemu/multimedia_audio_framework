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
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include "audio_stream_info.h"
#include "audio_volume_c.h"
#include "audio_utils.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class StreamVolume;
class PipeVolume;
class AppVolume;
class SystemAppVolume;
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

    float GetVolume(uint32_t sessionId, int32_t streamType, uint32_t pipeId,
        VolumeValues *volumes); // all volume
    float GetStreamVolume(uint32_t sessionId); // only stream volume factor
    float GetAppVolume(int32_t appUid, AudioVolumeMode mode);
    uint32_t GetDurationMs(uint32_t sessionId);
    // history volume
    float GetHistoryVolume(uint32_t sessionId);
    void SetHistoryVolume(uint32_t sessionId, float volume, uint32_t durationMs = 0);

    // stream volume
    void AddStreamVolume(StreamVolumeParams &streamVolumeParams);
    void RemoveStreamVolume(uint32_t sessionId);
    void SetStreamVolume(uint32_t sessionId, float volume);
    void SetStreamVolumeDuckFactor(uint32_t sessionId, float duckFactor, uint32_t durationMs = 0);
    void SetStreamVolumeLowPowerFactor(uint32_t sessionId, float lowPowerFactor);
    void SetStreamVolumeMute(uint32_t sessionId, bool isMuted);
    void SetNonInterruptMute(uint32_t sessionId, bool muteFlag);
    void SetDualStreamVolumeMute(uint32_t sessionId, bool isDualMuted);

    // app volume
    void SetAppVolume(AppVolume &appVolume);
    void SetAppVolumeMute(int32_t appUid, bool muted);
    void SetSystemAppVolume(SystemAppVolume &systemAppVolume);
    void SetSystemAppVolumeMuted(int32_t appUid, bool muted);
    bool SetAppRingMuted(int32_t appUid, bool isMuted);

    //pipe volume
    void SetPipeVolume(PipeVolume &pipeVolume);
    void SetPipeVolumeMute(uint32_t pipeId, AudioVolumeType volumeType, bool muted);
    void RemovePipeVolume(uint32_t pipeId, AudioVolumeType volumeType);

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
    std::vector<AdjustStreamVolumeInfo> GetStreamVolumeInfo(AdjustStreamVolume volumeType);
    void SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string invocationTime, uint32_t code);
    void SetScoActive(bool isActive);
    void SetCurrentActiveDevice(DeviceType currentActiveDevice);
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
    void HandleAppIndividualVolumeSettingChanged(bool appIndividualVolumeEnabled);
    float GetSystemAppVolume(int32_t uid, AudioStreamType streamType);
    std::set<int32_t> GetStreamVolumeAppUids();
    int32_t GetStreamAndPipeVolume(uint32_t sessionId, int32_t streamType, uint32_t pipeId,
        std::shared_ptr<StreamVolume> &streamVolume, std::shared_ptr<PipeVolume> &pipeVolume);
    float GetVolume(const std::shared_ptr<StreamVolume>& streamVolume,
        const std::shared_ptr<PipeVolume>& pipeVolume, VolumeValues *volumes);

private:
    AudioVolume();
    // MUST be called with volumeMutex_ held
    float GetAppVolumeInternal(int32_t appUid, AudioVolumeMode mode);
    uint32_t GetDoNotDisturbStatusVolumeInternal(int32_t volumeType, int32_t appUid, uint32_t sessionId);
    // Lock-free method for GetVolume(shared_ptr version)
    uint32_t GetDoNotDisturbStatusVolumeFromStream(StreamVolume& stream, int32_t volumeType, int32_t appUid);
    void ConstructEnforcedToneVolumeValues(VolumeValues *volumes);
    // MUST be called with volumeMutex_ held
    void GetVolumeValues(uint32_t sessionId, AudioStreamType streamType, uint32_t pipeId, VolumeValues *volumes);
    bool ShouldApplySystemAppVolume(AudioStreamType streamType);
    float GetSystemAppVolumeEffective(StreamVolume &stream);
private:
    std::unordered_map<uint32_t, std::shared_ptr<StreamVolume>> streamVolume_ {};
    std::unordered_map<std::string, std::shared_ptr<PipeVolume>> pipeVolume_ {};
    std::unordered_map<int32_t, AppVolume> appVolume_ {};
    std::unordered_map<int32_t, SystemAppVolume> systemAppVolume_ {};
    std::shared_mutex volumeMutex_ {};
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
    std::shared_ptr<std::unordered_set<uint32_t>> doNotDisturbStatusWhiteList_ =
        std::make_shared<std::unordered_set<uint32_t>>();
    std::atomic<bool> isDoNotDisturbStatus_{false};
    bool appIndividualVolumeEnabled_ = false;
};

class StreamVolume {
public:
    StreamVolume(uint32_t sessionId, int32_t streamType, int32_t streamUsage, int32_t uid, int32_t pid,
        bool isSystemApp, int32_t mode, bool isVKB) : sessionId_(sessionId), streamType_(streamType),
        streamUsage_(streamUsage), appUid_(uid), appPid_(pid), isSystemApp_(isSystemApp),
        isVirtualKeyboard_(isVKB) {volumeMode_ = static_cast<AudioVolumeMode>(mode);};

    StreamVolume(const StreamVolume&) = delete;
    StreamVolume(StreamVolume&&) = delete;
    StreamVolume& operator=(const StreamVolume&) = delete;
    StreamVolume& operator=(StreamVolume&&) = delete;

    ~StreamVolume() = default;
    uint32_t GetSessionId() const {return sessionId_;};
    int32_t GetStreamType() const {return streamType_;};
    int32_t GetStreamUsage() const {return streamUsage_;};
    int32_t GetAppUid() const {return appUid_;};
    int32_t GetAppPid() const {return appPid_;};
    bool IsSystemApp() const {return isSystemApp_;};
    AudioVolumeMode GetVolumeMode() const {return volumeMode_;};
    bool IsVirtualKeyboard() const {return isVirtualKeyboard_;};

    float GetTotalVolume() const { return totalVolume_.load(std::memory_order_acquire); }
    float GetHistoryVolume() const { return historyVolume_.load(std::memory_order_acquire); }
    uint32_t GetDurationMs() const { return durationMs_.load(std::memory_order_acquire); }
    float GetMonitorVolume() const { return monitorVolume_.load(std::memory_order_acquire); }
    int32_t GetMonitorVolumeLevel() const { return monitorVolumeLevel_.load(std::memory_order_acquire); }

    void SetHistoryVolumeAtomic(float vol, uint32_t duration)
    {
        historyVolume_.store(vol, std::memory_order_release);
        durationMs_.store(duration, std::memory_order_release);
    }

    void SetMonitorVolumeAtomic(float vol, int32_t level)
    {
        monitorVolume_.store(vol, std::memory_order_release);
        monitorVolumeLevel_.store(level, std::memory_order_release);
    }

    mutable std::shared_mutex volumeFactorsMutex_;

public:
    // Volume factors, protected by volumeFactorsMutex_
    float volume_ = 1.0f;
    float duckFactor_ = 1.0f;
    float lowPowerFactor_ = 1.0f;
    float appVolume_ = 1.0f;
    float systemAppVolume_ = 1.0f;
    bool isMuted_ = false;
    bool nonInterruptMute_ = false;
    // Indicates whether the stream is muted by SetAppRingMuted API.
    // This flag is only applicable to ring stream.
    bool isAppRingMuted_ = false;
    // dual mute flag, this flag is true only dual and stream is mute
    bool isDualMuted_ = false;

    std::atomic<float> totalVolume_{1.0f};

private:
    std::atomic<float> historyVolume_{0.0f};
    std::atomic<uint32_t> durationMs_{0};
    std::atomic<float> monitorVolume_{0.0f};
    std::atomic<int32_t> monitorVolumeLevel_{0};

    uint32_t sessionId_ = 0;
    int32_t streamType_ = 0;
    int32_t streamUsage_ = 0;
    int32_t appUid_ = 0;
    int32_t appPid_ = 0;
    bool isSystemApp_ = false;
    AudioVolumeMode volumeMode_ = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    bool isVirtualKeyboard_ = false;
};

class PipeVolume {
public:
    PipeVolume() = default;
    PipeVolume(uint32_t pipeId, AudioVolumeType volumeType, float volume, int32_t noMuteVolumeLevel,
        bool isMuted)
        : pipeId_(pipeId), volumeType_(volumeType), noMuteVolumeLevel_{noMuteVolumeLevel},
        isMuted_{isMuted}, volume_{volume}, totalVolume_{isMuted ? 0.0f : volume} {};

    PipeVolume(const PipeVolume&) = delete;
    PipeVolume(PipeVolume&&) = delete;
    PipeVolume& operator=(const PipeVolume&) = delete;
    PipeVolume& operator=(PipeVolume&&) = delete;

    ~PipeVolume() = default;
    AudioVolumeType GetVolumeType() const {return volumeType_;};
    int32_t GetPipeId() const {return pipeId_;};

    float GetTotalVolume() const { return totalVolume_.load(std::memory_order_acquire); }
    float GetVolume() const { return volume_.load(std::memory_order_acquire); }
    bool GetIsMuted() const { return isMuted_.load(std::memory_order_acquire); }
    int32_t GetNoMuteVolumeLevel() const { return noMuteVolumeLevel_.load(std::memory_order_acquire); }

    void SetTotalVolume(float vol) { totalVolume_.store(vol, std::memory_order_release); }
    void SetVolume(float vol)
    {
        volume_.store(vol, std::memory_order_release);
        bool isMuted = isMuted_.load(std::memory_order_acquire);
        float total = isMuted ? 0.0f : vol;
        totalVolume_.store(total, std::memory_order_release);
    }
    void SetIsMuted(bool muted)
    {
        isMuted_.store(muted, std::memory_order_release);
        float volume = volume_.load(std::memory_order_acquire);
        float total = muted ? 0.0f : volume;
        totalVolume_.store(total, std::memory_order_release);
    }
    void SetNoMuteVolumeLevel(int32_t level) { noMuteVolumeLevel_.store(level, std::memory_order_release); }

private:
    uint32_t pipeId_ = 0;
    AudioVolumeType volumeType_ = STREAM_DEFAULT;
    std::atomic<int32_t> noMuteVolumeLevel_{0};
    std::atomic<bool> isMuted_{false};
    std::atomic<float> volume_{0.0f};
    std::atomic<float> totalVolume_{0.0f};
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

class SystemAppVolume {
public:
    SystemAppVolume(int32_t appUid, float volume, int32_t volumeLevel, bool isMuted)
        : appUid_(appUid), volume_(volume),
        volumeLevel_(volumeLevel), isMuted_(isMuted) {};
    ~SystemAppVolume() = default;
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
