/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_PROCESS_IN_CLIENT_H
#define AUDIO_PROCESS_IN_CLIENT_H

#include <map>
#include <memory>

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

class FastAudioStream;

class AudioDataCallback {
public:
    virtual ~AudioDataCallback() = default;

    /**
     * Called when request handle data.
     *
     * @param length Indicates requested buffer length.
     */
    virtual void OnHandleData(size_t length) = 0;
};

class ClientUnderrunCallBack {
    virtual ~ClientUnderrunCallBack() = default;

    /**
     * Callback function when underrun occurs.
     *
     * @param posInFrames Indicates the postion when client handle underrun in frames.
     */
    virtual void OnUnderrun(size_t posInFrames) = 0;
};

class AudioProcessInClient {
public:
    static constexpr int32_t PROCESS_VOLUME_MAX = 1 << 16; // 0 ~ 65536
    static bool CheckIfSupport(const AudioProcessConfig &config);
    static std::shared_ptr<AudioProcessInClient> Create(const AudioProcessConfig &config,
        std::weak_ptr<FastAudioStream> weakStream);

    virtual ~AudioProcessInClient() = default;

    virtual int32_t SaveDataCallback(const std::shared_ptr<AudioDataCallback> &dataCallback) = 0;

    virtual int32_t SaveUnderrunCallback(const std::shared_ptr<ClientUnderrunCallBack> &underrunCallback) = 0;

    virtual int32_t GetBufferDesc(BufferDesc &bufDesc) const = 0;

    virtual int32_t Enqueue(const BufferDesc &bufDesc) = 0;

    virtual int32_t SetVolume(int32_t vol) = 0;

    virtual int32_t SetSourceDuration(int64_t duration) = 0;

    virtual int32_t Start() = 0;

    virtual int32_t Pause(bool isFlush = false) = 0;

    virtual int32_t Resume() = 0;

    virtual int32_t Stop(AudioProcessStage stage = AUDIO_PROC_STAGE_STOP) = 0;

    virtual int32_t Release(bool isSwitchStream = false) = 0;

    // methods for support IAudioStream
    virtual int32_t GetSessionID(uint32_t &sessionID) = 0;

    virtual bool GetAudioTime(uint32_t &framePos, int64_t &sec, int64_t &nanoSec) = 0;

    virtual int32_t GetBufferSize(size_t &bufferSize) = 0;

    virtual int32_t GetFrameCount(uint32_t &frameCount) = 0;

    virtual int32_t GetLatency(uint64_t &latency) = 0;

    virtual int32_t SetVolume(float vol) = 0;

    virtual float GetVolume() = 0;

    virtual int32_t SetDuckVolume(float vol) = 0;

    virtual float GetDuckVolume() = 0;

    virtual int32_t SetMute(bool mute) = 0;

    virtual bool GetMute() = 0;

    virtual uint32_t GetUnderflowCount() = 0;

    virtual uint32_t GetOverflowCount() = 0;

    virtual void SetUnderflowCount(uint32_t underflowCount) = 0;

    virtual void SetOverflowCount(uint32_t overflowCount) = 0;

    virtual int64_t GetFramesWritten() = 0;

    virtual int64_t GetFramesRead() = 0;

    virtual void SetPreferredFrameSize(int32_t frameSize) = 0;

    virtual void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) = 0;

    virtual int32_t SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce = false) = 0;

    virtual int32_t SetSilentModeAndMixWithOthers(bool on) = 0;

    virtual void GetRestoreInfo(RestoreInfo &restoreInfo) = 0;
    
    virtual void SetRestoreInfo(RestoreInfo &restoreInfo) = 0;

    virtual RestoreStatus CheckRestoreStatus() = 0;

    virtual RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) = 0;

    virtual void SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string adjustTime,
        uint32_t code) = 0;

    virtual int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, BoostTriggerMethod method) = 0;

    virtual bool GetStopFlag() const = 0;

    virtual void JoinCallbackLoop() = 0;

    virtual void SetAudioHapticsSyncId(const int32_t &audioHapticsSyncId) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_IN_CLIENT_H
