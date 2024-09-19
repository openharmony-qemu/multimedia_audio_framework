/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") = 0;
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
#ifndef I_AUDIO_STREAM_H
#define I_AUDIO_STREAM_H

#include <map>
#include <memory>
#include "timestamp.h"
#include "audio_info.h"
#include "audio_capturer.h"
#include "audio_renderer.h"
#include "audio_stream_manager.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioStreamCallback {
public:
    virtual ~AudioStreamCallback() = default;
    /**
     * Called when stream state is updated.
     *
     * @param state Indicates the InterruptEvent information needed by client.
     * For details, refer InterruptEvent struct in audio_info.h
     */
    virtual void OnStateChange(const State state, const StateChangeCmdType cmdType = CMD_FROM_CLIENT) = 0;
};

class RendererOrCapturerPolicyServiceDiedCallback {
public:
    virtual ~RendererOrCapturerPolicyServiceDiedCallback() = default;
    /**
     * Called when AudioPolicyService died.
     *
     * since 11
     */
    virtual void OnAudioPolicyServiceDied() = 0;
};

class IAudioStream {
public:
    enum StreamClass : uint32_t {
        PA_STREAM = 0,
        FAST_STREAM,
        VOIP_STREAM,
    };

    struct SwitchInfo {
        AudioStreamParams params;
        AudioStreamType eStreamType;
        int32_t appUid;
        AudioRendererInfo rendererInfo;
        AudioCapturerInfo capturerInfo;
        State state;
        uint32_t sessionId;
        std::string cachePath = "";
        uint32_t rendererSampleRate;
        uint32_t underFlowCount = 0;
        uint32_t overFlowCount = 0;
        AudioEffectMode effectMode;
        AudioRenderMode renderMode;
        AudioCaptureMode captureMode;
        AudioRendererRate renderRate;
        int32_t clientPid = 0;
        int32_t clientUid = 0;
        std::shared_ptr<AudioClientTracker> proxyObj;
        AudioPrivacyType privacyType;
        float volume;
        int32_t rendererFlags = AUDIO_FLAG_NORMAL;

        bool streamTrackerRegistered = false;

        uint64_t frameMarkPosition = 0;
        uint64_t framePeriodNumber = 0;

        uint64_t totalBytesWritten = 0;
        uint64_t framePeriodWritten = 0;
        std::shared_ptr<RendererPositionCallback> renderPositionCb;
        std::shared_ptr<RendererPeriodPositionCallback> renderPeriodPositionCb;

        uint64_t totalBytesRead = 0;
        uint64_t framePeriodRead = 0;
        std::shared_ptr<CapturerPositionCallback> capturePositionCb;
        std::shared_ptr<CapturerPeriodPositionCallback> capturePeriodPositionCb;

        // callback info
        std::shared_ptr<AudioStreamCallback> audioStreamCallback;
        std::shared_ptr<AudioRendererWriteCallback> rendererWriteCallback;
        std::shared_ptr<AudioCapturerReadCallback> capturerReadCallback;
        std::shared_ptr<AudioRendererFirstFrameWritingCallback> rendererFirstFrameWritingCallback;

        std::optional<int32_t> userSettedPreferredFrameSize = std::nullopt;
    };

    virtual ~IAudioStream() = default;

    static int32_t GetByteSizePerFrame(const AudioStreamParams &params, size_t &result);
    static bool IsStreamSupported(int32_t streamFlags, const AudioStreamParams &params);
    static std::shared_ptr<IAudioStream> GetPlaybackStream(StreamClass streamClass, AudioStreamParams params,
        AudioStreamType eStreamType, int32_t appUid);
    static std::shared_ptr<IAudioStream> GetRecordStream(StreamClass streamClass, AudioStreamParams params,
        AudioStreamType eStreamType, int32_t appUid);

    static AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);
    static std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap();
    static void CreateStreamMap(std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> &streamMap);
    static const std::string GetEffectSceneName(const StreamUsage &streamUsage);

    virtual int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) = 0;
    virtual void SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId) = 0;
    virtual void SetRendererInfo(const AudioRendererInfo &rendererInfo) = 0;
    virtual void SetCapturerInfo(const AudioCapturerInfo &capturerInfo) = 0;
    virtual int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj) = 0;
    virtual int32_t GetAudioStreamInfo(AudioStreamParams &info) = 0;
    virtual bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        SourceType sourceType = SOURCE_TYPE_MIC) = 0;
    virtual bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) = 0;
    virtual int32_t GetAudioSessionID(uint32_t &sessionID) = 0;
    virtual void GetAudioPipeType(AudioPipeType &pipeType) = 0;
    virtual State GetState() = 0;
    virtual bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) = 0;
    virtual bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) = 0;
    virtual int32_t GetBufferSize(size_t &bufferSize) = 0;
    virtual int32_t GetFrameCount(uint32_t &frameCount) = 0;
    virtual int32_t GetLatency(uint64_t &latency) = 0;
    virtual int32_t SetAudioStreamType(AudioStreamType audioStreamType) = 0;
    virtual int32_t SetVolume(float volume) = 0;
    virtual float GetVolume() = 0;
    virtual int32_t SetDuckVolume(float volume) = 0;
    virtual int32_t SetRenderRate(AudioRendererRate renderRate) = 0;
    virtual AudioRendererRate GetRenderRate() = 0;
    virtual int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback) = 0;
    virtual int32_t SetSpeed(float speed) = 0;
    virtual float GetSpeed() = 0;
    virtual int32_t ChangeSpeed(uint8_t *buffer, int32_t bufferSize,
        std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize) = 0;

    virtual void SetUnderflowCount(uint32_t underflowCount) = 0;
    virtual void SetOverflowCount(uint32_t overflowCount) = 0;

    // callback mode api
    virtual int32_t SetRenderMode(AudioRenderMode renderMode) = 0;
    virtual AudioRenderMode GetRenderMode() = 0;
    virtual int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) = 0;

    virtual int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) = 0;
    virtual void OnFirstFrameWriting() = 0;

    virtual int32_t SetCaptureMode(AudioCaptureMode captureMode) = 0;
    virtual AudioCaptureMode GetCaptureMode() = 0;
    virtual int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) = 0;

    virtual int32_t GetBufferDesc(BufferDesc &bufDesc) = 0;
    virtual int32_t GetBufQueueState(BufferQueueState &bufState) = 0;
    virtual int32_t Enqueue(const BufferDesc &bufDesc) = 0;
    virtual int32_t Clear() = 0;

    virtual int32_t SetLowPowerVolume(float volume) = 0;
    virtual float GetLowPowerVolume() = 0;
    virtual float GetSingleStreamVolume() = 0;
    virtual int32_t SetOffloadMode(int32_t state, bool isAppBack) = 0;
    virtual int32_t UnsetOffloadMode() = 0;

    // for effect
    virtual AudioEffectMode GetAudioEffectMode() = 0;
    virtual int32_t SetAudioEffectMode(AudioEffectMode effectMode) = 0;

    virtual int64_t GetFramesWritten() = 0;
    virtual int64_t GetFramesRead() = 0;

    // Common APIs
    virtual bool StartAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT,
        AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN) = 0;
    virtual bool PauseAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) = 0;
    virtual bool StopAudioStream() = 0;
    virtual bool ReleaseAudioStream(bool releaseRunner = true) = 0;
    virtual bool FlushAudioStream() = 0;

    // Playback related APIs
    virtual bool DrainAudioStream(bool stopFlag = false) = 0;
    virtual int32_t Write(uint8_t *buffer, size_t buffer_size) = 0;
    virtual int32_t Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize) = 0;

    // Recording related APIs
    virtual int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) = 0;

    // for inner capturer
    virtual void SetInnerCapturerState(bool isInnerCapturer) = 0;
    virtual void SetPrivacyType(AudioPrivacyType privacyType) = 0;

    virtual uint32_t GetUnderflowCount() = 0;
    virtual uint32_t GetOverflowCount() = 0;

    virtual void SetRendererPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPositionCallback> &callback) = 0;
    virtual void UnsetRendererPositionCallback() = 0;

    virtual void SetRendererPeriodPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) = 0;
    virtual void UnsetRendererPeriodPositionCallback() = 0;

    virtual void SetCapturerPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPositionCallback> &callback) = 0;
    virtual void UnsetCapturerPositionCallback() = 0;

    virtual void SetCapturerPeriodPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) = 0;

    virtual void UnsetCapturerPeriodPositionCallback() = 0;
    virtual int32_t SetRendererSamplingRate(uint32_t sampleRate) = 0;
    virtual uint32_t GetRendererSamplingRate() = 0;
    virtual int32_t SetBufferSizeInMsec(int32_t bufferSizeInMsec) = 0;
    virtual void SetApplicationCachePath(const std::string cachePath) = 0;
    virtual int32_t SetChannelBlendMode(ChannelBlendMode blendMode) = 0;
    virtual int32_t SetVolumeWithRamp(float volume, int32_t duration) = 0;
    virtual void SetPreferredFrameSize(int32_t frameSize) = 0;
    virtual IAudioStream::StreamClass GetStreamClass() = 0;
    virtual void SetStreamTrackerState(bool trackerRegisteredState) = 0;
    virtual void GetSwitchInfo(SwitchInfo& info) = 0;

    // for get pipetype
    virtual bool GetOffloadEnable() = 0;

    virtual bool GetSpatializationEnabled() = 0;

    virtual bool GetHighResolutionEnabled() = 0;

    //for wakeup capturer
    virtual void SetWakeupCapturerState(bool isWakeupCapturer) = 0;

    virtual void SetCapturerSource(int capturerSource) = 0;

    virtual void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) = 0;

    virtual int32_t RegisterRendererOrCapturerPolicyServiceDiedCB(
        const std::shared_ptr<RendererOrCapturerPolicyServiceDiedCallback> &callback)
    {
        return 0;
    }

    virtual int32_t RemoveRendererOrCapturerPolicyServiceDiedCB()
    {
        return 0;
    }

    virtual bool RestoreAudioStream()
    {
        return 0;
    }

    virtual void SetState() {}

    bool IsFormatValid(uint8_t format);

    bool IsRendererChannelValid(uint8_t channel);

    bool IsCapturerChannelValid(uint8_t channel);

    bool IsEncodingTypeValid(uint8_t encodingType);

    bool IsSamplingRateValid(uint32_t samplingRate);

    bool IsRendererChannelLayoutValid(uint64_t channelLayout);

    bool IsCapturerChannelLayoutValid(uint64_t channelLayout);

    bool IsPlaybackChannelRelatedInfoValid(uint8_t channels, uint64_t channelLayout);

    bool IsRecordChannelRelatedInfoValid(uint8_t channels, uint64_t channelLayout);

    virtual void SetSilentModeAndMixWithOthers(bool on) = 0;

    virtual bool GetSilentModeAndMixWithOthers() = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_STREAM_H
