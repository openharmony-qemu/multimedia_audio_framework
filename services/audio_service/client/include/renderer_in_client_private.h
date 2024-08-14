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
#ifndef RENDERER_IN_CLIENT_PRIVATE_H
#define RENDERER_IN_CLIENT_PRIVATE_H

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"

#include "audio_manager_base.h"
#include "audio_ring_cache.h"
#include "audio_channel_blend.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "ipc_stream_listener_impl.h"
#include "ipc_stream_listener_stub.h"
#include "volume_ramp.h"
#include "volume_tools.h"
#include "callback_handler.h"
#include "audio_speed.h"
#include "audio_spatial_channel_converter.h"
#include "audio_policy_manager.h"
#include "audio_spatialization_manager.h"

namespace OHOS {
namespace AudioStandard {
class SpatializationStateChangeCallbackImpl;

class RendererInClientPolicyServiceDiedCallbackImpl : public AudioStreamPolicyServiceDiedCallback {
public:
    RendererInClientPolicyServiceDiedCallbackImpl();
    virtual ~RendererInClientPolicyServiceDiedCallbackImpl();
    void OnAudioPolicyServiceDied() override;
    void SaveRendererOrCapturerPolicyServiceDiedCB(
        const std::shared_ptr<RendererOrCapturerPolicyServiceDiedCallback> &callback);
    void RemoveRendererOrCapturerPolicyServiceDiedCB();

private:
    std::mutex mutex_;
    std::shared_ptr<RendererOrCapturerPolicyServiceDiedCallback> policyServiceDiedCallback_;
};

class RendererInClientInner : public RendererInClient, public IStreamListener, public IHandler,
    public std::enable_shared_from_this<RendererInClientInner> {
public:
    RendererInClientInner(AudioStreamType eStreamType, int32_t appUid);
    ~RendererInClientInner();

    // IStreamListener
    int32_t OnOperationHandled(Operation operation, int64_t result) override;

    // IAudioStream
    void SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId) override;

    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override;
    void SetRendererInfo(const AudioRendererInfo &rendererInfo) override;
    void SetCapturerInfo(const AudioCapturerInfo &capturerInfo) override;
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj) override;
    int32_t GetAudioStreamInfo(AudioStreamParams &info) override;
    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid, SourceType sourceType =
        SOURCE_TYPE_MIC) override;
    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) override;
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
    float GetVolume() override;
    int32_t SetDuckVolume(float volume) override;
    int32_t SetRenderRate(AudioRendererRate renderRate) override;
    AudioRendererRate GetRenderRate() override;
    int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback) override;
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override;
    void OnFirstFrameWriting() override;
    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
    int32_t ChangeSpeed(uint8_t *buffer, int32_t bufferSize, std::unique_ptr<uint8_t[]> &outBuffer,
        int32_t &outBufferSize) override;

    // callback mode api
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
    bool ReleaseAudioStream(bool releaseRunner = true) override;
    bool FlushAudioStream() override;

    // Playback related APIs
    bool DrainAudioStream(bool stopFlag = false) override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    int32_t Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize) override;
    void SetPreferredFrameSize(int32_t frameSize) override;

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override;

    uint32_t GetUnderflowCount() override;
    uint32_t GetOverflowCount() override;
    void SetUnderflowCount(uint32_t underflowCount) override;
    void SetOverflowCount(uint32_t overflowCount) override;

    void SetRendererPositionCallback(int64_t markPosition, const std::shared_ptr<RendererPositionCallback> &callback)
        override;
    void UnsetRendererPositionCallback() override;
    void SetRendererPeriodPositionCallback(int64_t periodPosition,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;
    void SetCapturerPositionCallback(int64_t markPosition, const std::shared_ptr<CapturerPositionCallback> &callback)
        override;
    void UnsetCapturerPositionCallback() override;
    void SetCapturerPeriodPositionCallback(int64_t periodPosition,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override;
    void UnsetCapturerPeriodPositionCallback() override;
    int32_t SetRendererSamplingRate(uint32_t sampleRate) override;
    uint32_t GetRendererSamplingRate() override;
    int32_t SetBufferSizeInMsec(int32_t bufferSizeInMsec) override;
    void SetApplicationCachePath(const std::string cachePath) override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override;

    void SetStreamTrackerState(bool trackerRegisteredState) override;
    void GetSwitchInfo(IAudioStream::SwitchInfo& info) override;

    IAudioStream::StreamClass GetStreamClass() override;

    static const sptr<IStandardAudioService> GetAudioServerProxy();
    static void AudioServerDied(pid_t pid);

    void OnHandle(uint32_t code, int64_t data) override;
    void InitCallbackHandler();
    void SafeSendCallbackEvent(uint32_t eventCode, int64_t data);

    int32_t StateCmdTypeToParams(int64_t &params, State state, StateChangeCmdType cmdType);
    int32_t ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType);

    void SendRenderMarkReachedEvent(int64_t rendererMarkPosition);
    void SendRenderPeriodReachedEvent(int64_t rendererPeriodSize);

    void HandleRendererPositionChanges(size_t bytesWritten);
    void HandleStateChangeEvent(int64_t data);
    void HandleRenderMarkReachedEvent(int64_t rendererMarkPosition);
    void HandleRenderPeriodReachedEvent(int64_t rendererPeriodNumber);

    void OnSpatializationStateChange(const AudioSpatializationState &spatializationState);
    void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) override;

    int32_t RegisterRendererOrCapturerPolicyServiceDiedCB(
        const std::shared_ptr<RendererOrCapturerPolicyServiceDiedCallback> &callback) override;
    int32_t RemoveRendererOrCapturerPolicyServiceDiedCB() override;
    bool RestoreAudioStream() override;

    void GetStreamSwitchInfo(SwitchInfo &info);

    bool GetOffloadEnable() override;
    bool GetSpatializationEnabled() override;
    bool GetHighResolutionEnabled() override;

    void SetSilentModeAndMixWithOthers(bool on) override;
    bool GetSilentModeAndMixWithOthers() override;

private:
    void RegisterTracker(const std::shared_ptr<AudioClientTracker> &proxyObj);
    void UpdateTracker(const std::string &updateCase);

    int32_t DeinitIpcStream();

    int32_t InitIpcStream();

    const AudioProcessConfig ConstructConfig();

    int32_t InitSharedBuffer();
    int32_t InitCacheBuffer(size_t targetSize);

    int32_t FlushRingCache();
    int32_t DrainRingCache();

    int32_t WriteCacheData(bool isDrain = false);

    void InitCallbackBuffer(uint64_t bufferDurationInUs);
    void WriteCallbackFunc();
    // for callback mode. Check status if not running, wait for start or release.
    bool WaitForRunning();
    bool ProcessSpeed(uint8_t *&buffer, size_t &bufferSize, bool &speedCached);
    int32_t WriteInner(uint8_t *buffer, size_t bufferSize);
    int32_t WriteInner(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize);
    void WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize);

    int32_t RegisterSpatializationStateEventListener();

    int32_t UnregisterSpatializationStateEventListener(uint32_t sessionID);

    void FirstFrameProcess();

    int32_t WriteRingCache(uint8_t *buffer, size_t bufferSize, bool speedCached, size_t oriBufferSize);

    void ResetFramePosition();

    int32_t RegisterRendererInClientPolicyServerDiedCb();
    int32_t UnregisterRendererInClientPolicyServerDiedCb();

    void ReportDataToResSched();

    int32_t SetInnerVolume(float volume);

    bool IsHighResolution() const noexcept;

    void ProcessWriteInner(BufferDesc &bufferDesc);

    void ResetRingerModeMute();
private:
    AudioStreamType eStreamType_ = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid_ = 0;
    uint32_t sessionId_ = 0;
    int32_t clientPid_ = -1;
    int32_t clientUid_ = -1;
    uint32_t appTokenId_ = 0;
    uint64_t fullTokenId_ = 0;

    std::unique_ptr<AudioStreamTracker> audioStreamTracker_;

    AudioRendererInfo rendererInfo_ = {};
    AudioCapturerInfo capturerInfo_ = {}; // not in use

    AudioPrivacyType privacyType_ = PRIVACY_TYPE_PUBLIC;
    bool streamTrackerRegistered_ = false;

    bool needSetThreadPriority_ = true;

    AudioStreamParams curStreamParams_ = {0}; // in plan next: replace it with AudioRendererParams
    AudioStreamParams streamParams_ = {0};

    // for data process
    bool isBlendSet_ = false;
    AudioBlend audioBlend_;
    VolumeRamp volumeRamp_;

    // callbacks
    std::mutex streamCbMutex_;
    std::weak_ptr<AudioStreamCallback> streamCallback_;

    size_t cacheSizeInByte_ = 0;
    uint32_t spanSizeInFrame_ = 0;
    size_t clientSpanSizeInByte_ = 0;
    size_t sizePerFrameInByte_ = 4; // 16bit 2ch as default

    uint32_t bufferSizeInMsec_ = 20; // 20ms
    std::string cachePath_ = "";
    std::string dumpOutFile_ = "";
    FILE *dumpOutFd_ = nullptr;

    std::shared_ptr<AudioRendererFirstFrameWritingCallback> firstFrameWritingCb_ = nullptr;
    bool hasFirstFrameWrited_ = false;

    // callback mode releated
    AudioRenderMode renderMode_ = RENDER_MODE_NORMAL;
    std::thread callbackLoop_; // thread for callback to client and write.
    std::atomic<bool> cbThreadReleased_ = true;
    std::mutex writeCbMutex_;
    std::condition_variable cbThreadCv_;
    std::shared_ptr<AudioRendererWriteCallback> writeCb_ = nullptr;
    std::mutex cbBufferMutex_;
    std::condition_variable cbBufferCV_;
    std::unique_ptr<uint8_t[]> cbBuffer_ {nullptr};
    size_t cbBufferSize_ = 0;
    AudioSafeBlockQueue<BufferDesc> cbBufferQueue_; // only one cbBuffer_

    std::atomic<State> state_ = INVALID;
    // using this lock when change status_
    std::mutex statusMutex_;
    // for status operation wait and notify
    std::mutex callServerMutex_;
    std::condition_variable callServerCV_;

    Operation notifiedOperation_ = MAX_OPERATION_CODE;
    int64_t notifiedResult_ = 0;

    int32_t continueDownCount_ = 0;
    float lowPowerVolume_ = 1.0;
    float duckVolume_ = 1.0;
    float clientVolume_ = 1.0;
    bool silentModeAndMixWithOthers_ = false;

    uint64_t clientWrittenBytes_ = 0;
    // ipc stream related
    AudioProcessConfig clientConfig_;
    sptr<IpcStreamListenerImpl> listener_ = nullptr;
    sptr<IpcStream> ipcStream_ = nullptr;
    std::shared_ptr<OHAudioBuffer> clientBuffer_ = nullptr;

    // buffer handle
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    std::mutex writeMutex_; // used for prevent multi thread call write

    // Mark reach and period reach callback
    int64_t totalBytesWritten_ = 0;
    std::mutex markReachMutex_;
    bool rendererMarkReached_ = false;
    int64_t rendererMarkPosition_ = 0;
    std::shared_ptr<RendererPositionCallback> rendererPositionCallback_ = nullptr;

    std::mutex periodReachMutex_;
    int64_t rendererPeriodSize_ = 0;
    int64_t rendererPeriodWritten_ = 0;
    std::shared_ptr<RendererPeriodPositionCallback> rendererPeriodPositionCallback_ = nullptr;

    // Event handler
    bool runnerReleased_ = false;
    std::mutex runnerMutex_;
    std::shared_ptr<CallbackHandler> callbackHandler_ = nullptr;

    bool paramsIsSet_ = false;
    AudioRendererRate rendererRate_ = RENDER_RATE_NORMAL;
    AudioEffectMode effectMode_ = EFFECT_DEFAULT;

    float speed_ = 1.0;
    std::unique_ptr<uint8_t[]> speedBuffer_ {nullptr};
    size_t bufferSize_ = 0;
    std::unique_ptr<AudioSpeed> audioSpeed_ = nullptr;

    std::unique_ptr<AudioSpatialChannelConverter> converter_;

    bool offloadEnable_ = false;
    uint64_t offloadStartReadPos_ = 0;
    int64_t offloadStartHandleTime_ = 0;

    uint64_t lastFramePosition_ = 0;
    uint64_t lastFrameTimestamp_ = 0;

    std::string traceTag_;
    std::string spatializationEnabled_ = "Invalid";
    std::string headTrackingEnabled_ = "Invalid";
    uint32_t spatializationRegisteredSessionID_ = 0;
    bool firstSpatializationRegistered_ = true;
    std::shared_ptr<SpatializationStateChangeCallbackImpl> spatializationStateChangeCallback_ = nullptr;
    std::time_t startMuteTime_ = 0;
    bool isUpEvent_ = false;
    std::shared_ptr<RendererInClientPolicyServiceDiedCallbackImpl> policyServiceDiedCB_ = nullptr;
    std::shared_ptr<AudioClientTracker> proxyObj_ = nullptr;

    uint64_t lastFlushPosition_ = 0;

    enum {
        STATE_CHANGE_EVENT = 0,
        RENDERER_MARK_REACHED_EVENT,
        RENDERER_PERIOD_REACHED_EVENT,
        CAPTURER_PERIOD_REACHED_EVENT,
        CAPTURER_MARK_REACHED_EVENT,
    };

    // note that the starting elements should remain the same as the enum State
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

class SpatializationStateChangeCallbackImpl : public AudioSpatializationStateChangeCallback {
public:
    SpatializationStateChangeCallbackImpl();
    virtual ~SpatializationStateChangeCallbackImpl();

    void OnSpatializationStateChange(const AudioSpatializationState &spatializationState) override;
    void SetRendererInClientPtr(std::shared_ptr<RendererInClientInner> rendererInClientPtr);
private:
    std::weak_ptr<RendererInClientInner> rendererInClientPtr_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // RENDERER_IN_SERVER_H
