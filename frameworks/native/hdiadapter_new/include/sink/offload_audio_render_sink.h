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

#ifndef OFFLOAD_AUDIO_RENDER_SINK_H
#define OFFLOAD_AUDIO_RENDER_SINK_H

#include "sink/i_audio_render_sink.h"
#include <iostream>
#include <cstring>
#include <atomic>
#include <condition_variable>
#include <future>
#include <queue>
#include <thread>
#include "v6_1/iaudio_manager.h"
#include "audio_utils.h"
#include "util/audio_running_lock.h"
#include "util/callback_wrapper.h"

namespace OHOS {
namespace AudioStandard {
struct OffloadHdiCallback {
    struct IAudioCallback callback_;
    std::function<void(const RenderCallbackType type)> serviceCallback_;
    void *sink_;
};

// Control-plane actor: one resident ctrl thread serializes all HDI control calls and
// owns the flush state machine. Ownership constraint: instances MUST be shared_ptr-owned
// (the ctrl thread holds a self-ref for its whole lifetime). Abandon constraint: if
// DeInit times out on shutdown it detaches the ctrl thread and skips teardown on
// purpose (UAF over leak); a drain in flight at that moment is never joined afterwards
// (see the constraint note on ExecDrain).
class OffloadAudioRenderSink : public IAudioRenderSink,
    public std::enable_shared_from_this<OffloadAudioRenderSink> {
public:
    OffloadAudioRenderSink() = default;
    ~OffloadAudioRenderSink();

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
    int32_t GetVolumeDataCount(int64_t &volumeData) override;

    int32_t SetVolume(float left, float right) override;
    int32_t SetVolumeWithRamp(float left, float right, uint32_t durationMs) override;
    int32_t GetVolume(float &left, float &right) override;

    int32_t GetLatency(uint32_t &latency) override;
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude(void) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;
    void SetSpeed(float speed) override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t Drain(AudioDrainType type) override;
    void RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> callback) override;
    int32_t SetBufferSize(uint32_t sizeMs) override;
    int32_t SetOffloadRenderCallbackType(RenderCallbackType type) override;
    int32_t LockOffloadRunningLock(void) override;
    int32_t UnLockOffloadRunningLock(void) override;

    void DumpInfo(std::string &dumpString) override;

    int32_t UpdateActiveDevice(std::vector<DeviceType> &outputDevices) override;
    DeviceType GetCurrentOutputDevice() override;
    bool IsInA2dpOffload() override;
private:
    static uint32_t PcmFormatToBit(AudioSampleFormat format);
    static AudioFormat ConvertToHdiFormat(AudioSampleFormat format);
    static int32_t OffloadRenderCallback(struct IAudioCallback *self, enum AudioCallbackType type, int8_t *reserved,
        int8_t *cookie);
    void InitAudioSampleAttr(struct AudioSampleAttributes &param);
    void InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc);
    int32_t CreateRender(void);
    void InitLatencyMeasurement(void);
    void CheckLatencySignal(uint8_t *data, size_t len);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    void CheckUpdateState(char *data, uint64_t len);
    int32_t SetVolumeInner(float left, float right, uint32_t durationMs = 0);
    bool NeedToSetOffloadVolume(const float newVolume);
    void UpdateSinkState(bool started);
    // ---- control-plane actor: one resident thread owns all HDI control calls and the
    // flush state machine, replacing the old spawn-per-flush threads and the
    // isFlushing_/isNeedRestart_ cross-thread flags (see control-plane refactor design)
    enum class CtrlState : uint8_t { CTRL_IDLE, CTRL_STARTED, CTRL_FLUSHING };
    enum class CtrlCmd : uint8_t { CMD_START, CMD_STOP, CMD_FLUSH, CMD_RESET, CMD_DRAIN, CMD_SHUTDOWN };
    enum class PendingAfterFlush : uint8_t { PENDING_NONE, PENDING_START, PENDING_STOP };

    struct CtrlRequest {
        CtrlCmd cmd;
        int32_t arg = 0; // AudioDrainType for CMD_DRAIN
        std::shared_ptr<std::promise<int32_t>> done; // null = fire and forget
    };

    int32_t PostCtrlCmd(CtrlCmd cmd, int32_t arg, int32_t waitMs);
    void StartCtrlThread(); // requires a shared_ptr-owned instance (uses shared_from_this)
    void CtrlThreadMain();
    int32_t HandleCtrlCmd(CtrlCmd cmd, int32_t arg);
    int32_t ExecStart();
    int32_t ExecStop(bool flushFirst = true); // false = flush already done (pending-stop path)
    // publishStarted=false: the caller owns the terminal state, ExecStop keeps the
    // write gate closed until it publishes IDLE (or STARTED on legacy failure)
    void ExecFlush(bool restartAfter, bool publishStarted = true);
    int32_t ExecDrain(int32_t drainType);
    void ResolvePendingAfterFlush(bool restartAfter, bool publishStarted = true);

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
    static constexpr const char *RUNNING_LOCK_NAME = "AudioOffloadBackgroundPlay";
    static constexpr int32_t RUNNING_LOCK_TIMEOUTMS_LASTING = -1;
#endif
    static constexpr uint32_t AUDIO_SPEED_BASE = 1000;
    // caller wait budget for control replies; must stay well below the media side 8s write
    // timeout, otherwise a stuck reply makes media rebuild the stream on top of it
    static constexpr int32_t CTRL_WAIT_MS = 3000;
    static constexpr int32_t CTRL_SHUTDOWN_JOIN_MS = 2000;

    IAudioSinkAttr attr_ = {};
    struct OffloadHdiCallback hdiCallback_ = {};
    bool sinkInited_ = false;
    std::atomic<bool> started_ = false;
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;
    float offloadVolume_ = 0.0f;
    uint32_t durationMs_ = 0;
    std::chrono::milliseconds setVolumeTime_ = std::chrono::milliseconds(0);
    uint32_t hdiRenderId_ = HDI_INVALID_ID;
    struct IAudioRender *audioRender_ = nullptr;
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
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
    std::string logUtilsTag_ = "OffloadSink";
    mutable int64_t volumeDataCount_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock_;
    bool runningLocked_ = false;
#endif
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    std::atomic<uint64_t> renderPos_ = 0;

    DeviceType currentActiveDevice_ = DEVICE_TYPE_NONE;
    bool forceSetRouteFlag_ = false;
    std::thread ctrlThread_;
    std::mutex ctrlQueueMutex_; // leaf lock: guards the queue only, never across sinkMutex_
    std::condition_variable ctrlQueueCv_;
    std::queue<CtrlRequest> ctrlQueue_;
    std::atomic<CtrlState> ctrlState_ = CtrlState::CTRL_IDLE; // lock-free readers: RenderFrame etc.
    std::atomic<bool> ctrlRunning_ = false;
    std::atomic<bool> ctrlAbandoned_ = false;
    PendingAfterFlush pendingAfterFlush_ = PendingAfterFlush::PENDING_NONE; // ctrl thread only
};

} // namespace AudioStandard
} // namespace OHOS

#endif // OFFLOAD_AUDIO_RENDER_SINK_H
