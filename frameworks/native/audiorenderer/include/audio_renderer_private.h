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

#include "audio_interrupt_callback.h"
#include "audio_concurrency_callback.h"
#include "audio_renderer.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_utils.h"
#include "i_audio_stream.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t INVALID_SESSION_ID = static_cast<uint32_t>(-1);
class RendererPolicyServiceDiedCallback;
class OutputDeviceChangeWithInfoCallbackImpl;
class AudioRendererConcurrencyCallbackImpl;

class AudioRendererPrivate : public AudioRenderer {
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
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool Drain() const override;
    bool PauseTransitent(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Pause(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Stop() override;
    bool Flush() const override;
    bool Release() override;
    int32_t GetBufferSize(size_t &bufferSize) const override;
    int32_t GetAudioStreamId(uint32_t &sessionID) const override;
    int32_t SetAudioRendererDesc(AudioRendererDesc audioRendererDesc) override;
    int32_t SetStreamType(AudioStreamType audioStreamType) override;
    int32_t SetVolume(float volume) const override;
    float GetVolume() const override;
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
    int32_t GetBufferDesc(BufferDesc &bufDesc) const override;
    int32_t Enqueue(const BufferDesc &bufDesc) const override;
    int32_t Clear() const override;
    int32_t GetBufQueueState(BufferQueueState &bufState) const override;
    void SetApplicationCachePath(const std::string cachePath) override;
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
    void SwitchStream(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);

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

    void GetAudioInterrupt(AudioInterrupt &audioInterrupt);

    bool IsFastRenderer() override;

    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
#ifdef SONIC_ENABLE
    int32_t ChangeSpeed(uint8_t *buffer, int32_t bufferSize);
    int32_t ChangeSpeedFor8Bit(uint8_t *buffer, int32_t bufferSize,
        std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize);
    int32_t ChangeSpeedFor16Bit(uint8_t *buffer, int32_t bufferSize,
        std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize);
    int32_t ChangeSpeedFor24Bit(uint8_t *buffer, int32_t bufferSize,
        std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize);
    int32_t ChangeSpeedFor32Bit(uint8_t *buffer, int32_t bufferSize,
        std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize);
    int32_t ChangeSpeedForFloat(float *buffer, int32_t bufferSize, float* outBuffer, int32_t &outBufferSize);
    int32_t WriteSpeedBuffer(int32_t bufferSize, uint8_t *speedBuffer, size_t speedBufferSize);
#endif
    void ConcedeStream();

    void SetSilentModeAndMixWithOthers(bool on) override;
    bool GetSilentModeAndMixWithOthers() override;

    void EnableVoiceModemCommunicationStartStream(bool enable) override;

    bool IsNoStreamRenderer() const override;
    void RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter);

    int32_t SetDefaultOutputDevice(DeviceType deviceType) override;

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
    std::string cachePath_;
    std::shared_ptr<IAudioStream> audioStream_;
    bool abortRestore_ = false;
    mutable bool isStillMuted_ = false;

    explicit AudioRendererPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo, bool createStream = true);

    ~AudioRendererPrivate();

protected:
    // Method for switching between normal and low latency paths
    void SwitchStream(bool isLowLatencyDevice, bool isHalNeedChange);

private:
    int32_t PrepareAudioStream(const AudioStreamParams &audioStreamParams,
        const AudioStreamType &audioStreamType, IAudioStream::StreamClass &streamClass);
    int32_t InitAudioInterruptCallback();
    int32_t InitOutputDeviceChangeCallback();
    int32_t InitAudioStream(AudioStreamParams audioStreamParams);
    int32_t InitAudioConcurrencyCallback();
    void SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream);
    void UpdateRendererAudioStream(const std::shared_ptr<IAudioStream> &audioStream);
    void InitSwitchInfo(IAudioStream::StreamClass targetClass, IAudioStream::SwitchInfo &info);
    bool SwitchToTargetStream(IAudioStream::StreamClass targetClass, uint32_t &newSessionId,
        const AudioStreamDeviceChangeReasonExt reason);
    void InitLatencyMeasurement(const AudioStreamParams &audioStreamParams);
    void MockPcmData(uint8_t *buffer, size_t bufferSize) const;
    void ActivateAudioConcurrency(const AudioStreamParams &audioStreamParams,
        const AudioStreamType &audioStreamType, IAudioStream::StreamClass &streamClass);
    void WriteUnderrunEvent() const;
    IAudioStream::StreamClass GetPreferredStreamClass(AudioStreamParams audioStreamParams);
    bool IsDirectVoipParams(const AudioStreamParams &audioStreamParams);
    void UpdateAudioInterruptStrategy(float volume) const;
    void WriteSwitchStreamLogMsg();

    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;
    std::shared_ptr<AudioStreamCallback> audioStreamCallback_ = nullptr;
    std::shared_ptr<AudioRendererConcurrencyCallbackImpl> audioConcurrencyCallback_ = nullptr;
    AppInfo appInfo_ = {};
    AudioInterrupt audioInterrupt_ = {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, 0};
    uint32_t sessionID_ = INVALID_SESSION_ID;
    std::shared_ptr<AudioRendererProxyObj> rendererProxyObj_;
    FILE *dumpFile_ = nullptr;
    std::shared_ptr<AudioRendererErrorCallback> audioRendererErrorCallback_ = nullptr;
    std::mutex audioRendererErrCallbackMutex_;
    std::shared_ptr<OutputDeviceChangeWithInfoCallbackImpl> outputDeviceChangeCallback_ = nullptr;
    mutable std::shared_ptr<RendererPolicyServiceDiedCallback> audioPolicyServiceDiedCallback_ = nullptr;
    bool isFastRenderer_ = false;
    bool latencyMeasEnabled_ = false;
    std::shared_ptr<AudioLatencyMeasurement> latencyMeasurement_ = nullptr;
    bool isSwitching_ = false;
    mutable std::shared_mutex rendererMutex_;
    mutable AudioRenderMode audioRenderMode_ = RENDER_MODE_NORMAL;
    bool isFastVoipSupported_ = false;
    bool isDirectVoipSupported_ = false;
    bool isEnableVoiceModemCommunicationStartStream_ = false;
    RendererState state_ = RENDERER_INVALID;
    DeviceType selectedDefaultOutputDevice_ = DEVICE_TYPE_NONE;

    float speed_ = 1.0;

    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> policyServiceDiedCallback_ = nullptr;
    std::mutex policyServiceDiedCallbackMutex_;

    std::vector<uint32_t> usedSessionId_ = {};
    std::mutex silentModeAndMixWithOthersMutex_;
    std::mutex setStreamCallbackMutex_;
    std::mutex setParamsMutex_;
    std::mutex rendererPolicyServiceDiedCbMutex_;
    int64_t framesAlreadyWritten_ = 0;
};

class AudioRendererInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioRendererInterruptCallbackImpl(const std::shared_ptr<IAudioStream> &audioStream,
        const AudioInterrupt &audioInterrupt);
    virtual ~AudioRendererInterruptCallbackImpl();

    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback);
    void UpdateAudioStream(const std::shared_ptr<IAudioStream> &audioStream);
private:
    void NotifyEvent(const InterruptEvent &interruptEvent);
    void HandleAndNotifyForcedEvent(const InterruptEventInternal &interruptEvent);
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
};

class AudioStreamCallbackRenderer : public AudioStreamCallback {
public:
    virtual ~AudioStreamCallbackRenderer() = default;

    void OnStateChange(const State state, const StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    void SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback);
private:
    std::weak_ptr<AudioRendererCallback> callback_;
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

    void SetAudioRendererObj(AudioRendererPrivate *rendererObj)
    {
        std::lock_guard<std::mutex> lock(audioRendererObjMutex_);
        renderer_ = rendererObj;
    }

    void UnsetAudioRendererObj()
    {
        std::lock_guard<std::mutex> lock(audioRendererObjMutex_);
        renderer_ = nullptr;
    }
private:
    std::vector<std::shared_ptr<AudioRendererOutputDeviceChangeCallback>> callbacks_;
    AudioRendererPrivate *renderer_ = nullptr;
    std::mutex audioRendererObjMutex_;
    std::mutex callbackMutex_;
};

class RendererPolicyServiceDiedCallback : public AudioStreamPolicyServiceDiedCallback {
public:
    RendererPolicyServiceDiedCallback();
    virtual ~RendererPolicyServiceDiedCallback();
    void SetAudioRendererObj(AudioRendererPrivate *rendererObj);
    void SetAudioInterrupt(AudioInterrupt &audioInterrupt);
    void OnAudioPolicyServiceDied() override;

private:
    AudioRendererPrivate *renderer_ = nullptr;
    AudioInterrupt audioInterrupt_;
    void RestoreTheadLoop();
    std::unique_ptr<std::thread> restoreThread_ = nullptr;
};

class AudioRendererConcurrencyCallbackImpl : public AudioConcurrencyCallback {
public:
    explicit AudioRendererConcurrencyCallbackImpl();
    virtual ~AudioRendererConcurrencyCallbackImpl();
    void OnConcedeStream() override;
    void SetAudioRendererObj(AudioRendererPrivate *rendererObj)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        renderer_ = rendererObj;
    }
    void UnsetAudioRendererObj()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        renderer_ = nullptr;
    }
private:
    AudioRendererPrivate *renderer_ = nullptr;
    std::mutex mutex_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_PRIVATE_H
