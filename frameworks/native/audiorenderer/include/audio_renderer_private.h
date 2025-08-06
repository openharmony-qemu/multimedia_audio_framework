/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_PRIVATE_H
#define AUDIO_RENDERER_PRIVATE_H

#include <shared_mutex>
#include <optional>

#include "securec.h"

#include "audio_interrupt_callback.h"
#include "audio_route_callback.h"
#include "audio_renderer.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_utils.h"
#include "i_audio_stream.h"
#include "audio_stream_descriptor.h"
#include "audio_task_loop.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t INVALID_SESSION_ID = static_cast<uint32_t>(-1);
class RendererPolicyServiceDiedCallback;
class OutputDeviceChangeWithInfoCallbackImpl;
class FormatUnsupportedErrorCallbackImpl;
class AudioRouteCallbackImpl;

class AudioRendererPrivate : public AudioRenderer, public std::enable_shared_from_this<AudioRendererPrivate> {
    friend class AudioRouteCallbackImpl;
public:
    int32_t GetFrameCount(uint32_t &frameCount) const override;
    int32_t GetLatency(uint64_t &latency) const override;
    void SetAudioPrivacyType(AudioPrivacyType privacyType) override;
    AudioPrivacyType GetAudioPrivacyType() override;
    int32_t SetParams(const AudioRendererParams params) override;
    int32_t GetParams(AudioRendererParams &params) const override;
    int32_t GetRendererInfo(AudioRendererInfo &rendererInfo) const override;
    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override;
    bool Start(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    int32_t Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize) override;
    RendererState GetStatus() const override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    bool Drain() const override;
    bool PauseTransitent(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Pause(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Mute(StateChangeCmdType cmdType = CMD_FROM_CLIENT) const override;
    bool Unmute(StateChangeCmdType cmdType = CMD_FROM_CLIENT) const override;
    bool Stop() override;
    bool Flush() const override;
    bool Release() override;
    int32_t GetBufferSize(size_t &bufferSize) const override;
    int32_t GetAudioStreamId(uint32_t &sessionID) const override;
    int32_t SetAudioRendererDesc(AudioRendererDesc audioRendererDesc) override;
    int32_t SetStreamType(AudioStreamType audioStreamType) override;
    int32_t SetVolume(float volume) const override;
    int32_t SetVolumeMode(int32_t mode) override;
    float GetVolume() const override;
    int32_t SetLoudnessGain(float loudnessGain) const override;
    float GetLoudnessGain() const override;
    int32_t SetRenderRate(AudioRendererRate renderRate) const override;
    AudioRendererRate GetRenderRate() const override;
    int32_t SetRendererSamplingRate(uint32_t sampleRate) const override;
    uint32_t GetRendererSamplingRate() const override;
    int32_t SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback) override;
    int32_t SetRendererPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPositionCallback> &callback) override;
    void UnsetRendererPositionCallback() override;
    int32_t SetRendererPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;
    int32_t SetBufferDuration(uint64_t bufferDuration) const override;
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    AudioRenderMode GetRenderMode() const override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override;
    void SetPreferredFrameSize(int32_t frameSize) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    int32_t Clear() const override;
    int32_t GetBufQueueState(BufferQueueState &bufState) const override;
    void SetInterruptMode(InterruptMode mode) override;
    int32_t SetParallelPlayFlag(bool parallelPlayFlag) override;
    int32_t SetLowPowerVolume(float volume) const override;
    float GetLowPowerVolume() const override;
    int32_t SetOffloadAllowed(bool isAllowed) override;
    int32_t SetOffloadMode(int32_t state, bool isAppBack) const override;
    int32_t UnsetOffloadMode() const override;
    float GetSingleStreamVolume() const override;
    float GetMinStreamVolume() const override;
    float GetMaxStreamVolume() const override;
    int32_t GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const override;
    uint32_t GetUnderflowCount() const override;

    int32_t RegisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override;
    int32_t UnregisterOutputDeviceChangeWithInfoCallback() override;
    int32_t UnregisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override;
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback) override;
    int32_t UnregisterAudioPolicyServerDiedCb(const int32_t clientPid) override;
    AudioEffectMode GetAudioEffectMode() const override;
    int64_t GetFramesWritten() const override;
    int32_t SetAudioEffectMode(AudioEffectMode effectMode) const override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;
    void SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback) override;
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override;

    int32_t RegisterRendererPolicyServiceDiedCallback();
    int32_t RemoveRendererPolicyServiceDiedCallback();

    void SetFastStatusChangeCallback(const std::shared_ptr<AudioRendererFastStatusChangeCallback> &callback) override;

    void GetAudioInterrupt(AudioInterrupt &audioInterrupt);
    void SetAudioInterrupt(const AudioInterrupt &audioInterrupt);

    bool IsOffloadEnable() override;

    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
    bool IsFastRenderer() override;

    void SetSilentModeAndMixWithOthers(bool on) override;
    bool GetSilentModeAndMixWithOthers() override;

    void EnableVoiceModemCommunicationStartStream(bool enable) override;

    bool IsNoStreamRenderer() const override;
    void RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter);

    int64_t GetSourceDuration() const override;
    void SetSourceDuration(int64_t duration) override;

    int32_t SetDefaultOutputDevice(DeviceType deviceType) override;
    FastStatus GetFastStatus() override;
    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override;

    int32_t StartDataCallback() override;
    int32_t StopDataCallback() override;
    void SetAudioHapticsSyncId(int32_t audioHapticsSyncId) override;
    void ResetFirstFrameState() override;

    void SetInterruptEventCallbackType(InterruptEventCallbackType callbackType) override;

    bool IsVirtualKeyboard(const int32_t flags);
    void HandleSetRendererInfoByOptions(const AudioRendererOptions &rendererOptions, const AppInfo &appInfo);
    static inline AudioStreamParams ConvertToAudioStreamParams(const AudioRendererParams params)
    {
        AudioStreamParams audioStreamParams;

        audioStreamParams.format = params.sampleFormat;
        audioStreamParams.samplingRate = params.sampleRate;
        audioStreamParams.channels = params.channelCount;
        audioStreamParams.encoding = params.encodingType;
        audioStreamParams.channelLayout = params.channelLayout;

        return audioStreamParams;
    }

    AudioPrivacyType privacyType_ = PRIVACY_TYPE_PUBLIC;
    AudioRendererInfo rendererInfo_ = {CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC, 0};
    AudioSessionStrategy strategy_ = { AudioConcurrencyMode::INVALID };
    AudioSessionStrategy originalStrategy_ = { AudioConcurrencyMode::INVALID };
    std::shared_ptr<IAudioStream> audioStream_;
    bool abortRestore_ = false;
    mutable bool isStillZeroStreamVolume_ = false;

    explicit AudioRendererPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo, bool createStream = true);

    ~AudioRendererPrivate();

    AudioRendererPrivate(const AudioRendererPrivate &) = delete;
    AudioRendererPrivate &operator=(const AudioRendererPrivate &) = delete;
    AudioRendererPrivate(AudioRendererPrivate &&) = delete;
    AudioRendererPrivate &operator=(AudioRendererPrivate &&) = delete;
protected:

private:
    int32_t CheckAndRestoreAudioRenderer(std::string callingFunc);
    int32_t AsyncCheckAudioRenderer(std::string callingFunc);
    int32_t CheckAudioRenderer(std::string callingFunc);
    int32_t CheckAndStopAudioRenderer(std::string callingFunc);
    int32_t PrepareAudioStream(AudioStreamParams &audioStreamParams,
        const AudioStreamType &audioStreamType, IAudioStream::StreamClass &streamClass, uint32_t &flag);
    std::shared_ptr<AudioStreamDescriptor> ConvertToStreamDescriptor(const AudioStreamParams &audioStreamParams);
    std::shared_ptr<AudioStreamDescriptor> GetStreamDescBySwitchInfo(
        const IAudioStream::SwitchInfo &switchInfo, const RestoreInfo &restoreInfo);
    void SetClientInfo(uint32_t flag, IAudioStream::StreamClass &streamClass);
    int32_t InitAudioInterruptCallback(bool isRestoreAudio = false);
    int32_t InitOutputDeviceChangeCallback();
    void InitAudioRouteCallback();
    int32_t InitAudioStream(AudioStreamParams audioStreamParams);
    bool SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream);
    void UpdateRendererAudioStream(const std::shared_ptr<IAudioStream> &newAudioStream);
    void InitSwitchInfo(IAudioStream::StreamClass targetClass, IAudioStream::SwitchInfo &info);
    bool SwitchToTargetStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo);
    bool FinishOldStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo, RendererState previousState,
        IAudioStream::SwitchInfo &info);
    bool GenerateNewStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo, RendererState previousState,
        IAudioStream::SwitchInfo &info);
    bool ContinueAfterConcede(IAudioStream::StreamClass &targetClass, RestoreInfo restoreInfo);
    bool ContinueAfterSplit(RestoreInfo restoreInfo);
    bool InitTargetStream(IAudioStream::SwitchInfo &info, std::shared_ptr<IAudioStream> &audioStream);
    void HandleAudioInterruptWhenServerDied();
    void WriteSwitchStreamLogMsg();
    void InitLatencyMeasurement(const AudioStreamParams &audioStreamParams);
    void MockPcmData(uint8_t *buffer, size_t bufferSize) const;
    void WriteUnderrunEvent() const;
    IAudioStream::StreamClass GetPreferredStreamClass(AudioStreamParams audioStreamParams);
    bool IsDirectVoipParams(const AudioStreamParams &audioStreamParams);
    void UpdateAudioInterruptStrategy(float volume, bool setVolume) const;
    bool IsAllowedStartBackgroud();
    bool GetStartStreamResult(StateChangeCmdType cmdType);
    void UpdateFramesWritten();
    RendererState GetStatusInner();
    void SetAudioPrivacyTypeInner(AudioPrivacyType privacyType);
    int32_t GetAudioStreamIdInner(uint32_t &sessionID) const;
    float GetVolumeInner() const;
    uint32_t GetUnderflowCountInner() const;
    int32_t UnsetOffloadModeInner() const;
    std::shared_ptr<IAudioStream> GetInnerStream() const;
    int32_t InitFormatUnsupportedErrorCallback();
    int32_t SetPitch(float pitch);
    FastStatus GetFastStatusInner();
    void FastStatusChangeCallback(FastStatus status);
    int32_t HandleCreateFastStreamError(AudioStreamParams &audioStreamParams, AudioStreamType audioStreamType);
    int32_t StartSwitchProcess(RestoreInfo &restoreInfo, IAudioStream::StreamClass &targetClass,
        std::string callingFunc);
    bool GetFinalOffloadAllowed(bool originalAllowed);
    void SetReleaseFlagWithLock(bool releaseFlag);
    void SetReleaseFlagNoLock(bool releaseFlag);

    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;
    std::shared_ptr<AudioStreamCallback> audioStreamCallback_ = nullptr;
    std::shared_ptr<AudioRouteCallback> audioRouteCallback_ = nullptr;
    AppInfo appInfo_ = {};
    AudioInterrupt audioInterrupt_ = {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, 0};
    uint32_t sessionID_ = INVALID_SESSION_ID;
    std::shared_ptr<AudioRendererProxyObj> rendererProxyObj_;
    FILE *dumpFile_ = nullptr;
    std::shared_ptr<AudioRendererErrorCallback> audioRendererErrorCallback_ = nullptr;
    std::mutex audioRendererErrCallbackMutex_;
    std::shared_ptr<OutputDeviceChangeWithInfoCallbackImpl> outputDeviceChangeCallback_ = nullptr;
    std::shared_ptr<AudioRendererFastStatusChangeCallback> fastStatusChangeCallback_ = nullptr;
    std::mutex fastStatusChangeCallbackMutex_;
    mutable std::shared_ptr<RendererPolicyServiceDiedCallback> audioPolicyServiceDiedCallback_ = nullptr;
    std::shared_ptr<FormatUnsupportedErrorCallbackImpl> formatUnsupportedErrorCallback_ = nullptr;
    std::atomic<bool> isFastRenderer_ = false;
    bool latencyMeasEnabled_ = false;
    std::shared_ptr<AudioLatencyMeasurement> latencyMeasurement_ = nullptr;
    bool isSwitching_ = false;
    mutable std::shared_mutex rendererMutex_;
    mutable std::shared_mutex streamMutex_;
    mutable AudioRenderMode audioRenderMode_ = RENDER_MODE_NORMAL;
    int32_t callbackLoopTid_ = -1;
    bool isFastVoipSupported_ = false;
    bool isDirectVoipSupported_ = false;
    bool isEnableVoiceModemCommunicationStartStream_ = false;
    RendererState state_ = RENDERER_INVALID;

    std::optional<float> speed_ = std::nullopt;
    std::optional<float> pitch_ = std::nullopt;

    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> policyServiceDiedCallback_ = nullptr;
    std::mutex policyServiceDiedCallbackMutex_;

    std::vector<uint32_t> usedSessionId_ = {};
    std::mutex silentModeAndMixWithOthersMutex_;
    std::mutex setStreamCallbackMutex_;
    std::mutex setParamsMutex_;
    std::mutex rendererPolicyServiceDiedCbMutex_;
    int64_t framesAlreadyWritten_ = 0;
    int64_t sourceDuration_ = -1;
    std::atomic<uint32_t> switchStreamInNewThreadTaskCount_ = 0;

    AudioLoopThread taskLoop_ = AudioLoopThread("OS_Recreate");
    int32_t audioHapticsSyncId_ = 0;
    bool releaseFlag_ = false;
};

class AudioRendererInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioRendererInterruptCallbackImpl(const std::shared_ptr<IAudioStream> &audioStream,
        const AudioInterrupt &audioInterrupt);
    virtual ~AudioRendererInterruptCallbackImpl();

    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback);
    void UpdateAudioStream(const std::shared_ptr<IAudioStream> &audioStream);
    void StartSwitch();
    void FinishSwitch();
private:
    void NotifyEvent(const InterruptEvent &interruptEvent);
    InterruptCallbackEvent HandleAndNotifyForcedEvent(const InterruptEventInternal &interruptEvent);
    void NotifyForcedEvent(const InterruptEventInternal &interruptEvent);
    void NotifyForcePausedToResume(const InterruptEventInternal &interruptEvent);
    bool HandleForceDucking(const InterruptEventInternal &interruptEvent);
    std::shared_ptr<IAudioStream> audioStream_;
    std::weak_ptr<AudioRendererCallback> callback_;
    std::shared_ptr<AudioRendererCallback> cb_;
    AudioInterrupt audioInterrupt_ {};
    bool isForcePaused_ = false;
    bool isForceDucked_ = false;
    uint32_t sessionID_ = INVALID_SESSION_ID;
    std::mutex mutex_;
    bool switching_ = false;
    std::condition_variable switchStreamCv_;
};

class AudioStreamCallbackRenderer : public AudioStreamCallback {
public:
    AudioStreamCallbackRenderer(std::weak_ptr<AudioRendererPrivate> renderer);
    virtual ~AudioStreamCallbackRenderer() = default;

    void OnStateChange(const State state, const StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    void SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback);
private:
    std::weak_ptr<AudioRendererCallback> callback_;
    std::weak_ptr<AudioRendererPrivate> renderer_;
};

class OutputDeviceChangeWithInfoCallbackImpl : public DeviceChangeWithInfoCallback {
public:
    OutputDeviceChangeWithInfoCallbackImpl() = default;
    virtual ~OutputDeviceChangeWithInfoCallbackImpl() = default;

    void OnDeviceChangeWithInfo(const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReasonExt reason) override;

    void OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override;

    void SaveCallback(const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks_.push_back(callback);
    }

    void RemoveCallback()
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks_.clear();
    }

    void RemoveCallback(const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), callback), callbacks_.end());
    }

    void SetAudioRendererObj(std::weak_ptr<AudioRendererPrivate> rendererObj)
    {
        std::lock_guard<std::mutex> lock(audioRendererObjMutex_);
        renderer_ = rendererObj;
    }

    void UnsetAudioRendererObj()
    {
        std::lock_guard<std::mutex> lock(audioRendererObjMutex_);
        renderer_.reset();
    }
private:
    std::vector<std::shared_ptr<AudioRendererOutputDeviceChangeCallback>> callbacks_;
    std::weak_ptr<AudioRendererPrivate> renderer_;
    std::mutex audioRendererObjMutex_;
    std::mutex callbackMutex_;
};

class RendererPolicyServiceDiedCallback : public AudioStreamPolicyServiceDiedCallback,
    public std::enable_shared_from_this<RendererPolicyServiceDiedCallback> {
public:
    RendererPolicyServiceDiedCallback();
    virtual ~RendererPolicyServiceDiedCallback();
    void SetAudioRendererObj(std::weak_ptr<AudioRendererPrivate> rendererObj);
    void SetAudioInterrupt(AudioInterrupt &audioInterrupt);
    void OnAudioPolicyServiceDied() override;

private:
    std::weak_ptr<AudioRendererPrivate> renderer_;
    AudioInterrupt audioInterrupt_;
    void RestoreTheadLoop();

    std::atomic<int32_t> taskCount_ = 0;
};

class FormatUnsupportedErrorCallbackImpl : public AudioFormatUnsupportedErrorCallback {
public:
    FormatUnsupportedErrorCallbackImpl() = default;
    virtual ~FormatUnsupportedErrorCallbackImpl() = default;
    void OnFormatUnsupportedError(const AudioErrors &errorCode) override;
private:
    std::weak_ptr<AudioRendererErrorCallback> callback_;
};

class AudioRouteCallbackImpl : public AudioRouteCallback {
public:
    AudioRouteCallbackImpl(std::weak_ptr<AudioRendererPrivate> renderer)
        : renderer_(renderer) {}
    virtual ~AudioRouteCallbackImpl() = default;
    void OnRouteUpdate(uint32_t routeFlag, const std::string &networkId) override;

private:
    std::weak_ptr<AudioRendererPrivate> renderer_;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_PRIVATE_H
