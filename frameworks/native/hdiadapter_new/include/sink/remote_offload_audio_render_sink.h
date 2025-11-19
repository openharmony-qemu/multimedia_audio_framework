/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef REMOTE_OFFLOAD_AUDIO_RENDER_SINK_H
#define REMOTE_OFFLOAD_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include <v1_0/iaudio_manager.h>
#include "audio_utils.h"
#include "adapter/i_device_manager.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback RemoteIAudioCallback;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey RemoteAudioExtParamKey;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender RemoteIAudioRender;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat RemoteAudioFormat;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes RemoteAudioSampleAttributes;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor RemoteAudioDeviceDescriptor;
typedef OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCallbackType RemoteAudioCallbackType;

class RemoteOffloadAudioRenderSink;

class RemoteOffloadHdiCallbackImpl final : public RemoteIAudioCallback {
public:
    RemoteOffloadHdiCallbackImpl(RemoteOffloadAudioRenderSink *sink);
    ~RemoteOffloadHdiCallbackImpl() override {}

    int32_t RenderCallback(RemoteAudioCallbackType type, int8_t &reserved, int8_t &cookie) override;
    int32_t ParamCallback(RemoteAudioExtParamKey key, const std::string &condition, const std::string &value,
        int8_t &reserved, int8_t cookie) override;

private:
    RemoteOffloadAudioRenderSink *sink_ = nullptr;
};

struct RemoteOffloadHdiCallback {
    sptr<RemoteIAudioCallback> callback_;
    std::function<void(const RenderCallbackType type)> serviceCallback_;
};

class RemoteOffloadAudioRenderSink : public IAudioRenderSink, public IDeviceManagerCallback {
    friend class RemoteOffloadHdiCallbackImpl;
public:
    explicit RemoteOffloadAudioRenderSink(const std::string &deviceNetworkId);
    ~RemoteOffloadAudioRenderSink();

    int32_t Init(const IAudioSinkAttr &attr) override;
    void DeInit(void) override;
    bool IsInited(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Resume(void) override;
    int32_t Pause(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int64_t GetVolumeDataCount(void) override;

    int32_t SuspendRenderSink(void) override;
    int32_t RestoreRenderSink(void) override;

    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;

    int32_t GetLatency(uint32_t &latency) override;
    int32_t GetTransactionId(uint64_t &transactionId) override;
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude(void) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;
    void SetSpeed(float speed) override;

    int32_t SetAudioScene(AudioScene audioScene, bool scoExcludeFlag = false) override;
    int32_t GetAudioScene(void) override;

    int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) override;
    void RegistCallback(uint32_t type, IAudioSinkCallback *callback) override;
    void ResetActiveDeviceForDisconnect(DeviceType device) override;

    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower(void) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t Drain(AudioDrainType type) override;
    void RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> callback) override;
    int32_t SetBufferSize(uint32_t sizeMs) override;
    int32_t SetOffloadRenderCallbackType(RenderCallbackType type) override;
    int32_t LockOffloadRunningLock(void) override;
    int32_t UnLockOffloadRunningLock(void) override;

    void DumpInfo(std::string &dumpString) override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;

    int32_t GetHdiPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    int32_t GetHdiLatency(uint32_t &latency) override;
    int32_t ForceRefreshPresentationPosition(uint64_t &frames, uint64_t &hdiFrames, int64_t &timeSec,
        int64_t &timeNanoSec) override;

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override;

private:
    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static RemoteAudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    void InitAudioSampleAttr(RemoteAudioSampleAttributes &param);
    void InitDeviceDesc(RemoteAudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(void);
    void InitLatencyMeasurement(void);
    void DeInitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *data, size_t len);
    void CheckUpdateState(char *data, uint64_t len);
    int32_t SetVolumeInner(float left, float right);
    void UpdateSinkState(bool started);
    int32_t GetRenderPositionInner();
    void AddHdiLatency(uint64_t duration);
    void RemoveHdiLatency(uint64_t duration);
    void CheckHdiTime(int64_t &timeSec, int64_t &timeNanoSec);
    int32_t GetLatencyInner();
    void CalcHdiPosition(uint64_t frames, int64_t timeSec, int64_t timeNanoSec);
    void FlushResetPosition();
    int32_t EstimateRenderPosition();
    int32_t FlushInner(void);
    void CheckFlushThread();

private:
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;
    static constexpr float DEFAULT_VOLUME_LEVEL = 1.0f;
    static constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr size_t OFFLOAD_DFX_SPLIT = 2;
#ifdef FEATURE_POWER_MANAGER
    static constexpr const char *RUNNING_LOCK_NAME = "AudioRemoteOffloadBackgroundPlay";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif

    const std::string deviceNetworkId_ = "";
    IAudioSinkAttr attr_ = {};
    SinkCallbackWrapper callback_ = {};
    struct RemoteOffloadHdiCallback hdiCallback_ = {};
    std::atomic<bool> sinkInited_ = false;
    std::atomic<bool> renderInited_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    std::atomic<bool> isFlushing_ = false;
    bool isNeedRestart_ = false;
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    uint32_t hdiRenderId_ = HDI_INVALID_ID;
    sptr<RemoteIAudioRender> audioRender_ = nullptr;
    // for signal detect
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    bool signalDetected_ = false;
    size_t signalDetectedTime_ = 0;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
    // for device switch
    std::mutex switchDeviceMutex_;
    int32_t muteCount_ = 0;
    std::atomic<bool> switchDeviceMute_ = false;
    // for dfx log
    std::string logUtilsTag_ = "RemoteOffloadSink";
    mutable int64_t volumeDataCount_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
    bool runningLocked_ = false;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    uint64_t renderPos_ = 0;
    std::mutex sinkMutex_;
    // sample count (before scaling to one times speed)
    uint64_t lastHdiFramesUS_ = 0;
    // sample count (after scaling to one times speed)
    uint64_t lastHdiOriginFramesUS_ = 0;
    // Audio playback rate
    float speed_ = 1.0f;
    // Delay queue: pair <latency duration, speed>
    std::deque<std::pair<uint64_t, uint64_t>> realLatencyDeque_;
    // Delay queue real length
    uint64_t realLatencyTotalUS_ = 0;
    // remote offload hdi latency (us)
    uint64_t hdiLatencyUS_ = 0;
    // The timestamp (in microseconds) of the last HDI flush operation for frames (before scaling to one times speed)
    uint64_t lastHdiFlushFramesUS_ = 0;
    // The timestamp (in microseconds) of the last HDI flush operation for frames (after scaling to one times speed)
    uint64_t lastHdiOriginFlushFramesUS_ = 0;
    // The hdi timestamp (in nanoseconds) of the last get audio position operation
    int64_t lastHdiTimeNS_ = 0;
    // The system timestamp (in nanoseconds) of the last get audio position operation
    int64_t lastSystemTimeNS_ = 0;
    int64_t lastHdiTimeSec_ = 0;
    int64_t lastHdiTimeNanoSec_ = 0;
    std::shared_ptr<std::thread> flushThread_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_OFFLOAD_AUDIO_RENDER_SINK_H
