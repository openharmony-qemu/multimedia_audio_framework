/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef FAST_AUDIO_STREAM_H
#define FAST_AUDIO_STREAM_H

#include <mutex>
#include <condition_variable>
#include "timestamp.h"
#include "event_handler.h"
#include "event_runner.h"

#include "audio_process_in_client.h"
#include "audio_stream_tracker.h"
#include "i_audio_stream.h"
#include "audio_policy_manager.h"
#include "callback_handler.h"

namespace OHOS {
namespace AudioStandard {
class FastPolicyServiceDiedCallbackImpl;
class FastAudioStreamRenderCallback : public AudioDataCallback {
public:
    FastAudioStreamRenderCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback,
        IAudioStream &audioStream)
        : rendererWriteCallback_(callback), audioStreamImpl_(audioStream), hasFirstFrameWrited_(false) {};
    virtual ~FastAudioStreamRenderCallback() = default;

    void OnHandleData(size_t length) override;
    std::shared_ptr<AudioRendererWriteCallback> GetRendererWriteCallback() const;

    void ResetFirstFrameState();
private:
    std::shared_ptr<AudioRendererWriteCallback> rendererWriteCallback_ = nullptr;
    IAudioStream &audioStreamImpl_;
    std::atomic<bool> hasFirstFrameWrited_ = false;
};

class FastAudioStreamCaptureCallback : public AudioDataCallback {
public:
    FastAudioStreamCaptureCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
        : captureCallback_(callback) {};
    virtual ~FastAudioStreamCaptureCallback() = default;

    void OnHandleData(size_t length) override;
    std::shared_ptr<AudioCapturerReadCallback> GetCapturerReadCallback() const;
private:
    std::shared_ptr<AudioCapturerReadCallback> captureCallback_ = nullptr;
};

class FastAudioStream : public IAudioStream,  public IHandler,
    public std::enable_shared_from_this<FastAudioStream>{
public:
    FastAudioStream(AudioStreamType eStreamType, AudioMode eMode, int32_t appUid);
    virtual ~FastAudioStream();

    void SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId) override;

    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override;
    void SetRendererInfo(const AudioRendererInfo &rendererInfo) override;
    void GetRendererInfo(AudioRendererInfo &rendererInfo) override;
    void SetCapturerInfo(const AudioCapturerInfo &capturerInfo) override;
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj,
        const AudioPlaybackCaptureConfig &config = AudioPlaybackCaptureConfig()) override;
    int32_t GetAudioStreamInfo(AudioStreamParams &info) override;
    int32_t GetAudioSessionID(uint32_t &sessionID) override;
    void GetAudioPipeType(AudioPipeType &pipeType) override;
    State GetState() override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    int32_t GetBufferSize(size_t &bufferSize) override;
    int32_t GetFrameCount(uint32_t &frameCount) override;
    int32_t GetLatency(uint64_t &latency) override;
    int32_t SetAudioStreamType(AudioStreamType audioStreamType) override;
    int32_t SetVolume(float volume) override;
    int32_t SetLoudnessGain(float loudnessGain) override;
    float GetLoudnessGain() override;
    int32_t SetMute(bool mute, StateChangeCmdType cmdType) override;
    bool GetMute() override;
    int32_t SetSourceDuration(int64_t duration) override;
    float GetVolume() override;
    int32_t SetDuckVolume(float volume) override;
    float GetDuckVolume() override;
    int32_t SetRenderRate(AudioRendererRate renderRate) override;
    AudioRendererRate GetRenderRate() override;
    int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback) override;

    void InitCallbackHandler();
    void SafeSendCallbackEvent(uint32_t eventCode, int64_t data);
    void OnHandle(uint32_t code, int64_t data) override;
    void HandleStateChangeEvent(int64_t data);
    int32_t ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType);

    // callback mode api
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override;
    void OnFirstFrameWriting() override;
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    AudioRenderMode GetRenderMode() override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetCaptureMode(AudioCaptureMode captureMode) override;
    AudioCaptureMode GetCaptureMode() override;
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t GetBufQueueState(BufferQueueState &bufState) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    int32_t Clear() override;
    void SetPreferredFrameSize(int32_t frameSize) override;
    void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) override;
    int32_t SetLowPowerVolume(float volume) override;
    float GetLowPowerVolume() override;
    int32_t SetOffloadMode(int32_t state, bool isAppBack) override;
    int32_t UnsetOffloadMode() override;
    float GetSingleStreamVolume() override;
    AudioEffectMode GetAudioEffectMode() override;
    int32_t SetAudioEffectMode(AudioEffectMode effectMode) override;
    int64_t GetFramesWritten() override;
    int64_t GetFramesRead() override;

    void SetInnerCapturerState(bool isInnerCapturer) override;
    void SetWakeupCapturerState(bool isWakeupCapturer) override;
    void SetCapturerSource(int capturerSource) override;
    void SetPrivacyType(AudioPrivacyType privacyType) override;

    // Common APIs
    bool StartAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT,
        AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN) override;
    bool PauseAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool StopAudioStream() override;
    bool ReleaseAudioStream(bool releaseRunner = true, bool isSwitchStream = false) override;
    bool FlushAudioStream() override;

    // Playback related APIs
    bool DrainAudioStream(bool stopFlag = false) override;
    int32_t Write(uint8_t *buffer, size_t buffer_size) override;
    int32_t Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize) override;
    int32_t SetSpeed(float speed) override;
    int32_t SetPitch(float pitch) override;
    float GetSpeed() override;

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override;

    uint32_t GetUnderflowCount() override;
    uint32_t GetOverflowCount() override;
    void SetUnderflowCount(uint32_t underflowCount) override;
    void SetOverflowCount(uint32_t overflowCount) override;

    void SetRendererPositionCallback(int64_t markPosition, const std::shared_ptr<RendererPositionCallback> &callback)
        override;
    void UnsetRendererPositionCallback() override;
    void SetRendererPeriodPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;
    void SetCapturerPositionCallback(int64_t markPosition, const std::shared_ptr<CapturerPositionCallback> &callback)
        override;
    void UnsetCapturerPositionCallback() override;
    void SetCapturerPeriodPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override;
    void UnsetCapturerPeriodPositionCallback() override;
    int32_t SetRendererSamplingRate(uint32_t sampleRate) override;
    uint32_t GetRendererSamplingRate() override;
    int32_t SetBufferSizeInMsec(int32_t bufferSizeInMsec) override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override;

    void SetStreamTrackerState(bool trackerRegisteredState) override;
    void GetSwitchInfo(IAudioStream::SwitchInfo& info) override;

    IAudioStream::StreamClass GetStreamClass() override;

    bool RestoreAudioStream(bool needStoreState = true) override;
    void JoinCallbackLoop() override;

    bool GetOffloadEnable() override;
    bool GetSpatializationEnabled() override;
    bool GetHighResolutionEnabled() override;

    void SetSilentModeAndMixWithOthers(bool on) override;

    bool GetSilentModeAndMixWithOthers() override;

    int32_t SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce = false) override;

    FastStatus GetFastStatus() override;

    DeviceType GetDefaultOutputDevice() override;

    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) override;

    void SetSwitchingStatus(bool isSwitching) override;

    void GetRestoreInfo(RestoreInfo &restoreInfo) override;
    void SetRestoreInfo(RestoreInfo &restoreInfo) override;
    RestoreStatus CheckRestoreStatus() override;
    RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) override;
    void SetSwitchInfoTimestamp(std::vector<std::pair<uint64_t, uint64_t>> lastFramePosAndTimePair,
        std::vector<std::pair<uint64_t, uint64_t>> lastFramePosAndTimePairWithSpeed) override;
    void FetchDeviceForSplitStream() override;
    void SetCallStartByUserTid(pid_t tid) override;
    void SetCallbackLoopTid(int32_t tid) override;
    int32_t GetCallbackLoopTid() override;
    void ResetCallbackLoopTid();
    bool GetStopFlag() const override;
    void ResetFirstFrameState() override;
    void SetAudioHapticsSyncId(const int32_t &audioHapticsSyncId) override;
private:
    void UpdateRegisterTrackerInfo(AudioRegisterTrackerInfo &registerTrackerInfo);
    int32_t InitializeAudioProcessConfig(AudioProcessConfig &config, const AudioStreamParams &info);
    int32_t SetCallbacksWhenRestore();
    void RegisterThreadPriorityOnStart(StateChangeCmdType cmdType);

    AudioStreamType eStreamType_;
    AudioMode eMode_;
    std::shared_ptr<AudioProcessInClient> processClient_ = nullptr;
    std::shared_ptr<FastAudioStreamRenderCallback> spkProcClientCb_ = nullptr;
    std::shared_ptr<FastAudioStreamCaptureCallback> micProcClientCb_ = nullptr;
    std::shared_ptr<AudioRendererFirstFrameWritingCallback> firstFrameWritingCb_ = nullptr;
    std::unique_ptr<AudioStreamTracker> audioStreamTracker_;
    AudioRendererInfo rendererInfo_;
    AudioCapturerInfo capturerInfo_;
    AudioStreamParams streamInfo_;
    AudioProcessConfig processconfig_;
    State state_;
    uint32_t sessionId_ = 0;
    uint32_t underflowCount_ = 0;
    uint32_t overflowCount_ = 0;
    AudioRenderMode renderMode_;
    AudioCaptureMode captureMode_;
    AudioRendererRate renderRate_ = RENDER_RATE_NORMAL;
    int32_t clientPid_ = 0;
    int32_t clientUid_ = 0;
    uint32_t appTokenId_ = 0;
    uint64_t fullTokenId_ = 0;
    bool streamTrackerRegistered_ = false;
    std::shared_ptr<AudioClientTracker> proxyObj_ = nullptr;
    bool silentModeAndMixWithOthers_ = false;
    DeviceType defaultOutputDevice_ = DEVICE_TYPE_NONE;
    StateChangeCmdType muteCmd_ = CMD_FROM_CLIENT;

    std::mutex streamCbMutex_;
    std::weak_ptr<AudioStreamCallback> streamCallback_;

    bool runnerReleased_ = false;
    std::mutex runnerMutex_;
    std::shared_ptr<CallbackHandler> callbackHandler_ = nullptr;

    std::mutex setPreferredFrameSizeMutex_;
    std::optional<int32_t> userSettedPreferredFrameSize_ = std::nullopt;

    std::mutex switchingMutex_;
    StreamSwitchingInfo switchingInfo_ {false, INVALID};

    std::mutex lastCallStartByUserTidMutex_;
    std::optional<pid_t> lastCallStartByUserTid_ = std::nullopt;

    int32_t callbackLoopTid_ = -1;
    std::mutex callbackLoopTidMutex_;
    std::condition_variable callbackLoopTidCv_;

    enum {
        STATE_CHANGE_EVENT = 0
    };

    enum : int64_t {
        HANDLER_PARAM_INVALID = -1,
        HANDLER_PARAM_NEW = 0,
        HANDLER_PARAM_PREPARED,
        HANDLER_PARAM_RUNNING,
        HANDLER_PARAM_STOPPED,
        HANDLER_PARAM_RELEASED,
        HANDLER_PARAM_PAUSED,
        HANDLER_PARAM_STOPPING,
        HANDLER_PARAM_RUNNING_FROM_SYSTEM,
        HANDLER_PARAM_PAUSED_FROM_SYSTEM,
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // FAST_AUDIO_STREAM_H
