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

#ifndef AUDIO_ENDPOINT_PRIVATE_H
#define AUDIO_ENDPOINT_PRIVATE_H

#include <sstream>
#include <memory>
#include <thread>

#include "i_process_status_listener.h"
#include "linear_pos_time_model.h"
#include "audio_device_descriptor.h"
#include "i_stream_manager.h"
#include "i_renderer_stream.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "audio_ring_cache.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

class MockCallbacks : public IStatusCallback, public IWriteCallback {
public:
    explicit MockCallbacks(uint32_t streamIndex);
    virtual ~MockCallbacks();
    void OnStatusUpdate(IOperation operation) override;
    int32_t OnWriteData(size_t length) override;
    int32_t OnWriteData(int8_t *inputData, size_t requestDataLen) override;
    int32_t GetAvailableSize(size_t &length) override;
    std::unique_ptr<AudioRingCache>& GetDupRingBuffer();
private:
    uint32_t streamIndex_ = 0;
    FILE *dumpDupOut_ = nullptr;
    std::string dumpDupOutFileName_ = "";
    std::unique_ptr<AudioRingCache> dupRingBuffer_ = nullptr;
};

class AudioEndpointInner : public AudioEndpoint {
public:
    AudioEndpointInner(EndpointType type, uint64_t id, const AudioProcessConfig &clientConfig);
    ~AudioEndpointInner();

    bool Config(const AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo) override;
    bool StartDevice(EndpointStatus preferredState = INVALID);
    void HandleStartDeviceFailed();
    bool StopDevice();

    // when audio process start.
    int32_t OnStart(IAudioProcessStream *processStream) override;
    // when audio process pause.
    int32_t OnPause(IAudioProcessStream *processStream) override;
    // when audio process request update handle info.
    int32_t OnUpdateHandleInfo(IAudioProcessStream *processStream) override;

    /**
     * Call LinkProcessStream when first create process or link other process with this endpoint.
     * Here are cases:
     *   case1: endpointStatus_ = UNLINKED, link not running process; UNLINKED-->IDEL & godown
     *   case2: endpointStatus_ = UNLINKED, link running process; UNLINKED-->IDEL & godown
     *   case3: endpointStatus_ = IDEL, link not running process; IDEL-->IDEL
     *   case4: endpointStatus_ = IDEL, link running process; IDEL-->STARTING-->RUNNING
     *   case5: endpointStatus_ = RUNNING; RUNNING-->RUNNING
    */
    int32_t LinkProcessStream(IAudioProcessStream *processStream, bool startWhenLinking = true) override;
    void AddEndpointStreamVolume(IAudioProcessStream *processStream);
    void LinkProcessStreamExt(IAudioProcessStream *processStream,
    const std::shared_ptr<OHAudioBufferBase>& processBuffer);

    int32_t UnlinkProcessStream(IAudioProcessStream *processStream) override;

    int32_t GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe) override;

    void Dump(std::string &dumpString) override;

    std::string GetEndpointName() override;
    EndpointType GetEndpointType() override
    {
        return endpointType_;
    }
    int32_t SetVolume(AudioStreamType streamType, float volume) override;

    std::shared_ptr<OHAudioBufferBase> GetBuffer() override
    {
        // not support
        return nullptr;
    }

    // for inner-cap
    bool ShouldInnerCap(int32_t innerCapId) override;
    int32_t EnableFastInnerCap(int32_t innerCapId) override;
    int32_t DisableFastInnerCap() override;
    int32_t DisableFastInnerCap(int32_t innerCapId) override;

    int32_t InitDupStream(int32_t innerCapId);

    EndpointStatus GetStatus() override;

    void Release() override;
    float GetMaxAmplitude() override;
    uint32_t GetLinkedProcessCount() override;

    AudioMode GetAudioMode() const final;

    void BindCore();
    
    void CheckWakeUpTime(int64_t &wakeUpTime);
private:
    AudioProcessConfig GetInnerCapConfig();
    void StartThread(const IAudioSinkAttr &attr);
    void CheckTimeAndBufferReady(uint64_t &curWritePos, int64_t &wakeUpTime, int64_t &curTime);
    void MixToDupStream(const std::vector<AudioStreamData> &srcDataList, int32_t innerCapId);
    bool ConfigInputPoint(const AudioDeviceDescriptor &deviceInfo);
    int32_t PrepareDeviceBuffer(const AudioDeviceDescriptor &deviceInfo);
    int32_t GetAdapterBufferInfo(const AudioDeviceDescriptor &deviceInfo);
    void ReSyncPosition();
    void RecordReSyncPosition();
    void InitAudiobuffer(bool resetReadWritePos);
    void ProcessData(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData);
    void ProcessSingleData(const AudioStreamData &srcData, const AudioStreamData &dstData, bool applyVol);
    void ResetZeroVolumeState();
    void HandleZeroVolumeStartEvent();
    void HandleZeroVolumeStopEvent();
    void HandleRendererDataParams(const AudioStreamData &srcData, const AudioStreamData &dstData, bool applyVol = true);
    int32_t HandleCapturerDataParams(RingBufferWrapper &writeBuf, const BufferDesc &readBuf,
        const BufferDesc &convertedBuffer);
    void ZeroVolumeCheck(const int32_t vol);
    int64_t GetPredictNextReadTime(uint64_t posInFrame);
    int64_t GetPredictNextWriteTime(uint64_t posInFrame);
    bool PrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime, const std::function<void()> &moveClientIndex);
    bool RecordPrepareNextLoop(uint64_t curReadPos, int64_t &wakeUpTime);
    int32_t InitDupBuffer(AudioProcessConfig processConfig, int32_t innerCapId, uint32_t dupStreamIndex);

    /**
     * @brief Get the current read position in frame and the read-time with it.
     *
     * @param frames the read position in frame
     * @param nanoTime the time in nanosecond when device-sink start read the buffer
    */
    bool GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime);

    void CheckStandBy();
    bool IsAnyProcessRunning();
    bool IsAnyProcessRunningInner();
    bool CheckAllBufferReady(int64_t checkTime, uint64_t curWritePos);
    void WaitAllProcessReady(uint64_t curWritePos);
    void CheckSyncInfo(uint64_t curWritePos);
    bool ProcessToEndpointDataHandle(uint64_t curWritePos, std::function<void()> &moveClientIndex);
    void ProcessToDupStream(const std::vector<AudioStreamData> &audioDataList, AudioStreamData &dstStreamData,
        int32_t innerCapId);

    struct VolumeResult {
        int32_t volumeStart;
        float volumeEnd;
        float volumeHap;
        bool muteFlag;
    };

    VolumeResult CalculateVolume(size_t i);
    void GetAllReadyProcessData(std::vector<AudioStreamData> &audioDataList, std::function<void()> &moveClientsIndex);
    void GetAllReadyProcessDataSub(size_t i,
        std::vector<AudioStreamData> &audioDataList, uint64_t curRead, std::function<void()> &moveClientIndex);
    std::string GetStatusStr(EndpointStatus status);

    bool IsNearlinkAbsVolSupportStream(DeviceType deviceType, AudioVolumeType volumeType);

    int32_t WriteToSpecialProcBuf(const std::shared_ptr<OHAudioBufferBase> &procBuf, const BufferDesc &readBuf,
        const BufferDesc &convertedBuffer, bool muteFlag);
    void WriteToProcessBuffers(const BufferDesc &readBuf);
    int32_t ReadFromEndpoint(uint64_t curReadPos);
    bool KeepWorkloopRunning();

    void EndpointWorkLoopFuc();
    void RecordEndpointWorkLoopFuc();

    // Call GetMmapHandlePosition in ipc may block more than a cycle, call it in another thread.
    void AsyncGetPosTime();
    bool DelayStopDevice();

    std::shared_ptr<IAudioRenderSink> GetFastSink(const AudioDeviceDescriptor &deviceInfo, EndpointType type);
    std::shared_ptr<IAudioCaptureSource> GetFastSource(const std::string &networkId, EndpointType type,
        IAudioSourceAttr &attr);
    void InitSinkAttr(IAudioSinkAttr &attr, const AudioDeviceDescriptor &deviceInfo);

    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckPlaySignal(uint8_t *buffer, size_t bufferSize);
    void CheckRecordSignal(uint8_t *buffer, size_t bufferSize);

    void CheckUpdateState(char *frame, uint64_t replyBytes);

    void ProcessUpdateAppsUidForPlayback();
    void ProcessUpdateAppsUidForRecord();

    int32_t HandleDisableFastCap(CaptureInfo &captureInfo);

    void WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize, int32_t index);
    bool IsInvalidBuffer(uint8_t *buffer, size_t bufferSize, AudioSampleFormat format);
    void ReportDataToResSched(std::unordered_map<std::string, std::string> payload, uint32_t type);
    void HandleMuteWriteData(BufferDesc &bufferDesc, int32_t index);
    int32_t CreateDupBufferInner(int32_t innerCapId);
    int32_t WriteDupBufferInner(const BufferDesc &bufferDesc, int32_t innerCapId);
    bool PrepareRingBuffer(size_t i, uint64_t curRead, RingBufferWrapper& ringBuffer);
    int32_t WriteToRingBuffer(RingBufferWrapper &writeBuf, const BufferDesc &buffer);
    void SetupMoveCallback(size_t i, uint64_t curRead, const RingBufferWrapper& ringBuffer,
        std::function<void()>& moveClientIndex);
    void AddProcessStreamToList(IAudioProcessStream *processStream,
        const std::shared_ptr<OHAudioBufferBase> &processBuffer);
    void CheckAudioHapticsSync(uint64_t curWritePos);
    bool IsBufferDataInsufficient(int32_t readableDataFrame, uint32_t spanSizeInFrame);
    bool NeedUseTempBuffer(const RingBufferWrapper &ringBuffer, size_t spanSizeInByte);
    void PrepareStreamDataBuffer(size_t i, size_t spanSizeInByte,
        RingBufferWrapper &ringBuffer, AudioStreamData &streamData);
private:
    static constexpr int64_t ONE_MILLISECOND_DURATION = 1000000; // 1ms
    static constexpr int64_t TWO_MILLISECOND_DURATION = 2000000; // 2ms
    static constexpr int64_t THREE_MILLISECOND_DURATION = 3000000; // 3ms
    static constexpr int64_t WRITE_TO_HDI_AHEAD_TIME = -1000000; // ahead 1ms
    static constexpr int32_t UPDATE_THREAD_TIMEOUT = 1000; // 1000ms
    static constexpr int32_t CPU_INDEX = 2;
    static constexpr int64_t MAX_WAKEUP_TIME_NS = 2000000000; // 2s
    static constexpr int64_t WAKEUPTIME_FOR_VOIP_MMAP_NS = 40000000; // 40ms
    static constexpr int64_t WAKEUPTIME_FOR_MMAP_NS = 10000000; // 10ms
    static constexpr int64_t RELATIVE_SLEEP_TIME_NS = 5000000; // 5ms
    
    enum ThreadStatus : uint32_t {
        WAITTING = 0,
        SLEEPING,
        INRUNNING
    };
    enum FastSinkType {
        NONE_FAST_SINK = 0,
        FAST_SINK_TYPE_NORMAL,
        FAST_SINK_TYPE_REMOTE,
        FAST_SINK_TYPE_VOIP,
        FAST_SINK_TYPE_BLUETOOTH
    };
    enum FastSourceType {
        NONE_FAST_SOURCE = 0,
        FAST_SOURCE_TYPE_NORMAL,
        FAST_SOURCE_TYPE_REMOTE,
        FAST_SOURCE_TYPE_VOIP
    };
    enum ZeroVolumeState : uint32_t {
        INACTIVE = 0,
        ACTIVE,
        IN_TIMING
    };
    
    EndpointType endpointType_;
    int32_t id_ = 0;
    std::mutex listLock_;
    std::vector<IAudioProcessStream *> processList_;
    std::vector<std::shared_ptr<OHAudioBufferBase>> processBufferList_;
    std::vector<std::vector<uint8_t>> processTmpBufferList_;
    AudioProcessConfig clientConfig_;

    std::atomic<bool> isInited_ = false;

    // for inner-cap
    std::mutex dupMutex_;
    size_t dupTotalSizeInFrame_ = 0;
    size_t dupSpanSizeInFrame_ = 0;
    size_t dupSpanSizeInByte_ = 0;
    size_t dupByteSizePerFrame_ = 0;
    FILE *dumpDupIn_ = nullptr;
    std::string dumpDupInFileName_ = "";
    std::map<int32_t, std::shared_ptr<MockCallbacks>> innerCapIdToDupStreamCallbackMap_;
    size_t dupBufferSize_ = 0;
    std::unique_ptr<uint8_t []> dupBuffer_ = nullptr;
    FILE *dumpC2SDup_ = nullptr; // client to server inner-cap dump file
    std::string dupDumpName_ = "";

    uint32_t fastRenderId_ = HDI_INVALID_ID;
    uint32_t fastCaptureId_ = HDI_INVALID_ID;
    FastSinkType fastSinkType_ = NONE_FAST_SINK;
    FastSourceType fastSourceType_ = NONE_FAST_SOURCE;

    LinearPosTimeModel readTimeModel_;
    LinearPosTimeModel writeTimeModel_;

    int64_t spanDuration_ = 0; // nano second
    int64_t serverAheadReadTime_ = 0;
    int dstBufferFd_ = -1; // -1: invalid fd.
    uint32_t dstTotalSizeInframe_ = 0;
    uint32_t dstSpanSizeInframe_ = 0;
    uint32_t dstByteSizePerFrame_ = 0;
    uint32_t syncInfoSize_ = 0;
    int64_t lastWriteTime_ = 0; // ns
    std::shared_ptr<OHAudioBuffer> dstAudioBuffer_ = nullptr;

    std::atomic<EndpointStatus> endpointStatus_ = INVALID;
    bool isStarted_ = false;
    int64_t delayStopTime_ = INT64_MAX;
    int64_t zeroVolumeStartTime_ = INT64_MAX;
    ZeroVolumeState zeroVolumeState_ = INACTIVE;

    std::atomic<ThreadStatus> threadStatus_ = WAITTING;
    std::thread endpointWorkThread_;
    std::mutex loopThreadLock_;
    std::condition_variable workThreadCV_;
    int64_t lastHandleProcessTime_ = 0;

    std::thread updatePosTimeThread_;
    std::mutex updateThreadLock_;
    std::condition_variable updateThreadCV_;
    std::atomic<bool> stopUpdateThread_ = false;

    std::atomic<uint64_t> posInFrame_ = 0;
    std::atomic<int64_t> timeInNano_ = 0;

    bool isDeviceRunningInIdel_ = true; // will call start sink when linked.
    bool needReSyncPosition_ = true;
    FILE *dumpHdi_ = nullptr;
    std::string dumpHdiName_ = "";
    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "";

    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;

    bool signalDetected_ = false;
    bool latencyMeasEnabled_ = false;
    size_t detectedTime_ = 0;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    std::unordered_map<int32_t, CaptureInfo> fastCaptureInfos_;
    bool coreBinded_ = false;
    bool isExistLoopback_ = false;
    int32_t audioHapticsSyncId_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ENDPOINT_H
