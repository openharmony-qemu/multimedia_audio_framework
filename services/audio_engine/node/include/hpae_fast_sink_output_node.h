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

#ifndef HPAE_FAST_SINK_OUTPUT_NODE_H
#define HPAE_FAST_SINK_OUTPUT_NODE_H

#include <functional>
#include <memory>
#include <sched.h>
#include <ctime>
#include <vector>
#include <thread>
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "oh_audio_buffer.h"
#include "linear_pos_time_model.h"

namespace OHOS {
namespace AudioStandard {
struct SignalDetectAgent;
namespace HPAE {

class HpaeFastSinkOutputNode : public InputNode<HpaePcmBuffer*> {
public:
    HpaeFastSinkOutputNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeFastSinkOutputNode();

    // HpaeNode overrides
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;

    // Node chain connection
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode) override;

    // HDI sink lifecycle
    int32_t GetRenderSinkInstance(const std::string &deviceClass, const std::string &deviceNetworkId);
    int32_t RenderSinkInit(IAudioSinkAttr &attr);
    int32_t RenderSinkStart();
    int32_t RenderSinkStop();
    int32_t RenderSinkDeInit();
    int32_t RenderSinkFlush(void);
    const char* GetRenderFrameData(void);

    // State query
    StreamManagerState GetSinkState() const;
    int32_t SetSinkState(StreamManagerState sinkState);
    size_t GetPreOutNum();

    // Configuration
    int32_t SetTimeoutStopThd(int64_t timeoutMs);
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid);
    int32_t RefreshSpanSize(bool &isUpdated);
    uint32_t GetSpanSizeInFrame() const;
    void SetNeedCheckZeroVolume(bool needCheckZeroVolume);
    void SetMuteForSwitchDevice(bool mute);

    // Latency query
    uint64_t GetLatency();
    float GetMaxAmplitude() const;

    // Stream change notification
    void NotifyStreamChangeToSink(StreamChangeType change,
        uint32_t sessionId, StreamUsage usage, RendererState state, uint32_t appUid = INVALID_UID);

    // Loopback state
    void SetLoopbackState(bool isExistLoopback);

    // Client wake callback - invoked between PrepareNextLoop and AbsoluteSleep
    void SetClientWakeCallback(std::function<void()> callback) { clientWakeCallback_ = std::move(callback); }

private:
    static constexpr int64_t ONE_MILLISECOND_DURATION_NS = 1000000;
    static constexpr int64_t THREE_MILLISECOND_DURATION_NS = 3000000;
    static constexpr int64_t RELATIVE_SLEEP_TIME_NS = 5000000;
    static constexpr int64_t MAX_WAKEUP_TIME_NS = 2000000000;
    static constexpr int64_t DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS = 4000000000;
    static constexpr uint32_t FRAME_LEN_MS_DEFAULT = 20;
    static constexpr uint32_t TIME_OUT_STOP_THD_DEFAULT_FRAME = 150;
    static constexpr int32_t CPU_INDEX = 2;
    static constexpr int32_t ONE_MINUTE = 60;
    static constexpr size_t SIMD_ALIGNMENT_BYTES = 16;

    enum ZeroVolumeState : uint32_t {
        INACTIVE = 0,
        ACTIVE,
        IN_TIMING
    };

    int32_t PrepareDeviceBuffer();
    int32_t GetAdapterBufferInfo();
    void InitTransBuffer();
    void InitAudiobuffer(bool resetReadWritePos);
    void AsyncGetPosTime();
    void StopUpdateThread();
    bool TriggerPrepareNextLoop();
    bool TriggerPrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime);
    bool IsCheckingSuspend() const;
    bool CheckIfSuspend();
    void ReSyncPosition();
    bool GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime);
    bool CheckAllBufferReady(int64_t checkTime, uint64_t curWritePos);
    void CheckTimeAndBufferReady(uint64_t &curWritePos, int64_t &wakeUpTime, int64_t &curTime);
    bool PrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime);
    int64_t GetPredictNextReadTime(uint64_t posInFrame);
    void CheckWakeUpTime(int64_t &wakeUpTime);
    void CheckJank(uint64_t curWritePos);
    void CheckSyncInfo(uint64_t curWritePos);
    bool GetRenderFrameDataInner(HpaePcmBuffer *&pcmBuffer);
    int32_t WriteToDeviceBuffer(HpaePcmBuffer *pcmBuffer, uint64_t curWritePos);
    AudioStreamInfo GetDfxStreamInfo();
    void InitDumpFile();
    void InitSinkVolume();
    void SyncCurrentOutputDevice();
    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckPlaySignal(uint8_t *buffer, size_t bufferSize);
    bool IsInvalidBuffer(uint8_t *buffer, size_t bufferSize) const;
    void UpdateSilentState(uint8_t *buffer, size_t bufferSize);
    int64_t CalcServerAheadReadTime(uint32_t sampleRate);
    void UpdateAmplitudeIfNeeded(BufferDesc &writeBuf);
    void ZeroVolumeCheck(int32_t vol);
    void HandleZeroVolumeStartEvent();
    void HandleZeroVolumeStopEvent();
    void ResetZeroVolumeState();

    InputPort<HpaePcmBuffer*> inputStream_;
    std::shared_ptr<IAudioRenderSink> audioRendererSink_ = nullptr;
    uint32_t renderId_ = HDI_INVALID_ID;
    IAudioSinkAttr sinkOutAttr_;
    StreamManagerState state_ = STREAM_MANAGER_NEW;
    LinearPosTimeModel readTimeModel_;
    LinearPosTimeModel writeTimeModel_;
    int64_t spanDuration_ = 0;
    int64_t serverAheadReadTime_ = 0;

    // mmap shared memory buffer (migrated from AudioEndpointInner)
    std::shared_ptr<OHAudioBuffer> dstAudioBuffer_ = nullptr;
    int dstBufferFd_ = -1;
    uint32_t dstTotalSizeInframe_ = 0;
    uint32_t dstSpanSizeInframe_ = 0;
    uint32_t dstByteSizePerFrame_ = 0;
    uint32_t syncInfoSize_ = 0;

    // Single span size in bytes = dstSpanSizeInframe_ * dstByteSizePerFrame_
    size_t dstSpanSizeInByte_ = 0;

    // Current write position in frames (advances per cycle)
    uint64_t curWritePos_ = 0;

    uint32_t frameLenMs_ = FRAME_LEN_MS_DEFAULT;
    uint32_t timeoutThdFrames_ = TIME_OUT_STOP_THD_DEFAULT_FRAME;
    uint32_t timeoutStopCount_ = 0;
    AdapterType adapterType_ = ADAPTER_TYPE_FAST;
    DeviceType currentOutputDevice_ = DEVICE_TYPE_INVALID;
    int64_t lastWriteTime_ = 0;
    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "";
    FILE *dumpHdi_ = nullptr;
    std::string dumpHdiName_ = "";
    std::vector<char, AlignedAllocator<char, SIMD_ALIGNMENT_BYTES>> renderFrameData_;
    size_t validRenderDataLen_ = 0;

    // Position update thread
    std::thread updatePosTimeThread_;
    std::mutex updateThreadLock_;
    std::condition_variable updateThreadCV_;
    std::atomic<bool> stopUpdateThread_ = false;
    bool isCheckingSuspend_ = false;
    bool needReSyncPosition_ = true;
    bool isStarted_ = false;
    bool needCheckZeroVolume_ = true;
    bool isUltraFast_ = false;
    int64_t zeroVolumeStartTime_ = INT64_MAX;
    ZeroVolumeState zeroVolumeState_ = INACTIVE;
    bool latencyMeasEnabled_ = false;
    bool signalDetected_ = false;
    size_t detectedTime_ = 0;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
    std::time_t startMuteTime_ = 0;
    bool isInSilentState_ = false;
    bool isExistLoopback_ = false;

    // Position tracking
    std::atomic<uint64_t> posInFrame_ = 0;
    std::atomic<int64_t> timeInNano_ = 0;
    int64_t lastPredictWakeUpTime_ = 0;
    bool switchDevicesMute_ = false;

    // Client wake callback set by manager
    std::function<void()> clientWakeCallback_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif // HPAE_FAST_SINK_OUTPUT_NODE_H
