/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioEndpointInner"
#endif

#include "audio_endpoint.h"

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <thread>
#include <vector>
#include <mutex>

#include "securec.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "bluetooth_renderer_sink.h"
#include "fast_audio_renderer_sink.h"
#include "fast_audio_capturer_source.h"
#include "format_converter.h"
#include "i_audio_capturer_source.h"
#include "i_stream_manager.h"
#include "linear_pos_time_model.h"
#include "policy_handler.h"
#include "audio_log_utils.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"
#ifdef DAUDIO_ENABLE
#include "remote_fast_audio_renderer_sink.h"
#include "remote_fast_audio_capturer_source.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
    static constexpr int64_t RECORD_DELAY_TIME_NS = 4000000; // 4ms = 4 * 1000 * 1000ns
    static constexpr int64_t RECORD_VOIP_DELAY_TIME_NS = 20000000; // 20ms = 20 * 1000 * 1000ns
    static constexpr int64_t MAX_SPAN_DURATION_NS = 100000000; // 100ms = 100 * 1000 * 1000ns
    static constexpr int64_t DELAY_STOP_HDI_TIME = 10000000000; // 10s
    static constexpr int64_t WAIT_CLIENT_STANDBY_TIME_NS = 1000000000; // 1s = 1000 * 1000 * 1000ns
    static constexpr int64_t DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS = 4000000000; // 4s = 4 * 1000 * 1000 * 1000ns
    static constexpr int64_t DELAY_STOP_HDI_TIME_WHEN_NO_RUNNING_NS = 1000000000; // 1s
    static constexpr int32_t SLEEP_TIME_IN_DEFAULT = 400; // 400ms
    static constexpr int64_t DELTA_TO_REAL_READ_START_TIME = 0; // 0ms
    const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 40;
    static const int32_t HALF_FACTOR = 2;
}

static enum HdiAdapterFormat ConvertToHdiAdapterFormat(AudioSampleFormat format)
{
    enum HdiAdapterFormat adapterFormat;
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
            adapterFormat = HdiAdapterFormat::SAMPLE_U8;
            break;
        case AudioSampleFormat::SAMPLE_S16LE:
            adapterFormat = HdiAdapterFormat::SAMPLE_S16;
            break;
        case AudioSampleFormat::SAMPLE_S24LE:
            adapterFormat = HdiAdapterFormat::SAMPLE_S24;
            break;
        case AudioSampleFormat::SAMPLE_S32LE:
            adapterFormat = HdiAdapterFormat::SAMPLE_S32;
            break;
        default:
            adapterFormat = HdiAdapterFormat::INVALID_WIDTH;
            break;
    }

    return adapterFormat;
}

class MockCallbacks : public IStatusCallback, public IWriteCallback {
public:
    explicit MockCallbacks(uint32_t streamIndex);
    virtual ~MockCallbacks() = default;
    void OnStatusUpdate(IOperation operation) override;
    int32_t OnWriteData(size_t length) override;
private:
    uint32_t streamIndex_ = 0;
};

class AudioEndpointInner : public AudioEndpoint {
public:
    AudioEndpointInner(EndpointType type, uint64_t id, const AudioProcessConfig &clientConfig);
    ~AudioEndpointInner();

    bool Config(const AudioDeviceDescriptor &deviceInfo) override;
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
    int32_t LinkProcessStream(IAudioProcessStream *processStream) override;
    void LinkProcessStreamExt(IAudioProcessStream *processStream,
    const std::shared_ptr<OHAudioBuffer>& processBuffer);

    int32_t UnlinkProcessStream(IAudioProcessStream *processStream) override;

    int32_t GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe) override;

    void Dump(std::string &dumpString) override;

    std::string GetEndpointName() override;
    EndpointType GetEndpointType() override
    {
        return endpointType_;
    }
    int32_t SetVolume(AudioStreamType streamType, float volume) override;

    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) override;

    std::shared_ptr<OHAudioBuffer> GetBuffer() override
    {
        return dstAudioBuffer_;
    }

    // for inner-cap
    bool ShouldInnerCap() override;
    int32_t EnableFastInnerCap() override;
    int32_t DisableFastInnerCap() override;

    int32_t InitDupStream();

    EndpointStatus GetStatus() override;

    void Release() override;

    AudioDeviceDescriptor &GetDeviceInfo() override
    {
        return deviceInfo_;
    }

    DeviceRole GetDeviceRole() override
    {
        return deviceInfo_.deviceRole_;
    }

    float GetMaxAmplitude() override;
    uint32_t GetLinkedProcessCount() override;
    void BindCore();

private:
    AudioProcessConfig GetInnerCapConfig();
    void StartThread(const IAudioSinkAttr &attr);
    void MixToDupStream(const std::vector<AudioStreamData> &srcDataList);
    bool ConfigInputPoint(const AudioDeviceDescriptor &deviceInfo);
    int32_t PrepareDeviceBuffer(const AudioDeviceDescriptor &deviceInfo);
    int32_t GetAdapterBufferInfo(const AudioDeviceDescriptor &deviceInfo);
    void ReSyncPosition();
    void RecordReSyncPosition();
    void InitAudiobuffer(bool resetReadWritePos);
    void ProcessData(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData);
    void ProcessSingleData(const AudioStreamData &srcData, const AudioStreamData &dstData);
    void HandleZeroVolumeCheckEvent();
    void HandleRendererDataParams(const AudioStreamData &srcData, const AudioStreamData &dstData);
    int32_t HandleCapturerDataParams(const BufferDesc &writeBuf, const BufferDesc &readBuf,
        const BufferDesc &convertedBuffer);
    void ZeroVolumeCheck(const int32_t vol);
    int64_t GetPredictNextReadTime(uint64_t posInFrame);
    int64_t GetPredictNextWriteTime(uint64_t posInFrame);
    bool PrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime);
    bool RecordPrepareNextLoop(uint64_t curReadPos, int64_t &wakeUpTime);

    /**
     * @brief Get the current read position in frame and the read-time with it.
     *
     * @param frames the read position in frame
     * @param nanoTime the time in nanosecond when device-sink start read the buffer
    */
    bool GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime);
    int32_t GetProcLastWriteDoneInfo(const std::shared_ptr<OHAudioBuffer> processBuffer, uint64_t curWriteFrame,
        uint64_t &proHandleFrame, int64_t &proHandleTime);

    void CheckStandBy();
    bool IsAnyProcessRunning();
    bool IsAnyProcessRunningInner();
    bool CheckAllBufferReady(int64_t checkTime, uint64_t curWritePos);
    void WaitAllProcessReady(uint64_t curWritePos);
    bool ProcessToEndpointDataHandle(uint64_t curWritePos);
    void GetAllReadyProcessData(std::vector<AudioStreamData> &audioDataList);

    std::string GetStatusStr(EndpointStatus status);

    int32_t WriteToSpecialProcBuf(const std::shared_ptr<OHAudioBuffer> &procBuf, const BufferDesc &readBuf,
        const BufferDesc &convertedBuffer, bool muteFlag);
    void WriteToProcessBuffers(const BufferDesc &readBuf);
    int32_t ReadFromEndpoint(uint64_t curReadPos);
    bool KeepWorkloopRunning();

    void EndpointWorkLoopFuc();
    void RecordEndpointWorkLoopFuc();

    // Call GetMmapHandlePosition in ipc may block more than a cycle, call it in another thread.
    void AsyncGetPosTime();
    bool DelayStopDevice();

    IMmapAudioRendererSink *GetFastSink(const AudioDeviceDescriptor &deviceInfo, EndpointType type);
    IMmapAudioCapturerSource *GetFastSource(const std::string &networkId, EndpointType type, IAudioSourceAttr &attr);

    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckPlaySignal(uint8_t *buffer, size_t bufferSize);
    void CheckRecordSignal(uint8_t *buffer, size_t bufferSize);
    void DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel) const;

    void CheckUpdateState(char *frame, uint64_t replyBytes);

    void ProcessUpdateAppsUidForPlayback();
    void ProcessUpdateAppsUidForRecord();

    void WriterRenderStreamStandbySysEvent(uint32_t sessionId, int32_t standby);
private:
    static constexpr int64_t ONE_MILLISECOND_DURATION = 1000000; // 1ms
    static constexpr int64_t THREE_MILLISECOND_DURATION = 3000000; // 3ms
    static constexpr int64_t WRITE_TO_HDI_AHEAD_TIME = -1000000; // ahead 1ms
    static constexpr int32_t UPDATE_THREAD_TIMEOUT = 1000; // 1000ms
    static constexpr int32_t CPU_INDEX = 2;
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
    // SamplingRate EncodingType SampleFormat Channel
    AudioDeviceDescriptor deviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo dstStreamInfo_;
    EndpointType endpointType_;
    int32_t id_ = 0;
    std::mutex listLock_;
    std::vector<IAudioProcessStream *> processList_;
    std::vector<std::shared_ptr<OHAudioBuffer>> processBufferList_;
    AudioProcessConfig clientConfig_;

    std::atomic<bool> isInited_ = false;

    // for inner-cap
    std::mutex dupMutex_;
    std::atomic<bool> isInnerCapEnabled_ = false;
    uint32_t dupStreamIndex_ = 0;
    std::shared_ptr<MockCallbacks> dupStreamCallback_ = nullptr;
    std::shared_ptr<IRendererStream> dupStream_ = nullptr;
    size_t dupBufferSize_ = 0;
    std::unique_ptr<uint8_t []> dupBuffer_ = nullptr;
    FILE *dumpC2SDup_ = nullptr; // client to server inner-cap dump file
    std::string dupDumpName_ = "";

    IMmapAudioRendererSink *fastSink_ = nullptr;
    IMmapAudioCapturerSource *fastSource_ = nullptr;
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
    std::shared_ptr<OHAudioBuffer> dstAudioBuffer_ = nullptr;

    std::atomic<EndpointStatus> endpointStatus_ = INVALID;
    bool isStarted_ = false;
    int64_t delayStopTime_ = INT64_MAX;
    int64_t delayStopTimeForZeroVolume_ = INT64_MAX;

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
    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "";
    std::string dumpHdiName_ = "";

    bool signalDetected_ = false;
    bool latencyMeasEnabled_ = false;
    size_t detectedTime_ = 0;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;

    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;

    bool zeroVolumeStopDevice_ = false;
    bool isVolumeAlreadyZero_ = false;
    bool coreBinded_ = false;
};

std::string AudioEndpoint::GenerateEndpointKey(AudioDeviceDescriptor &deviceInfo, int32_t endpointFlag)
{
    // All primary sinks share one endpoint
    int32_t endpointId = 0;
    if (deviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        endpointId = deviceInfo.deviceId_;
    }
    return deviceInfo.networkId_ + "_" + std::to_string(endpointId) + "_" +
        std::to_string(deviceInfo.deviceRole_) + "_" + std::to_string(endpointFlag);
}

std::shared_ptr<AudioEndpoint> AudioEndpoint::CreateEndpoint(EndpointType type, uint64_t id,
    const AudioProcessConfig &clientConfig, const AudioDeviceDescriptor &deviceInfo)
{
    std::shared_ptr<AudioEndpoint> audioEndpoint = nullptr;
    if (type == EndpointType::TYPE_INDEPENDENT && deviceInfo.deviceRole_ != INPUT_DEVICE &&
         deviceInfo.networkId_ == LOCAL_NETWORK_ID) {
        audioEndpoint = std::make_shared<AudioEndpointSeparate>(type, id, clientConfig.streamType);
    } else {
        audioEndpoint = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    }
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "Create AudioEndpoint failed.");

    if (!audioEndpoint->Config(deviceInfo)) {
        AUDIO_ERR_LOG("Config AudioEndpoint failed.");
        audioEndpoint = nullptr;
    }
    return audioEndpoint;
}

AudioEndpointInner::AudioEndpointInner(EndpointType type, uint64_t id,
    const AudioProcessConfig &clientConfig) : endpointType_(type), id_(id), clientConfig_(clientConfig)
{
    AUDIO_INFO_LOG("AudioEndpoint type:%{public}d", endpointType_);
    if (clientConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        logUtilsTag_ = "AudioEndpoint::Play";
    } else {
        logUtilsTag_ = "AudioEndpoint::Rec";
    }
}

std::string AudioEndpointInner::GetEndpointName()
{
    return GenerateEndpointKey(deviceInfo_, id_);
}

int32_t AudioEndpointInner::SetVolume(AudioStreamType streamType, float volume)
{
    // No need set hdi volume in shared stream mode.
    return SUCCESS;
}

int32_t AudioEndpointInner::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    return SUCCESS;
}

MockCallbacks::MockCallbacks(uint32_t streamIndex) : streamIndex_(streamIndex)
{
    AUDIO_INFO_LOG("DupStream %{public}u create MockCallbacks", streamIndex_);
}

void MockCallbacks::OnStatusUpdate(IOperation operation)
{
    AUDIO_INFO_LOG("DupStream %{public}u recv operation: %{public}d", streamIndex_, operation);
}

int32_t MockCallbacks::OnWriteData(size_t length)
{
    Trace trace("DupStream::OnWriteData length " + std::to_string(length));
    return SUCCESS;
}

bool AudioEndpointInner::ShouldInnerCap()
{
    bool shouldBecapped = false;
    std::lock_guard<std::mutex> lock(listLock_);
    for (uint32_t i = 0; i < processList_.size(); i++) {
        if (processList_[i]->GetInnerCapState()) {
            shouldBecapped = true;
            break;
        }
    }
    AUDIO_INFO_LOG("find endpoint inner-cap state: %{public}s", shouldBecapped ? "true" : "false");
    return shouldBecapped;
}

AudioProcessConfig AudioEndpointInner::GetInnerCapConfig()
{
    AudioProcessConfig processConfig;

    processConfig.appInfo.appPid = static_cast<int32_t>(getpid());
    processConfig.appInfo.appUid = static_cast<int32_t>(getuid());

    processConfig.streamInfo = dstStreamInfo_;

    processConfig.audioMode = AUDIO_MODE_PLAYBACK;

    // processConfig.rendererInfo ?

    processConfig.streamType = STREAM_MUSIC;

    return processConfig;
}

int32_t AudioEndpointInner::InitDupStream()
{
    std::lock_guard<std::mutex> lock(dupMutex_);
    CHECK_AND_RETURN_RET_LOG(isInnerCapEnabled_ == false, SUCCESS, "already enabled");

    AudioProcessConfig processConfig = GetInnerCapConfig();
    int32_t ret = IStreamManager::GetDupPlaybackManager().CreateRender(processConfig, dupStream_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && dupStream_ != nullptr, ERR_OPERATION_FAILED, "Failed: %{public}d", ret);
    dupStreamIndex_ = dupStream_->GetStreamIndex();

    dupStreamCallback_ = std::make_shared<MockCallbacks>(dupStreamIndex_);
    dupStream_->RegisterStatusCallback(dupStreamCallback_);
    dupStream_->RegisterWriteCallback(dupStreamCallback_);

    // eg: /data/local/tmp/LocalDevice6_0_c2s_dup_48000_2_1.pcm
    AudioStreamInfo tempInfo = processConfig.streamInfo;
    dupDumpName_ = GetEndpointName() + "_c2s_dup_" + std::to_string(tempInfo.samplingRate) + "_" +
        std::to_string(tempInfo.channels) + "_" + std::to_string(tempInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dupDumpName_, &dumpC2SDup_);

    AUDIO_INFO_LOG("Dup Renderer %{public}d with Endpoint status: %{public}s", dupStreamIndex_,
        GetStatusStr(endpointStatus_).c_str());

    // buffer init
    dupBufferSize_ = dstSpanSizeInframe_ * dstByteSizePerFrame_; // each
    CHECK_AND_RETURN_RET_LOG(dupBufferSize_ < dstAudioBuffer_->GetDataSize(), ERR_OPERATION_FAILED, "Init buffer fail");
    dupBuffer_ = std::make_unique<uint8_t []>(dupBufferSize_);
    ret = memset_s(reinterpret_cast<void *>(dupBuffer_.get()), dupBufferSize_, 0, dupBufferSize_);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret %{public}d", ret);
    }

    if (endpointStatus_ == RUNNING || (endpointStatus_ == IDEL && isDeviceRunningInIdel_)) {
        int32_t audioId = deviceInfo_.deviceId_;
        AUDIO_INFO_LOG("Endpoint %{public}d is already running, let's start the dup stream", audioId);
        dupStream_->Start();
    }
    // mark enabled last
    isInnerCapEnabled_ = true;
    return SUCCESS;
}

int32_t AudioEndpointInner::EnableFastInnerCap()
{
    if (isInnerCapEnabled_) {
        AUDIO_INFO_LOG("InnerCap is already enabled");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(deviceInfo_.deviceRole_ == OUTPUT_DEVICE, ERR_INVALID_OPERATION, "Not output device!");
    int32_t ret = InitDupStream();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Init dup stream failed");
    return SUCCESS;
}

int32_t AudioEndpointInner::DisableFastInnerCap()
{
    if (deviceInfo_.deviceRole_ != OUTPUT_DEVICE) {
        return SUCCESS;
    }
    std::lock_guard<std::mutex> lock(dupMutex_);
    if (!isInnerCapEnabled_) {
        AUDIO_INFO_LOG("InnerCap is already disabled.");
        return SUCCESS;
    }
    isInnerCapEnabled_ = false;
    AUDIO_INFO_LOG("Disable dup renderer %{public}d with Endpoint status: %{public}s", dupStreamIndex_,
        GetStatusStr(endpointStatus_).c_str());

    IStreamManager::GetDupPlaybackManager().ReleaseRender(dupStreamIndex_);
    dupStream_ = nullptr;

    return SUCCESS;
}

AudioEndpoint::EndpointStatus AudioEndpointInner::GetStatus()
{
    AUDIO_INFO_LOG("AudioEndpoint get status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    return endpointStatus_.load();
}

void AudioEndpointInner::Release()
{
    // Wait for thread end and then clear other data to avoid using any cleared data in thread.
    AUDIO_INFO_LOG("Release enter.");
    if (!isInited_.load()) {
        AUDIO_WARNING_LOG("already released");
        return;
    }

    isInited_.store(false);
    workThreadCV_.notify_all();
    if (endpointWorkThread_.joinable()) {
        AUDIO_DEBUG_LOG("AudioEndpoint join work thread start");
        endpointWorkThread_.join();
        AUDIO_DEBUG_LOG("AudioEndpoint join work thread end");
    }

    stopUpdateThread_.store(true);
    updateThreadCV_.notify_all();
    if (updatePosTimeThread_.joinable()) {
        AUDIO_DEBUG_LOG("AudioEndpoint join update thread start");
        updatePosTimeThread_.join();
        AUDIO_DEBUG_LOG("AudioEndpoint join update thread end");
    }

    if (fastSink_ != nullptr) {
        fastSink_->DeInit();
        fastSink_ = nullptr;
    }

    if (fastSource_ != nullptr) {
        fastSource_->DeInit();
        fastSource_ = nullptr;
    }

    endpointStatus_.store(INVALID);

    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("Set device buffer null");
        dstAudioBuffer_ = nullptr;
    }

    if (deviceInfo_.deviceRole_ == OUTPUT_DEVICE && isInnerCapEnabled_) {
        DisableFastInnerCap();
    }

    DumpFileUtil::CloseDumpFile(&dumpHdi_);
}

AudioEndpointInner::~AudioEndpointInner()
{
    if (isInited_.load()) {
        AudioEndpointInner::Release();
    }
    AUDIO_INFO_LOG("~AudioEndpoint()");
}

void AudioEndpointInner::Dump(std::string &dumpString)
{
    // dump endpoint stream info
    dumpString += "Endpoint stream info:\n";
    AppendFormat(dumpString, "  - samplingRate: %d\n", dstStreamInfo_.samplingRate);
    AppendFormat(dumpString, "  - channels: %u\n", dstStreamInfo_.channels);
    AppendFormat(dumpString, "  - format: %u\n", dstStreamInfo_.format);
    AppendFormat(dumpString, "  - sink type: %d\n", fastSinkType_);
    AppendFormat(dumpString, "  - source type: %d\n", fastSourceType_);

    // dump status info
    AppendFormat(dumpString, "  - Current endpoint status: %s\n", GetStatusStr(endpointStatus_).c_str());
    if (dstAudioBuffer_ != nullptr) {
        AppendFormat(dumpString, "  - Currend hdi read position: %u\n", dstAudioBuffer_->GetCurReadFrame());
        AppendFormat(dumpString, "  - Currend hdi write position: %u\n", dstAudioBuffer_->GetCurWriteFrame());
    }

    // dump linked process info
    std::lock_guard<std::mutex> lock(listLock_);
    AppendFormat(dumpString, "  - linked process:: %zu\n", processBufferList_.size());
    for (auto item : processBufferList_) {
        AppendFormat(dumpString, "  - process read position: %u\n", item->GetCurReadFrame());
        AppendFormat(dumpString, "  - process write position: %u\n", item->GetCurWriteFrame());
    }
    dumpString += "\n";
}

bool AudioEndpointInner::ConfigInputPoint(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("ConfigInputPoint enter.");
    IAudioSourceAttr attr = {};
    attr.sampleRate = dstStreamInfo_.samplingRate;
    attr.channel = dstStreamInfo_.channels;
    attr.format = ConvertToHdiAdapterFormat(dstStreamInfo_.format);
    attr.deviceNetworkId = deviceInfo.networkId_.c_str();
    attr.deviceType = deviceInfo.deviceType_;
    attr.audioStreamFlag = endpointType_ == TYPE_VOIP_MMAP ? AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;
    attr.sourceType = endpointType_ == TYPE_VOIP_MMAP ? SOURCE_TYPE_VOICE_COMMUNICATION : SOURCE_TYPE_MIC;

    fastSource_ = GetFastSource(deviceInfo.networkId_, endpointType_, attr);

    if (deviceInfo.networkId_ == LOCAL_NETWORK_ID) {
        attr.adapterName = "primary";
        fastSource_ = FastAudioCapturerSource::GetInstance();
    } else {
#ifdef DAUDIO_ENABLE
        attr.adapterName = "remote";
        fastSource_ = RemoteFastAudioCapturerSource::GetInstance(deviceInfo.networkId_);
#endif
    }
    CHECK_AND_RETURN_RET_LOG(fastSource_ != nullptr, false, "ConfigInputPoint GetInstance failed.");

    int32_t err = fastSource_->Init(attr);
    if (err != SUCCESS || !fastSource_->IsInited()) {
        AUDIO_ERR_LOG("init remote fast fail, err %{public}d.", err);
        fastSource_ = nullptr;
        return false;
    }
    if (PrepareDeviceBuffer(deviceInfo) != SUCCESS) {
        fastSource_->DeInit();
        fastSource_ = nullptr;
        return false;
    }

    bool ret = writeTimeModel_.ConfigSampleRate(dstStreamInfo_.samplingRate);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "Config LinearPosTimeModel failed.");

    endpointStatus_ = UNLINKED;
    isInited_.store(true);
    endpointWorkThread_ = std::thread([this] { this->RecordEndpointWorkLoopFuc(); });
    pthread_setname_np(endpointWorkThread_.native_handle(), "OS_AudioEpLoop");

    updatePosTimeThread_ = std::thread([this] { this->AsyncGetPosTime(); });
    pthread_setname_np(updatePosTimeThread_.native_handle(), "OS_AudioEpUpdate");

    // eg: input_endpoint_hdi_audio_8_0_20240527202236189_48000_2_1.pcm
    dumpHdiName_ = "input_endpoint_hdi_audio_" + std::to_string(attr.deviceType) + '_' +
        std::to_string(endpointType_) + '_' + GetTime() +
        '_' + std::to_string(attr.sampleRate) + "_" +
        std::to_string(attr.channel) + "_" + std::to_string(attr.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpHdiName_, &dumpHdi_);
    return true;
}

IMmapAudioCapturerSource *AudioEndpointInner::GetFastSource(const std::string &networkId, EndpointType type,
    IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("Network id %{public}s, endpoint type %{public}d", networkId.c_str(), type);
#ifdef DAUDIO_ENABLE
    if (networkId != LOCAL_NETWORK_ID) {
        attr.adapterName = "remote";
        fastSourceType_ = type == AudioEndpoint::TYPE_MMAP ? FAST_SOURCE_TYPE_REMOTE : FAST_SOURCE_TYPE_VOIP;
        // Distributed only requires a singleton because there won't be both voip and regular fast simultaneously
        return RemoteFastAudioCapturerSource::GetInstance(networkId);
    }
#endif

    attr.adapterName = "primary";
    if (type == AudioEndpoint::TYPE_MMAP) {
        fastSourceType_ = FAST_SOURCE_TYPE_NORMAL;
        return FastAudioCapturerSource::GetInstance();
    } else if (type == AudioEndpoint::TYPE_VOIP_MMAP) {
        fastSourceType_ = FAST_SOURCE_TYPE_VOIP;
        return FastAudioCapturerSource::GetVoipInstance();
    }
    return nullptr;
}

void AudioEndpointInner::StartThread(const IAudioSinkAttr &attr)
{
    endpointStatus_ = UNLINKED;
    isInited_.store(true);
    endpointWorkThread_ = std::thread([this] { this->EndpointWorkLoopFuc(); });
    pthread_setname_np(endpointWorkThread_.native_handle(), "OS_AudioEpLoop");

    updatePosTimeThread_ = std::thread([this] { this->AsyncGetPosTime(); });
    pthread_setname_np(updatePosTimeThread_.native_handle(), "OS_AudioEpUpdate");

    // eg: endpoint_hdi_audio_8_0_20240527202236189_48000_2_1.pcm
    dumpHdiName_ = "endpoint_hdi_audio_" + std::to_string(attr.deviceType) + '_' + std::to_string(endpointType_) +
        '_' + GetTime() + '_' +
        std::to_string(attr.sampleRate) + "_" +
        std::to_string(attr.channel) + "_" + std::to_string(attr.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpHdiName_, &dumpHdi_);
}

bool AudioEndpointInner::Config(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("Config enter, deviceRole %{public}d.", deviceInfo.deviceRole_);
    deviceInfo_ = deviceInfo;
    bool res = deviceInfo_.audioStreamInfo_.CheckParams();
    CHECK_AND_RETURN_RET_LOG(res, false, "samplingRate or channels size is 0");

    dstStreamInfo_ = {
        *deviceInfo.audioStreamInfo_.samplingRate.rbegin(),
        deviceInfo.audioStreamInfo_.encoding,
        deviceInfo.audioStreamInfo_.format,
        *deviceInfo.audioStreamInfo_.channels.rbegin()
    };
    dstStreamInfo_.channelLayout = deviceInfo.audioStreamInfo_.channelLayout;

    if (deviceInfo.deviceRole_ == INPUT_DEVICE) {
        return ConfigInputPoint(deviceInfo);
    }

    fastSink_ = GetFastSink(deviceInfo, endpointType_);
    CHECK_AND_RETURN_RET_LOG(fastSink_ != nullptr, false, "Get fastSink instance failed");

    IAudioSinkAttr attr = {};
    attr.adapterName = deviceInfo.networkId_ == LOCAL_NETWORK_ID ? "primary" : "remote";
    attr.sampleRate = dstStreamInfo_.samplingRate; // 48000hz
    attr.channel = dstStreamInfo_.channels; // STEREO = 2
    attr.format = ConvertToHdiAdapterFormat(dstStreamInfo_.format); // SAMPLE_S16LE = 1
    attr.deviceNetworkId = deviceInfo.networkId_.c_str();
    attr.deviceType = static_cast<int32_t>(deviceInfo.deviceType_);
    attr.audioStreamFlag = endpointType_ == TYPE_VOIP_MMAP ? AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;

    fastSink_->Init(attr);
    if (!fastSink_->IsInited()) {
        fastSink_ = nullptr;
        return false;
    }
    if (PrepareDeviceBuffer(deviceInfo) != SUCCESS) {
        fastSink_->DeInit();
        fastSink_ = nullptr;
        return false;
    }

    float initVolume = 1.0; // init volume to 1.0
    fastSink_->SetVolume(initVolume, initVolume);

    bool ret = readTimeModel_.ConfigSampleRate(dstStreamInfo_.samplingRate);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "Config LinearPosTimeModel failed.");

    StartThread(attr);
    return true;
}

IMmapAudioRendererSink *AudioEndpointInner::GetFastSink(const AudioDeviceDescriptor &deviceInfo, EndpointType type)
{
    AUDIO_INFO_LOG("Network id %{public}s, endpoint type %{public}d", deviceInfo.networkId_.c_str(), type);
    if (deviceInfo.networkId_ != LOCAL_NETWORK_ID) {
#ifdef DAUDIO_ENABLE
        fastSinkType_ = type == AudioEndpoint::TYPE_MMAP ? FAST_SINK_TYPE_REMOTE : FAST_SINK_TYPE_VOIP;
        // Distributed only requires a singleton because there won't be both voip and regular fast simultaneously
        return RemoteFastAudioRendererSink::GetInstance(deviceInfo.networkId_);
#endif
    }

    if (deviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && deviceInfo.a2dpOffloadFlag_ != A2DP_OFFLOAD) {
        fastSinkType_ = FAST_SINK_TYPE_BLUETOOTH;
        return BluetoothRendererSink::GetMmapInstance();
    }

    if (type == AudioEndpoint::TYPE_MMAP) {
        fastSinkType_ = FAST_SINK_TYPE_NORMAL;
        return FastAudioRendererSink::GetInstance();
    } else if (type == AudioEndpoint::TYPE_VOIP_MMAP) {
        fastSinkType_ = FAST_SINK_TYPE_VOIP;
        return FastAudioRendererSink::GetVoipInstance();
    }
    return nullptr;
}

int32_t AudioEndpointInner::GetAdapterBufferInfo(const AudioDeviceDescriptor &deviceInfo)
{
    int32_t ret = 0;
    AUDIO_INFO_LOG("GetAdapterBufferInfo enter, deviceRole %{public}d.", deviceInfo.deviceRole_);
    if (deviceInfo.deviceRole_ == INPUT_DEVICE) {
        CHECK_AND_RETURN_RET_LOG(fastSource_ != nullptr, ERR_INVALID_HANDLE,
            "fast source is null.");
        ret = fastSource_->GetMmapBufferInfo(dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_,
        dstByteSizePerFrame_);
    } else {
        CHECK_AND_RETURN_RET_LOG(fastSink_ != nullptr, ERR_INVALID_HANDLE, "fast sink is null.");
        ret = fastSink_->GetMmapBufferInfo(dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_,
        dstByteSizePerFrame_);
    }

    if (ret != SUCCESS || dstBufferFd_ == -1 || dstTotalSizeInframe_ == 0 || dstSpanSizeInframe_ == 0 ||
        dstByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("get mmap buffer info fail, ret %{public}d, dstBufferFd %{public}d, \
            dstTotalSizeInframe %{public}d, dstSpanSizeInframe %{public}d, dstByteSizePerFrame %{public}d.",
            ret, dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
        return ERR_ILLEGAL_STATE;
    }
    AUDIO_DEBUG_LOG("end, fd %{public}d.", dstBufferFd_);
    return SUCCESS;
}

int32_t AudioEndpointInner::PrepareDeviceBuffer(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("enter, deviceRole %{public}d.", deviceInfo.deviceRole_);
    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("endpoint buffer is preapred, fd:%{public}d", dstBufferFd_);
        return SUCCESS;
    }

    int32_t ret = GetAdapterBufferInfo(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "get adapter buffer Info fail, ret %{public}d.", ret);

    // spanDuration_ may be less than the correct time of dstSpanSizeInframe_.
    spanDuration_ = static_cast<int64_t>(dstSpanSizeInframe_) * AUDIO_NS_PER_SECOND /
        static_cast<int64_t>(dstStreamInfo_.samplingRate);
    int64_t temp = spanDuration_ / 5 * 3; // 3/5 spanDuration
    serverAheadReadTime_ = temp < ONE_MILLISECOND_DURATION ? ONE_MILLISECOND_DURATION : temp; // at least 1ms ahead.
    AUDIO_DEBUG_LOG("panDuration %{public}" PRIu64" ns, serverAheadReadTime %{public}" PRIu64" ns.",
        spanDuration_, serverAheadReadTime_);

    CHECK_AND_RETURN_RET_LOG(spanDuration_ > 0 && spanDuration_ < MAX_SPAN_DURATION_NS,
        ERR_INVALID_PARAM, "mmap span info error, spanDuration %{public}" PRIu64".", spanDuration_);
    dstAudioBuffer_ = OHAudioBuffer::CreateFromRemote(dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_,
        AUDIO_SERVER_ONLY, dstBufferFd_, OHAudioBuffer::INVALID_BUFFER_FD);
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr && dstAudioBuffer_->GetBufferHolder() ==
        AudioBufferHolder::AUDIO_SERVER_ONLY, ERR_ILLEGAL_STATE, "create buffer from remote fail.");

    if (dstAudioBuffer_ == nullptr || dstAudioBuffer_->GetStreamStatus() == nullptr) {
        AUDIO_ERR_LOG("The stream status is null!");
        return ERR_INVALID_PARAM;
    }

    dstAudioBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_IDEL);

    // clear data buffer
    ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0, dstAudioBuffer_->GetDataSize());
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret %{public}d, fd %{public}d.", ret, dstBufferFd_);
    }
    InitAudiobuffer(true);

    AUDIO_DEBUG_LOG("end, fd %{public}d.", dstBufferFd_);
    return SUCCESS;
}

void AudioEndpointInner::InitAudiobuffer(bool resetReadWritePos)
{
    CHECK_AND_RETURN_LOG((dstAudioBuffer_ != nullptr), "dst audio buffer is null.");
    if (resetReadWritePos) {
        dstAudioBuffer_->ResetCurReadWritePos(0, 0);
    }

    uint32_t spanCount = dstAudioBuffer_->GetSpanCount();
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = dstAudioBuffer_->GetSpanInfoByIndex(i);
        CHECK_AND_RETURN_LOG(spanInfo != nullptr, "InitAudiobuffer failed.");
        if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
            spanInfo->spanStatus = SPAN_WRITE_DONE;
        } else {
            spanInfo->spanStatus = SPAN_READ_DONE;
        }
        spanInfo->offsetInFrame = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->writeStartTime = 0;
        spanInfo->writeDoneTime = 0;

        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->isMute = false;
    }
    return;
}

int32_t AudioEndpointInner::GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe)
{
    totalSizeInframe = dstTotalSizeInframe_;
    spanSizeInframe = dstSpanSizeInframe_;
    return SUCCESS;
}

bool AudioEndpointInner::IsAnyProcessRunning()
{
    std::lock_guard<std::mutex> lock(listLock_);
    return IsAnyProcessRunningInner();
}

// Should be called with AudioEndpointInner::listLock_ locked
bool AudioEndpointInner::IsAnyProcessRunningInner()
{
    bool isRunning = false;
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        if (processBufferList_[i]->GetStreamStatus() &&
            processBufferList_[i]->GetStreamStatus()->load() == STREAM_RUNNING) {
            isRunning = true;
            break;
        }
    }
    return isRunning;
}

void AudioEndpointInner::RecordReSyncPosition()
{
    AUDIO_INFO_LOG("RecordReSyncPosition enter.");
    uint64_t curHdiWritePos = 0;
    int64_t writeTime = 0;
    CHECK_AND_RETURN_LOG(GetDeviceHandleInfo(curHdiWritePos, writeTime),
        "get device handle info fail.");
    AUDIO_DEBUG_LOG("get capturer info, curHdiWritePos %{public}" PRIu64", writeTime %{public}" PRId64".",
        curHdiWritePos, writeTime);
    int64_t temp = ClockTime::GetCurNano() - writeTime;
    if (temp > spanDuration_) {
        AUDIO_WARNING_LOG("GetDeviceHandleInfo cost long time %{public}" PRIu64".", temp);
    }

    writeTimeModel_.ResetFrameStamp(curHdiWritePos, writeTime);
    uint64_t nextDstReadPos = curHdiWritePos;
    uint64_t nextDstWritePos = curHdiWritePos;
    InitAudiobuffer(false);
    int32_t ret = dstAudioBuffer_->ResetCurReadWritePos(nextDstReadPos, nextDstWritePos);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "ResetCurReadWritePos failed.");

    SpanInfo *nextReadSapn = dstAudioBuffer_->GetSpanInfo(nextDstReadPos);
    CHECK_AND_RETURN_LOG(nextReadSapn != nullptr, "GetSpanInfo failed.");
    nextReadSapn->offsetInFrame = nextDstReadPos;
    nextReadSapn->spanStatus = SpanStatus::SPAN_WRITE_DONE;
}

void AudioEndpointInner::ReSyncPosition()
{
    Trace loopTrace("AudioEndpoint::ReSyncPosition");
    uint64_t curHdiReadPos = 0;
    int64_t readTime = 0;
    bool res = GetDeviceHandleInfo(curHdiReadPos, readTime);
    CHECK_AND_RETURN_LOG(res, "ReSyncPosition call GetDeviceHandleInfo failed.");
    int64_t curTime = ClockTime::GetCurNano();
    int64_t temp = curTime - readTime;
    if (temp > spanDuration_) {
        AUDIO_ERR_LOG("GetDeviceHandleInfo may cost long time.");
    }

    readTimeModel_.ResetFrameStamp(curHdiReadPos, readTime);
    uint64_t nextDstWritePos = curHdiReadPos + dstSpanSizeInframe_;
    InitAudiobuffer(false);
    int32_t ret = dstAudioBuffer_->ResetCurReadWritePos(nextDstWritePos, nextDstWritePos);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "ResetCurReadWritePos failed.");

    SpanInfo *nextWriteSapn = dstAudioBuffer_->GetSpanInfo(nextDstWritePos);
    CHECK_AND_RETURN_LOG(nextWriteSapn != nullptr, "GetSpanInfo failed.");
    nextWriteSapn->offsetInFrame = nextDstWritePos;
    nextWriteSapn->spanStatus = SpanStatus::SPAN_READ_DONE;
    return;
}

bool AudioEndpointInner::StartDevice(EndpointStatus preferredState)
{
    AUDIO_INFO_LOG("StartDevice enter.");
    // how to modify the status while unlinked and started?
    CHECK_AND_RETURN_RET_LOG(endpointStatus_ == IDEL, false, "Endpoint status is %{public}s",
        GetStatusStr(endpointStatus_).c_str());
    endpointStatus_ = STARTING;
    if ((deviceInfo_.deviceRole_ == INPUT_DEVICE && (fastSource_ == nullptr || fastSource_->Start() != SUCCESS)) ||
        (deviceInfo_.deviceRole_ == OUTPUT_DEVICE && (fastSink_ == nullptr || fastSink_->Start() != SUCCESS))) {
        HandleStartDeviceFailed();
        return false;
    }
    isStarted_ = true;

    if (isInnerCapEnabled_) {
        Trace trace("AudioEndpointInner::StartDupStream");
        std::lock_guard<std::mutex> lock(dupMutex_);
        if (dupStream_ != nullptr) {
            dupStream_->Start();
        }
    }

    std::unique_lock<std::mutex> lock(loopThreadLock_);
    needReSyncPosition_ = true;
    endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    if (preferredState != INVALID) {
        AUDIO_INFO_LOG("Preferred state: %{public}d, current: %{public}d", preferredState, endpointStatus_.load());
        endpointStatus_ = preferredState;
    }
    workThreadCV_.notify_all();
    AUDIO_DEBUG_LOG("StartDevice out, status is %{public}s", GetStatusStr(endpointStatus_).c_str());
    return true;
}

void AudioEndpointInner::HandleStartDeviceFailed()
{
    AUDIO_ERR_LOG("Start failed for %{public}d, endpoint type %{public}u, process list size: %{public}zu.",
        deviceInfo_.deviceRole_, endpointType_, processList_.size());
    std::lock_guard<std::mutex> lock(listLock_);
    isStarted_ = false;
    if (processList_.size() <= 1) { // The endpoint only has the current stream
        endpointStatus_ = UNLINKED;
    } else {
        endpointStatus_ = IDEL;
    }
    workThreadCV_.notify_all();
}

// will not change state to stopped
bool AudioEndpointInner::DelayStopDevice()
{
    AUDIO_INFO_LOG("Status:%{public}s", GetStatusStr(endpointStatus_).c_str());

    // Clear data buffer to avoid noise in some case.
    if (dstAudioBuffer_ != nullptr) {
        int32_t ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0,
            dstAudioBuffer_->GetDataSize());
        if (ret != EOK) {
            AUDIO_WARNING_LOG("reset buffer fail, ret %{public}d.", ret);
        }
    }

    if (isInnerCapEnabled_) {
        Trace trace("AudioEndpointInner::StopDupStreamInDelay");
        std::lock_guard<std::mutex> lock(dupMutex_);
        if (dupStream_ != nullptr) {
            dupStream_->Stop();
        }
    }

    if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
        CHECK_AND_RETURN_RET_LOG(fastSource_ != nullptr && fastSource_->Stop() == SUCCESS,
            false, "Source stop failed.");
    } else {
        CHECK_AND_RETURN_RET_LOG(endpointStatus_ == IDEL && fastSink_ != nullptr && fastSink_->Stop() == SUCCESS,
            false, "Sink stop failed.");
    }
    isStarted_ = false;
    return true;
}

bool AudioEndpointInner::StopDevice()
{
    DeinitLatencyMeasurement();

    AUDIO_INFO_LOG("StopDevice with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    // todo
    endpointStatus_ = STOPPING;
    // Clear data buffer to avoid noise in some case.
    if (dstAudioBuffer_ != nullptr) {
        int32_t ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0,
            dstAudioBuffer_->GetDataSize());
        AUDIO_INFO_LOG("StopDevice clear buffer ret:%{public}d", ret);
    }

    if (isInnerCapEnabled_) {
        Trace trace("AudioEndpointInner::StopDupStream");
        std::lock_guard<std::mutex> lock(dupMutex_);
        if (dupStream_ != nullptr) {
            dupStream_->Stop();
        }
    }

    if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
        CHECK_AND_RETURN_RET_LOG(fastSource_ != nullptr && fastSource_->Stop() == SUCCESS,
            false, "Source stop failed.");
    } else {
        CHECK_AND_RETURN_RET_LOG(fastSink_ != nullptr && fastSink_->Stop() == SUCCESS,
            false, "Sink stop failed.");
    }
    endpointStatus_ = STOPPED;
    isStarted_ = false;
    return true;
}

int32_t AudioEndpointInner::OnStart(IAudioProcessStream *processStream)
{
    InitLatencyMeasurement();
    // Prevents the audio from immediately stopping at 0 volume on start
    delayStopTimeForZeroVolume_ = ClockTime::GetCurNano() + DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS;
    AUDIO_PRERELEASE_LOGI("OnStart endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == RUNNING) {
        AUDIO_INFO_LOG("OnStart find endpoint already in RUNNING.");
        return SUCCESS;
    }
    if (endpointStatus_ == IDEL) {
        // call sink start
        if (!isStarted_) {
            CHECK_AND_RETURN_RET_LOG(StartDevice(RUNNING), ERR_OPERATION_FAILED, "StartDevice failed");
        }
    }

    endpointStatus_ = RUNNING;
    delayStopTime_ = INT64_MAX;
    return SUCCESS;
}

int32_t AudioEndpointInner::OnPause(IAudioProcessStream *processStream)
{
    AUDIO_PRERELEASE_LOGI("OnPause endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == RUNNING) {
        endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    }
    if (endpointStatus_ == IDEL) {
        // delay call sink stop when no process running
        AUDIO_PRERELEASE_LOGI("OnPause status is IDEL, need delay call stop");
        delayStopTime_ = ClockTime::GetCurNano() + DELAY_STOP_HDI_TIME;
    }
    // todo
    return SUCCESS;
}

int32_t AudioEndpointInner::GetProcLastWriteDoneInfo(const std::shared_ptr<OHAudioBuffer> processBuffer,
    uint64_t curWriteFrame, uint64_t &proHandleFrame, int64_t &proHandleTime)
{
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_HANDLE, "Process found but buffer is null");
    uint64_t curReadFrame = processBuffer->GetCurReadFrame();
    SpanInfo *curWriteSpan = processBuffer->GetSpanInfo(curWriteFrame);
    CHECK_AND_RETURN_RET_LOG(curWriteSpan != nullptr, ERR_INVALID_HANDLE,
        "curWriteSpan of curWriteFrame %{public}" PRIu64" is null", curWriteFrame);
    if (curWriteSpan->spanStatus == SpanStatus::SPAN_WRITE_DONE || curWriteFrame < dstSpanSizeInframe_ ||
        curWriteFrame < curReadFrame) {
        proHandleFrame = curWriteFrame;
        proHandleTime = curWriteSpan->writeDoneTime;
    } else {
        int32_t ret = GetProcLastWriteDoneInfo(processBuffer, curWriteFrame - dstSpanSizeInframe_,
            proHandleFrame, proHandleTime);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
            "get process last write done info fail, ret %{public}d.", ret);
    }

    AUDIO_INFO_LOG("GetProcLastWriteDoneInfo end, curWriteFrame %{public}" PRIu64", proHandleFrame %{public}" PRIu64", "
        "proHandleTime %{public}" PRId64".", curWriteFrame, proHandleFrame, proHandleTime);
    return SUCCESS;
}

int32_t AudioEndpointInner::OnUpdateHandleInfo(IAudioProcessStream *processStream)
{
    Trace trace("AudioEndpoint::OnUpdateHandleInfo");
    bool isFind = false;
    std::lock_guard<std::mutex> lock(listLock_);
    auto processItr = processList_.begin();
    while (processItr != processList_.end()) {
        if (*processItr != processStream) {
            processItr++;
            continue;
        }
        std::shared_ptr<OHAudioBuffer> processBuffer = (*processItr)->GetStreamBuffer();
        CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_OPERATION_FAILED, "Process found but buffer is null");
        uint64_t proHandleFrame = 0;
        int64_t proHandleTime = 0;
        if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
            uint64_t curWriteFrame = processBuffer->GetCurWriteFrame();
            int32_t ret = GetProcLastWriteDoneInfo(processBuffer, curWriteFrame, proHandleFrame, proHandleTime);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
                "get process last write done info fail, ret %{public}d.", ret);
            processBuffer->SetHandleInfo(proHandleFrame, proHandleTime);
        } else {
            // For output device, handle info is updated in CheckAllBufferReady
            processBuffer->GetHandleInfo(proHandleFrame, proHandleTime);
        }

        isFind = true;
        break;
    }
    CHECK_AND_RETURN_RET_LOG(isFind, ERR_OPERATION_FAILED, "Can not find any process to UpdateHandleInfo");
    return SUCCESS;
}

int32_t AudioEndpointInner::LinkProcessStream(IAudioProcessStream *processStream)
{
    CHECK_AND_RETURN_RET_LOG(processStream != nullptr, ERR_INVALID_PARAM, "IAudioProcessStream is null");
    std::shared_ptr<OHAudioBuffer> processBuffer = processStream->GetStreamBuffer();
    processBuffer->SetSessionId(processStream->GetAudioSessionId());
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_PARAM, "processBuffer is null");
    CHECK_AND_RETURN_RET_LOG(processBuffer->GetStreamStatus() != nullptr, ERR_INVALID_PARAM,
        "the stream status is null");

    CHECK_AND_RETURN_RET_LOG(processList_.size() < MAX_LINKED_PROCESS, ERR_OPERATION_FAILED, "reach link limit.");

    AUDIO_INFO_LOG("LinkProcessStream start status is:%{public}s.", GetStatusStr(endpointStatus_).c_str());

    bool needEndpointRunning = processBuffer->GetStreamStatus()->load() == STREAM_RUNNING;

    if (endpointStatus_ == STARTING) {
        AUDIO_INFO_LOG("LinkProcessStream wait start begin.");
        std::unique_lock<std::mutex> lock(loopThreadLock_);
        workThreadCV_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME_IN_DEFAULT), [this] {
            return endpointStatus_ != STARTING;
        });
        AUDIO_DEBUG_LOG("LinkProcessStream wait start end.");
    }

    if (endpointStatus_ == RUNNING) {
        LinkProcessStreamExt(processStream, processBuffer);
        return SUCCESS;
    }

    if (endpointStatus_ == UNLINKED) {
        endpointStatus_ = IDEL; // handle push_back in IDEL
        if (isDeviceRunningInIdel_) {
            CHECK_AND_RETURN_RET_LOG(StartDevice(), ERR_OPERATION_FAILED, "StartDevice failed");
            delayStopTime_ = ClockTime::GetCurNano() + DELAY_STOP_HDI_TIME;
        }
    }

    if (endpointStatus_ == IDEL) {
        {
            std::lock_guard<std::mutex> lock(listLock_);
            processList_.push_back(processStream);
            processBufferList_.push_back(processBuffer);
        }
        if (!needEndpointRunning) {
            AUDIO_INFO_LOG("LinkProcessStream success, process stream status is not running.");
            return SUCCESS;
        }
        // needEndpointRunning = true
        if (isDeviceRunningInIdel_) {
            endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
        } else {
            // needEndpointRunning = true & isDeviceRunningInIdel_ = false
            // KeepWorkloopRunning will wait on IDEL
            CHECK_AND_RETURN_RET_LOG(StartDevice(), ERR_OPERATION_FAILED, "StartDevice failed");
        }
        AUDIO_INFO_LOG("LinkProcessStream success with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
        return SUCCESS;
    }

    AUDIO_INFO_LOG("LinkProcessStream success with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    return SUCCESS;
}

void AudioEndpointInner::LinkProcessStreamExt(IAudioProcessStream *processStream,
    const std::shared_ptr<OHAudioBuffer>& processBuffer)
{
    std::lock_guard<std::mutex> lock(listLock_);
    processList_.push_back(processStream);
    processBufferList_.push_back(processBuffer);
    AUDIO_INFO_LOG("LinkProcessStream success in RUNNING.");
}

int32_t AudioEndpointInner::UnlinkProcessStream(IAudioProcessStream *processStream)
{
    AUDIO_INFO_LOG("UnlinkProcessStream in status:%{public}s.", GetStatusStr(endpointStatus_).c_str());
    CHECK_AND_RETURN_RET_LOG(processStream != nullptr, ERR_INVALID_PARAM, "IAudioProcessStream is null");
    std::shared_ptr<OHAudioBuffer> processBuffer = processStream->GetStreamBuffer();
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_PARAM, "processBuffer is null");

    bool isFind = false;
    std::lock_guard<std::mutex> lock(listLock_);
    auto processItr = processList_.begin();
    auto bufferItr = processBufferList_.begin();
    while (processItr != processList_.end()) {
        if (*processItr == processStream && *bufferItr == processBuffer) {
            processList_.erase(processItr);
            processBufferList_.erase(bufferItr);
            isFind = true;
            break;
        } else {
            processItr++;
            bufferItr++;
        }
    }
    if (processList_.size() == 0) {
        StopDevice();
        endpointStatus_ = UNLINKED;
    } else if (!IsAnyProcessRunningInner()) {
        endpointStatus_ = IDEL;
        isStarted_ = false;
        delayStopTime_ = DELAY_STOP_HDI_TIME_WHEN_NO_RUNNING_NS;
    }

    AUDIO_DEBUG_LOG("UnlinkProcessStream end, %{public}s the process.", (isFind ? "find and remove" : "not find"));
    return SUCCESS;
}

void AudioEndpointInner::CheckStandBy()
{
    if (endpointStatus_ == RUNNING) {
        endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    }

    if (endpointStatus_ == RUNNING) {
        return;
    }

    AUDIO_INFO_LOG("endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == IDEL) {
        // delay call sink stop when no process running
        AUDIO_INFO_LOG("status is IDEL, need delay call stop");
        delayStopTime_ = ClockTime::GetCurNano() + DELAY_STOP_HDI_TIME;
    }
}

bool AudioEndpointInner::CheckAllBufferReady(int64_t checkTime, uint64_t curWritePos)
{
    bool isAllReady = true;
    bool needCheckStandby = false;
    {
        // lock list without sleep
        std::lock_guard<std::mutex> lock(listLock_);
        for (size_t i = 0; i < processBufferList_.size(); i++) {
            std::shared_ptr<OHAudioBuffer> tempBuffer = processBufferList_[i];
            uint64_t eachCurReadPos = processBufferList_[i]->GetCurReadFrame();
            lastHandleProcessTime_ = checkTime;
            processBufferList_[i]->SetHandleInfo(eachCurReadPos, lastHandleProcessTime_); // update handle info
            if (tempBuffer->GetStreamStatus() &&
                tempBuffer->GetStreamStatus()->load() != StreamStatus::STREAM_RUNNING) {
                // Process is not running, server will continue to check the same location in the next cycle.
                int64_t duration = 5000000; // 5ms
                processBufferList_[i]->SetHandleInfo(eachCurReadPos, lastHandleProcessTime_ + duration);
                continue; // process not running
            }
            // Status is RUNNING
            int64_t current = ClockTime::GetCurNano();
            int64_t lastWrittenTime = tempBuffer->GetLastWrittenTime();
            uint32_t sessionId = processList_[i]->GetAudioSessionId();
            if (current - lastWrittenTime > WAIT_CLIENT_STANDBY_TIME_NS) {
                Trace trace("AudioEndpoint::MarkClientStandby:" + std::to_string(sessionId));
                AUDIO_INFO_LOG("change the status to stand-by, session %{public}u", sessionId);
                processList_[i]->EnableStandby();
                WriterRenderStreamStandbySysEvent(sessionId, 1);
                needCheckStandby = true;
                continue;
            }
            uint64_t curRead = tempBuffer->GetCurReadFrame();
            SpanInfo *curReadSpan = tempBuffer->GetSpanInfo(curRead);
            if (curReadSpan == nullptr || curReadSpan->spanStatus != SpanStatus::SPAN_WRITE_DONE) {
                AUDIO_DEBUG_LOG("Find one process not ready"); // print uid of the process?
                isAllReady = false;
                continue;
            }
            // process Status is RUNNING && buffer status is WRITE_DONE
            tempBuffer->SetLastWrittenTime(current);
        }
    }

    if (needCheckStandby) {
        CheckStandBy();
    }

    if (!isAllReady) {
        WaitAllProcessReady(curWritePos);
    }
    return isAllReady;
}

void AudioEndpointInner::WaitAllProcessReady(uint64_t curWritePos)
{
    Trace trace("AudioEndpoint::WaitAllProcessReady");
    int64_t tempWakeupTime = readTimeModel_.GetTimeOfPos(curWritePos) + WRITE_TO_HDI_AHEAD_TIME;
    if (tempWakeupTime - ClockTime::GetCurNano() < ONE_MILLISECOND_DURATION) {
        ClockTime::RelativeSleep(ONE_MILLISECOND_DURATION);
    } else {
        ClockTime::AbsoluteSleep(tempWakeupTime); // sleep to hdi read time ahead 1ms.
    }
}

void AudioEndpointInner::MixToDupStream(const std::vector<AudioStreamData> &srcDataList)
{
    Trace trace("AudioEndpointInner::MixToDupStream");
    std::lock_guard<std::mutex> lock(dupMutex_);
    CHECK_AND_RETURN_LOG(dupBuffer_ != nullptr, "Buffer is not ready");

    for (size_t i = 0; i < srcDataList.size(); i++) {
        if (!srcDataList[i].isInnerCaped) {
            continue;
        }
        size_t dataLength = dupBufferSize_;
        dataLength /= 2; // SAMPLE_S16LE--> 2 byte
        int16_t *dstPtr = reinterpret_cast<int16_t *>(dupBuffer_.get());

        for (size_t offset = 0; dataLength > 0; dataLength--) {
            int32_t sum = *dstPtr;
            sum += *(reinterpret_cast<int16_t *>(srcDataList[i].bufferDesc.buffer) + offset);
            *dstPtr = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
            dstPtr++;
            offset++;
        }
    }
    BufferDesc temp;
    temp.buffer = dupBuffer_.get();
    temp.bufLength = dupBufferSize_;
    temp.dataLength = dupBufferSize_;

    CHECK_AND_RETURN_LOG(dupStream_ != nullptr, "dupStream_ is nullptr");
    int32_t ret = dupStream_->EnqueueBuffer(temp);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "EnqueueBuffer failed:%{public}d", ret);

    ret = memset_s(reinterpret_cast<void *>(dupBuffer_.get()), dupBufferSize_, 0, dupBufferSize_);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret %{public}d", ret);
    }
}

void AudioEndpointInner::ProcessData(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData)
{
    size_t srcListSize = srcDataList.size();

    for (size_t i = 0; i < srcListSize; i++) {
        if (srcDataList[i].streamInfo.format != SAMPLE_S16LE || srcDataList[i].streamInfo.channels != STEREO ||
            srcDataList[i].bufferDesc.bufLength != dstData.bufferDesc.bufLength ||
            srcDataList[i].bufferDesc.dataLength != dstData.bufferDesc.dataLength) {
            AUDIO_ERR_LOG("ProcessData failed, streamInfo are different");
            return;
        }
    }

    // Assum using the same format and same size
    CHECK_AND_RETURN_LOG(dstData.streamInfo.format == SAMPLE_S16LE && dstData.streamInfo.channels == STEREO,
        "ProcessData failed, streamInfo are not support");

    size_t dataLength = dstData.bufferDesc.dataLength;
    dataLength /= 2; // SAMPLE_S16LE--> 2 byte
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstData.bufferDesc.buffer);
    for (size_t offset = 0; dataLength > 0; dataLength--) {
        int32_t sum = 0;
        for (size_t i = 0; i < srcListSize; i++) {
            int32_t vol = srcDataList[i].volumeStart; // change to modify volume of each channel
            int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDataList[i].bufferDesc.buffer) + offset;
            sum += (*srcPtr * static_cast<int64_t>(vol)) >> VOLUME_SHIFT_NUMBER; // 1/65536
            ZeroVolumeCheck(vol);
        }
        offset++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
    HandleZeroVolumeCheckEvent();
}

void AudioEndpointInner::HandleZeroVolumeCheckEvent()
{
    if (fastSinkType_ == FAST_SINK_TYPE_BLUETOOTH) {
        return;
    }
    if (!zeroVolumeStopDevice_ && (ClockTime::GetCurNano() >= delayStopTimeForZeroVolume_)) {
        if (isStarted_) {
            if (fastSink_ != nullptr && fastSink_->Stop() == SUCCESS) {
                AUDIO_INFO_LOG("Volume from none-zero to zero more than 4s, stop device success.");
                isStarted_ = false;
            } else {
                AUDIO_INFO_LOG("Volume from none-zero to zero more than 4s, stop device failed.");
                isStarted_ = true;
            }
        }
        zeroVolumeStopDevice_ = true;
    }
}


void AudioEndpointInner::HandleRendererDataParams(const AudioStreamData &srcData, const AudioStreamData &dstData)
{
    if (srcData.streamInfo.encoding != dstData.streamInfo.encoding) {
        AUDIO_ERR_LOG("Different encoding formats");
        return;
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == STEREO) {
        return ProcessSingleData(srcData, dstData);
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == MONO) {
        CHECK_AND_RETURN_LOG(processList_.size() > 0 && processList_[0] != nullptr, "No avaliable process");
        BufferDesc &convertedBuffer = processList_[0]->GetConvertedBuffer();
        int32_t ret = FormatConverter::S16MonoToS16Stereo(srcData.bufferDesc, convertedBuffer);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Convert channel from mono to stereo failed");
        AudioStreamData dataAfterProcess = srcData;
        dataAfterProcess.bufferDesc = convertedBuffer;
        ProcessSingleData(dataAfterProcess, dstData);
        ret = memset_s(static_cast<void *>(convertedBuffer.buffer), convertedBuffer.bufLength, 0,
            convertedBuffer.bufLength);
        CHECK_AND_RETURN_LOG(ret == EOK, "memset converted buffer to 0 failed");
    }
}

void AudioEndpointInner::ProcessSingleData(const AudioStreamData &srcData, const AudioStreamData &dstData)
{
    CHECK_AND_RETURN_LOG(dstData.streamInfo.format == SAMPLE_S16LE && dstData.streamInfo.channels == STEREO,
        "ProcessData failed, streamInfo are not support");

    size_t dataLength = dstData.bufferDesc.dataLength;
    dataLength /= 2; // SAMPLE_S16LE--> 2 byte
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstData.bufferDesc.buffer);
    for (size_t offset = 0; dataLength > 0; dataLength--) {
        int32_t vol = srcData.volumeStart; // change to modify volume of each channel
        int16_t *srcPtr = reinterpret_cast<int16_t *>(srcData.bufferDesc.buffer) + offset;
        int32_t sum = (*srcPtr * static_cast<int64_t>(vol)) >> VOLUME_SHIFT_NUMBER; // 1/65536
        ZeroVolumeCheck(vol);
        offset++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
    HandleZeroVolumeCheckEvent();
}

void AudioEndpointInner::ZeroVolumeCheck(const int32_t vol)
{
    if (fastSinkType_ == FAST_SINK_TYPE_BLUETOOTH) {
        return;
    }
    if (std::abs(vol - 0) <= std::numeric_limits<float>::epsilon()) {
        if (!zeroVolumeStopDevice_ && !isVolumeAlreadyZero_) {
            AUDIO_INFO_LOG("Begin zero volume, will stop device.");
            delayStopTimeForZeroVolume_ = ClockTime::GetCurNano() + DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS;
            isVolumeAlreadyZero_ = true;
        }
    } else {
        if (zeroVolumeStopDevice_ && !isStarted_) {
            if (fastSink_ == nullptr || fastSink_->Start() != SUCCESS) {
                AUDIO_INFO_LOG("Volume from zero to none-zero, start device failed.");
                isStarted_ = false;
            } else {
                AUDIO_INFO_LOG("Volume from zero to none-zero, start device success.");
                isStarted_ = true;
                needReSyncPosition_ = true;
            }
            zeroVolumeStopDevice_ = false;
        }
        isVolumeAlreadyZero_ = false;
        delayStopTimeForZeroVolume_ = INT64_MAX;
    }
}

// call with listLock_ hold
void AudioEndpointInner::GetAllReadyProcessData(std::vector<AudioStreamData> &audioDataList)
{
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        uint64_t curRead = processBufferList_[i]->GetCurReadFrame();
        Trace trace("AudioEndpoint::ReadProcessData->" + std::to_string(curRead));
        SpanInfo *curReadSpan = processBufferList_[i]->GetSpanInfo(curRead);
        CHECK_AND_CONTINUE_LOG(curReadSpan != nullptr, "GetSpanInfo failed, can not get client curReadSpan");
        AudioStreamData streamData;
        Volume vol = {true, 1.0f, 0};
        AudioStreamType streamType = processList_[i]->GetAudioStreamType();
        AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
        DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
        bool muteFlag = processList_[i]->GetMuteState();
        if (deviceInfo_.networkId_ == LOCAL_NETWORK_ID &&
            !(deviceInfo_.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && volumeType == STREAM_MUSIC &&
                PolicyHandler::GetInstance().IsAbsVolumeSupported()) &&
            PolicyHandler::GetInstance().GetSharedVolume(volumeType, deviceType, vol)) {
            streamData.volumeStart = vol.isMute ? 0 : static_cast<int32_t>(curReadSpan->volumeStart * vol.volumeFloat);
        } else {
            streamData.volumeStart = curReadSpan->volumeStart;
        }
        streamData.volumeEnd = curReadSpan->volumeEnd;
        streamData.streamInfo = processList_[i]->GetStreamInfo();
        streamData.isInnerCaped = processList_[i]->GetInnerCapState();
        SpanStatus targetStatus = SpanStatus::SPAN_WRITE_DONE;
        if (curReadSpan->spanStatus.compare_exchange_strong(targetStatus, SpanStatus::SPAN_READING)) {
            processBufferList_[i]->GetReadbuffer(curRead, streamData.bufferDesc); // check return?
            if (muteFlag) {
                memset_s(static_cast<void *>(streamData.bufferDesc.buffer), streamData.bufferDesc.bufLength,
                    0, streamData.bufferDesc.bufLength);
            }
            CheckPlaySignal(streamData.bufferDesc.buffer, streamData.bufferDesc.bufLength);
            audioDataList.push_back(streamData);
            curReadSpan->readStartTime = ClockTime::GetCurNano();
            processList_[i]->WriteDumpFile(static_cast<void *>(streamData.bufferDesc.buffer),
                streamData.bufferDesc.bufLength);
        }
    }
}

bool AudioEndpointInner::ProcessToEndpointDataHandle(uint64_t curWritePos)
{
    std::lock_guard<std::mutex> lock(listLock_);

    std::vector<AudioStreamData> audioDataList;
    GetAllReadyProcessData(audioDataList);

    AudioStreamData dstStreamData;
    dstStreamData.streamInfo = dstStreamInfo_;
    int32_t ret = dstAudioBuffer_->GetWriteBuffer(curWritePos, dstStreamData.bufferDesc);
    CHECK_AND_RETURN_RET_LOG(((ret == SUCCESS && dstStreamData.bufferDesc.buffer != nullptr)), false,
        "GetWriteBuffer failed, ret:%{public}d", ret);

    SpanInfo *curWriteSpan = dstAudioBuffer_->GetSpanInfo(curWritePos);
    CHECK_AND_RETURN_RET_LOG(curWriteSpan != nullptr, false, "GetSpanInfo failed, can not get curWriteSpan");

    dstStreamData.volumeStart = curWriteSpan->volumeStart;
    dstStreamData.volumeEnd = curWriteSpan->volumeEnd;

    Trace trace("AudioEndpoint::WriteDstBuffer=>" + std::to_string(curWritePos));
    // do write work
    if (audioDataList.size() == 0) {
        memset_s(dstStreamData.bufferDesc.buffer, dstStreamData.bufferDesc.bufLength, 0,
            dstStreamData.bufferDesc.bufLength);
    } else {
        if (endpointType_ == TYPE_VOIP_MMAP && audioDataList.size() == 1) {
            HandleRendererDataParams(audioDataList[0], dstStreamData);
        } else {
            ProcessData(audioDataList, dstStreamData);
        }
    }

    if (isInnerCapEnabled_) {
        MixToDupStream(audioDataList);
    }

    DfxOperation(dstStreamData.bufferDesc, dstStreamInfo_.format, dstStreamInfo_.channels);

    if (AudioDump::GetInstance().GetVersionType() == BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpHdi_, static_cast<void *>(dstStreamData.bufferDesc.buffer),
            dstStreamData.bufferDesc.bufLength);
        AudioCacheMgr::GetInstance().CacheData(dumpHdiName_,
            static_cast<void *>(dstStreamData.bufferDesc.buffer), dstStreamData.bufferDesc.bufLength);
    }

    CheckUpdateState(reinterpret_cast<char *>(dstStreamData.bufferDesc.buffer),
        dstStreamData.bufferDesc.bufLength);

    return true;
}

void AudioEndpointInner::DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel) const
{
    ChannelVolumes vols = VolumeTools::CountVolumeLevel(buffer, format, channel);
    if (channel == MONO) {
        Trace::Count(logUtilsTag_, vols.volStart[0]);
    } else {
        Trace::Count(logUtilsTag_, (vols.volStart[0] + vols.volStart[1]) / HALF_FACTOR);
    }
    AudioLogUtils::ProcessVolumeData(logUtilsTag_, vols, volumeDataCount_);
}

void AudioEndpointInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(dstStreamInfo_.format),
            frame, replyBytes);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float AudioEndpointInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int64_t AudioEndpointInner::GetPredictNextReadTime(uint64_t posInFrame)
{
    Trace trace("AudioEndpoint::GetPredictNextRead");
    uint64_t handleSpanCnt = posInFrame / dstSpanSizeInframe_;
    uint32_t startPeriodCnt = 20; // sync each time when start
    uint32_t oneBigPeriodCnt = 40; // 200ms
    if (handleSpanCnt < startPeriodCnt || handleSpanCnt % oneBigPeriodCnt == 0) {
        updateThreadCV_.notify_all();
    }
    uint64_t readFrame = 0;
    int64_t readtime = 0;
    if (readTimeModel_.GetFrameStamp(readFrame, readtime)) {
        if (readFrame != posInFrame_) {
            readTimeModel_.UpdataFrameStamp(posInFrame_, timeInNano_);
        }
    }

    int64_t nextHdiReadTime = readTimeModel_.GetTimeOfPos(posInFrame);
    return nextHdiReadTime;
}

int64_t AudioEndpointInner::GetPredictNextWriteTime(uint64_t posInFrame)
{
    uint64_t handleSpanCnt = posInFrame / dstSpanSizeInframe_;
    uint32_t startPeriodCnt = 20;
    uint32_t oneBigPeriodCnt = 40;
    if (handleSpanCnt < startPeriodCnt || handleSpanCnt % oneBigPeriodCnt == 0) {
        updateThreadCV_.notify_all();
    }
    uint64_t writeFrame = 0;
    int64_t writetime = 0;
    if (writeTimeModel_.GetFrameStamp(writeFrame, writetime)) {
        if (writeFrame != posInFrame_) {
            writeTimeModel_.UpdataFrameStamp(posInFrame_, timeInNano_);
        }
    }
    int64_t nextHdiWriteTime = writeTimeModel_.GetTimeOfPos(posInFrame);
    return nextHdiWriteTime;
}

bool AudioEndpointInner::RecordPrepareNextLoop(uint64_t curReadPos, int64_t &wakeUpTime)
{
    uint64_t nextHandlePos = curReadPos + dstSpanSizeInframe_;
    int64_t nextHdiWriteTime = GetPredictNextWriteTime(nextHandlePos);
    int64_t tempDelay = endpointType_ == TYPE_VOIP_MMAP ? RECORD_VOIP_DELAY_TIME_NS : RECORD_DELAY_TIME_NS;
    int64_t predictWakeupTime = nextHdiWriteTime + tempDelay;
    if (predictWakeupTime <= ClockTime::GetCurNano()) {
        wakeUpTime = ClockTime::GetCurNano() + ONE_MILLISECOND_DURATION;
        AUDIO_ERR_LOG("hdi send wrong position time");
    } else {
        wakeUpTime = predictWakeupTime;
    }

    int32_t ret = dstAudioBuffer_->SetCurWriteFrame(nextHandlePos);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "set dst buffer write frame fail, ret %{public}d.", ret);
    ret = dstAudioBuffer_->SetCurReadFrame(nextHandlePos);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "set dst buffer read frame fail, ret %{public}d.", ret);

    return true;
}

bool AudioEndpointInner::PrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime)
{
    uint64_t nextHandlePos = curWritePos + dstSpanSizeInframe_;
    Trace prepareTrace("AudioEndpoint::PrepareNextLoop " + std::to_string(nextHandlePos));
    int64_t nextHdiReadTime = GetPredictNextReadTime(nextHandlePos);
    int64_t predictWakeupTime = nextHdiReadTime - serverAheadReadTime_;
    if (predictWakeupTime <= ClockTime::GetCurNano()) {
        wakeUpTime = ClockTime::GetCurNano() + ONE_MILLISECOND_DURATION;
        AUDIO_ERR_LOG("hdi send wrong position time");
    } else {
        wakeUpTime = predictWakeupTime;
    }

    SpanInfo *nextWriteSpan = dstAudioBuffer_->GetSpanInfo(nextHandlePos);
    CHECK_AND_RETURN_RET_LOG(nextWriteSpan != nullptr, false, "GetSpanInfo failed, can not get next write span");

    int32_t ret1 = dstAudioBuffer_->SetCurWriteFrame(nextHandlePos);
    int32_t ret2 = dstAudioBuffer_->SetCurReadFrame(nextHandlePos);
    CHECK_AND_RETURN_RET_LOG(ret1 == SUCCESS && ret2 == SUCCESS, false,
        "SetCurWriteFrame or SetCurReadFrame failed, ret1:%{public}d ret2:%{public}d", ret1, ret2);
    // handl each process buffer info
    int64_t curReadDoneTime = ClockTime::GetCurNano();
    {
        std::lock_guard<std::mutex> lock(listLock_);
        for (size_t i = 0; i < processBufferList_.size(); i++) {
            uint64_t eachCurReadPos = processBufferList_[i]->GetCurReadFrame();
            SpanInfo *tempSpan = processBufferList_[i]->GetSpanInfo(eachCurReadPos);
            CHECK_AND_RETURN_RET_LOG(tempSpan != nullptr, false,
                "GetSpanInfo failed, can not get process read span");
            SpanStatus targetStatus = SpanStatus::SPAN_READING;
            CHECK_AND_RETURN_RET_LOG(processBufferList_[i]->GetStreamStatus() != nullptr, false,
                "stream status is null");
            if (tempSpan->spanStatus.compare_exchange_strong(targetStatus, SpanStatus::SPAN_READ_DONE)) {
                tempSpan->readDoneTime = curReadDoneTime;
                BufferDesc bufferReadDone = { nullptr, 0, 0};
                processBufferList_[i]->GetReadbuffer(eachCurReadPos, bufferReadDone);
                if (bufferReadDone.buffer != nullptr && bufferReadDone.bufLength != 0) {
                    memset_s(bufferReadDone.buffer, bufferReadDone.bufLength, 0, bufferReadDone.bufLength);
                }
                processBufferList_[i]->SetCurReadFrame(eachCurReadPos + dstSpanSizeInframe_); // use client span size
            } else if (processBufferList_[i]->GetStreamStatus() &&
                processBufferList_[i]->GetStreamStatus()->load() == StreamStatus::STREAM_RUNNING) {
                AUDIO_DEBUG_LOG("Current %{public}" PRIu64" span not ready:%{public}d", eachCurReadPos, targetStatus);
            }
        }
    }
    return true;
}

bool AudioEndpointInner::GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime)
{
    Trace trace("AudioEndpoint::GetMmapHandlePosition");
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = 0;
    if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
        CHECK_AND_RETURN_RET_LOG(fastSource_ != nullptr && fastSource_->IsInited(),
            false, "Source start failed.");
        // GetMmapHandlePosition will call using ipc.
        ret = fastSource_->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    } else {
        CHECK_AND_RETURN_RET_LOG(fastSink_ != nullptr && fastSink_->IsInited(),
            false, "GetDeviceHandleInfo failed: sink is not inited.");
        // GetMmapHandlePosition will call using ipc.
        ret = fastSink_->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Call adapter GetMmapHandlePosition failed: %{public}d", ret);
    trace.End();
    nanoTime = timeNanoSec + timeSec * AUDIO_NS_PER_SECOND;
    Trace infoTrace("AudioEndpoint::GetDeviceHandleInfo frames=>" + std::to_string(frames) + " " +
        std::to_string(nanoTime) + " at " + std::to_string(ClockTime::GetCurNano()));
    nanoTime += DELTA_TO_REAL_READ_START_TIME; // global delay in server
    return true;
}

void AudioEndpointInner::AsyncGetPosTime()
{
    AUDIO_INFO_LOG("AsyncGetPosTime thread start.");
    while (!stopUpdateThread_) {
        std::unique_lock<std::mutex> lock(updateThreadLock_);
        updateThreadCV_.wait_for(lock, std::chrono::milliseconds(UPDATE_THREAD_TIMEOUT));
        if (stopUpdateThread_) {
            break;
        }
        if (endpointStatus_ == IDEL && isStarted_ && ClockTime::GetCurNano() > delayStopTime_) {
            AUDIO_INFO_LOG("IDEL for too long, let's call hdi stop");
            DelayStopDevice();
            continue;
        }
        if (endpointStatus_ == IDEL && !isStarted_) {
            continue;
        }
        // get signaled, call get pos-time
        uint64_t curHdiHandlePos = posInFrame_;
        int64_t handleTime = timeInNano_;
        if (!GetDeviceHandleInfo(curHdiHandlePos, handleTime)) {
            AUDIO_WARNING_LOG("AsyncGetPosTime call GetDeviceHandleInfo failed.");
            continue;
        }
        // keep it
        if (posInFrame_ != curHdiHandlePos) {
            posInFrame_ = curHdiHandlePos;
            timeInNano_ = handleTime;
        }
    }
}

std::string AudioEndpointInner::GetStatusStr(EndpointStatus status)
{
    switch (status) {
        case INVALID:
            return "INVALID";
        case UNLINKED:
            return "UNLINKED";
        case IDEL:
            return "IDEL";
        case STARTING:
            return "STARTING";
        case RUNNING:
            return "RUNNING";
        case STOPPING:
            return "STOPPING";
        case STOPPED:
            return "STOPPED";
        default:
            break;
    }
    return "NO_SUCH_STATUS";
}

bool AudioEndpointInner::KeepWorkloopRunning()
{
    EndpointStatus targetStatus = INVALID;
    switch (endpointStatus_.load()) {
        case RUNNING:
            return true;
        case IDEL:
            if (ClockTime::GetCurNano() > delayStopTime_) {
                targetStatus = RUNNING;
                break;
            }
            if (isDeviceRunningInIdel_) {
                return true;
            }
            break;
        case UNLINKED:
            targetStatus = IDEL;
            break;
        case STARTING:
            targetStatus = RUNNING;
            break;
        case STOPPING:
            targetStatus = STOPPED;
            break;
        default:
            break;
    }

    // when return false, EndpointWorkLoopFuc will continue loop immediately. Wait to avoid a inifity loop.
    std::unique_lock<std::mutex> lock(loopThreadLock_);
    AUDIO_PRERELEASE_LOGI("Status is %{public}s now, wait for %{public}s...", GetStatusStr(endpointStatus_).c_str(),
        GetStatusStr(targetStatus).c_str());
    threadStatus_ = WAITTING;
    workThreadCV_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME_IN_DEFAULT));
    AUDIO_DEBUG_LOG("Wait end. Cur is %{public}s now, target is %{public}s...", GetStatusStr(endpointStatus_).c_str(),
        GetStatusStr(targetStatus).c_str());

    return false;
}

int32_t AudioEndpointInner::WriteToSpecialProcBuf(const std::shared_ptr<OHAudioBuffer> &procBuf,
    const BufferDesc &readBuf, const BufferDesc &convertedBuffer, bool muteFlag)
{
    CHECK_AND_RETURN_RET_LOG(procBuf != nullptr, ERR_INVALID_HANDLE, "process buffer is null.");
    uint64_t curWritePos = procBuf->GetCurWriteFrame();
    Trace trace("AudioEndpoint::WriteProcessData-<" + std::to_string(curWritePos));

    int32_t writeAbleSize = procBuf->GetAvailableDataFrames();
    if (writeAbleSize <= 0 || static_cast<uint32_t>(writeAbleSize) <= dstSpanSizeInframe_) {
        AUDIO_WARNING_LOG("client read too slow: curWritePos:%{public}" PRIu64" writeAbleSize:%{public}d",
            curWritePos, writeAbleSize);
        return ERR_OPERATION_FAILED;
    }

    SpanInfo *curWriteSpan = procBuf->GetSpanInfo(curWritePos);
    CHECK_AND_RETURN_RET_LOG(curWriteSpan != nullptr, ERR_INVALID_HANDLE,
        "get write span info of procBuf fail.");

    AUDIO_DEBUG_LOG("process buffer write start, curWritePos %{public}" PRIu64".", curWritePos);
    curWriteSpan->spanStatus.store(SpanStatus::SPAN_WRITTING);
    curWriteSpan->writeStartTime = ClockTime::GetCurNano();

    BufferDesc writeBuf;
    int32_t ret = procBuf->GetWriteBuffer(curWritePos, writeBuf);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get write buffer fail, ret %{public}d.", ret);
    if (muteFlag) {
        memset_s(static_cast<void *>(writeBuf.buffer), writeBuf.bufLength, 0, writeBuf.bufLength);
    } else {
        if (endpointType_ == TYPE_VOIP_MMAP) {
            ret = HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
        } else {
            ret = memcpy_s(static_cast<void *>(writeBuf.buffer), writeBuf.bufLength,
                static_cast<void *>(readBuf.buffer), readBuf.bufLength);
        }
    }

    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memcpy data to process buffer fail, "
        "curWritePos %{public}" PRIu64", ret %{public}d.", curWritePos, ret);

    curWriteSpan->writeDoneTime = ClockTime::GetCurNano();
    procBuf->SetHandleInfo(curWritePos, curWriteSpan->writeDoneTime);
    ret = procBuf->SetCurWriteFrame(curWritePos + dstSpanSizeInframe_);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set procBuf next write frame fail, ret %{public}d.", ret);
        curWriteSpan->spanStatus.store(SpanStatus::SPAN_READ_DONE);
        return ERR_OPERATION_FAILED;
    }
    curWriteSpan->spanStatus.store(SpanStatus::SPAN_WRITE_DONE);
    return SUCCESS;
}

int32_t AudioEndpointInner::HandleCapturerDataParams(const BufferDesc &writeBuf, const BufferDesc &readBuf,
    const BufferDesc &convertedBuffer)
{
    if (clientConfig_.streamInfo.format == SAMPLE_S16LE && clientConfig_.streamInfo.channels == STEREO) {
        return memcpy_s(static_cast<void *>(writeBuf.buffer), writeBuf.bufLength,
            static_cast<void *>(readBuf.buffer), readBuf.bufLength);
    }
    if (clientConfig_.streamInfo.format == SAMPLE_S16LE && clientConfig_.streamInfo.channels == MONO) {
        int32_t ret = FormatConverter::S16StereoToS16Mono(readBuf, convertedBuffer);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "Convert channel from stereo to mono failed");
        ret = memcpy_s(static_cast<void *>(writeBuf.buffer), writeBuf.bufLength,
            static_cast<void *>(convertedBuffer.buffer), convertedBuffer.bufLength);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memcpy_s failed");
        ret = memset_s(static_cast<void *>(convertedBuffer.buffer), convertedBuffer.bufLength, 0,
            convertedBuffer.bufLength);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memset converted buffer to 0 failed");
        return EOK;
    }
    return ERR_NOT_SUPPORTED;
}

void AudioEndpointInner::WriteToProcessBuffers(const BufferDesc &readBuf)
{
    CheckRecordSignal(readBuf.buffer, readBuf.bufLength);
    std::lock_guard<std::mutex> lock(listLock_);
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        CHECK_AND_CONTINUE_LOG(processBufferList_[i] != nullptr,
            "process buffer %{public}zu is null.", i);
        if (processBufferList_[i]->GetStreamStatus() &&
            processBufferList_[i]->GetStreamStatus()->load() != STREAM_RUNNING) {
            AUDIO_WARNING_LOG("process buffer %{public}zu not running, stream status %{public}d.",
                i, processBufferList_[i]->GetStreamStatus()->load());
            continue;
        }

        int32_t ret = WriteToSpecialProcBuf(processBufferList_[i], readBuf, processList_[i]->GetConvertedBuffer(),
            processList_[i]->GetMuteState());
        CHECK_AND_CONTINUE_LOG(ret == SUCCESS,
            "endpoint write to process buffer %{public}zu fail, ret %{public}d.", i, ret);
        AUDIO_DEBUG_LOG("endpoint process buffer %{public}zu write success.", i);
    }
}

int32_t AudioEndpointInner::ReadFromEndpoint(uint64_t curReadPos)
{
    Trace trace("AudioEndpoint::ReadDstBuffer=<" + std::to_string(curReadPos));
    AUDIO_DEBUG_LOG("ReadFromEndpoint enter, dstAudioBuffer curReadPos %{public}" PRIu64".", curReadPos);
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_INVALID_HANDLE,
        "dst audio buffer is null.");
    SpanInfo *curReadSpan = dstAudioBuffer_->GetSpanInfo(curReadPos);
    CHECK_AND_RETURN_RET_LOG(curReadSpan != nullptr, ERR_INVALID_HANDLE,
        "get source read span info of source adapter fail.");
    curReadSpan->readStartTime = ClockTime::GetCurNano();
    curReadSpan->spanStatus.store(SpanStatus::SPAN_READING);
    BufferDesc readBuf;
    int32_t ret = dstAudioBuffer_->GetReadbuffer(curReadPos, readBuf);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get read buffer fail, ret %{public}d.", ret);
    DfxOperation(readBuf, dstStreamInfo_.format, dstStreamInfo_.channels);
    if (AudioDump::GetInstance().GetVersionType() == BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpHdi_, static_cast<void *>(readBuf.buffer), readBuf.bufLength);
        AudioCacheMgr::GetInstance().CacheData(dumpHdiName_,
            static_cast<void *>(readBuf.buffer), readBuf.bufLength);
    }
    WriteToProcessBuffers(readBuf);
    ret = memset_s(readBuf.buffer, readBuf.bufLength, 0, readBuf.bufLength);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("reset buffer fail, ret %{public}d.", ret);
    }
    curReadSpan->readDoneTime = ClockTime::GetCurNano();
    curReadSpan->spanStatus.store(SpanStatus::SPAN_READ_DONE);
    return SUCCESS;
}

void AudioEndpointInner::RecordEndpointWorkLoopFuc()
{
    ScheduleReportData(getpid(), gettid(), "audio_server");
    int64_t curTime = 0;
    uint64_t curReadPos = 0;
    int64_t wakeUpTime = ClockTime::GetCurNano();
    AUDIO_INFO_LOG("Record endpoint work loop fuc start.");
    while (isInited_.load()) {
        if (!KeepWorkloopRunning()) {
            continue;
        }
        threadStatus_ = INRUNNING;
        if (needReSyncPosition_) {
            RecordReSyncPosition();
            wakeUpTime = ClockTime::GetCurNano();
            needReSyncPosition_ = false;
            continue;
        }
        curTime = ClockTime::GetCurNano();
        Trace loopTrace("Record_loop_trace");
        if (curTime - wakeUpTime > THREE_MILLISECOND_DURATION) {
            AUDIO_WARNING_LOG("Wake up cost %{public}" PRId64" ms!", (curTime - wakeUpTime) / AUDIO_US_PER_SECOND);
        } else if (curTime - wakeUpTime > ONE_MILLISECOND_DURATION) {
            AUDIO_DEBUG_LOG("Wake up cost %{public}" PRId64" ms!", (curTime - wakeUpTime) / AUDIO_US_PER_SECOND);
        }

        curReadPos = dstAudioBuffer_->GetCurReadFrame();
        CHECK_AND_BREAK_LOG(ReadFromEndpoint(curReadPos) == SUCCESS, "read from endpoint to process service fail.");

        bool ret = RecordPrepareNextLoop(curReadPos, wakeUpTime);
        CHECK_AND_BREAK_LOG(ret, "PrepareNextLoop failed!");

        ProcessUpdateAppsUidForRecord();

        loopTrace.End();
        threadStatus_ = SLEEPING;
        ClockTime::AbsoluteSleep(wakeUpTime);
    }
}

void AudioEndpointInner::BindCore()
{
    if (coreBinded_) {
        return;
    }
    // bind cpu cores 2-7 for fast mixer
    cpu_set_t targetCpus;
    CPU_ZERO(&targetCpus);
    int32_t cpuNum = sysconf(_SC_NPROCESSORS_CONF);
    for (int32_t i = CPU_INDEX; i < cpuNum; i++) {
        CPU_SET(i, &targetCpus);
    }

    int32_t ret = sched_setaffinity(gettid(), sizeof(cpu_set_t), &targetCpus);
    if (ret != 0) {
        AUDIO_ERR_LOG("set target cpu failed, set ret: %{public}d", ret);
    }
    AUDIO_INFO_LOG("set pid: %{public}d, tid: %{public}d cpus", getpid(), gettid());
    coreBinded_ = true;
}

void AudioEndpointInner::EndpointWorkLoopFuc()
{
    BindCore();
    ScheduleReportData(getpid(), gettid(), "audio_server");
    int64_t curTime = 0;
    uint64_t curWritePos = 0;
    int64_t wakeUpTime = ClockTime::GetCurNano();
    AUDIO_INFO_LOG("Endpoint work loop fuc start");
    while (isInited_.load()) {
        if (!KeepWorkloopRunning()) {
            continue;
        }
        threadStatus_ = INRUNNING;
        curTime = ClockTime::GetCurNano();
        Trace loopTrace("AudioEndpoint::loop_trace");
        if (needReSyncPosition_) {
            ReSyncPosition();
            wakeUpTime = curTime;
            needReSyncPosition_ = false;
            continue;
        }
        if (curTime - wakeUpTime > THREE_MILLISECOND_DURATION) {
            AUDIO_WARNING_LOG("Wake up cost %{public}" PRId64" ms!", (curTime - wakeUpTime) / AUDIO_US_PER_SECOND);
        } else if (curTime - wakeUpTime > ONE_MILLISECOND_DURATION) {
            AUDIO_DEBUG_LOG("Wake up cost %{public}" PRId64" ms!", (curTime - wakeUpTime) / AUDIO_US_PER_SECOND);
        }

        // First, wake up at client may-write-done time, and check if all process write done.
        // If not, do another sleep to the possible latest write time.
        curWritePos = dstAudioBuffer_->GetCurWriteFrame();
        if (!CheckAllBufferReady(wakeUpTime, curWritePos)) {
            curTime = ClockTime::GetCurNano();
        }

        // then do mix & write to hdi buffer and prepare next loop
        if (!ProcessToEndpointDataHandle(curWritePos)) {
            AUDIO_ERR_LOG("ProcessToEndpointDataHandle failed!");
            break;
        }

        // prepare info of next loop
        if (!PrepareNextLoop(curWritePos, wakeUpTime)) {
            AUDIO_ERR_LOG("PrepareNextLoop failed!");
            break;
        }

        ProcessUpdateAppsUidForPlayback();

        loopTrace.End();
        // start sleep
        threadStatus_ = SLEEPING;
        ClockTime::AbsoluteSleep(wakeUpTime);
    }
    AUDIO_DEBUG_LOG("Endpoint work loop fuc end");
}

void AudioEndpointInner::InitLatencyMeasurement()
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = SAMPLE_S16LE;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(SAMPLE_S16LE);
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
}

void AudioEndpointInner::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
}

void AudioEndpointInner::CheckPlaySignal(uint8_t *buffer, size_t bufferSize)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    size_t byteSize = static_cast<size_t>(GetFormatByteSize(dstStreamInfo_.format));
    size_t newlyCheckedTime = bufferSize / (dstStreamInfo_.samplingRate /
        MILLISECOND_PER_SECOND) / (byteSize * sizeof(uint8_t) * dstStreamInfo_.channels);
    detectedTime_ += newlyCheckedTime;
    if (detectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_ &&
        !signalDetectAgent_->dspTimestampGot_) {
            AudioParamKey key = NONE;
            std::string condition = "debug_audio_latency_measurement";
            std::string dspTime = fastSink_->GetAudioParameter(key, condition);
            LatencyMonitor::GetInstance().UpdateDspTime(dspTime);
            LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true,
                signalDetectAgent_->lastPeakBufferTime_);
            AUDIO_INFO_LOG("LatencyMeas fastSink signal detected");
            LatencyMonitor::GetInstance().ShowTimestamp(true);
            signalDetectAgent_->dspTimestampGot_ = true;
            signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (signalDetected_) {
        AUDIO_INFO_LOG("LatencyMeas fastSink signal detected");
        detectedTime_ = 0;
    }
}

void AudioEndpointInner::CheckRecordSignal(uint8_t *buffer, size_t bufferSize)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetected_ = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (signalDetected_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        std::string dspTime = fastSource_->GetAudioParameter(key, condition);
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false,
            signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().UpdateDspTime(dspTime);
        AUDIO_INFO_LOG("LatencyMeas fastSource signal detected");
        signalDetected_ = false;
    }
}

void AudioEndpointInner::ProcessUpdateAppsUidForPlayback()
{
    std::vector<int32_t> appsUid;
    {
        std::lock_guard<std::mutex> lock(listLock_);

        appsUid.reserve(processList_.size());
        for (auto iProccessStream : processList_) {
            appsUid.push_back(iProccessStream->GetAppInfo().appUid);
        }
    }
    CHECK_AND_RETURN_LOG(fastSink_, "fastSink_ is nullptr");
    fastSink_->UpdateAppsUid(appsUid);
}

void AudioEndpointInner::ProcessUpdateAppsUidForRecord()
{
    std::vector<int32_t> appsUid;
    {
        std::lock_guard<std::mutex> lock(listLock_);

        appsUid.reserve(processList_.size());
        for (auto iProccessStream : processList_) {
            appsUid.push_back(iProccessStream->GetAppInfo().appUid);
        }
    }
    CHECK_AND_RETURN_LOG(fastSource_, "fastSource_ is nullptr");
    fastSource_->UpdateAppsUid(appsUid);
}

void AudioEndpointInner::WriterRenderStreamStandbySysEvent(uint32_t sessionId, int32_t standby)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_STANDBY,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("STREAMID", static_cast<int32_t>(sessionId));
    bean->Add("STANDBY", standby);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

uint32_t AudioEndpointInner::GetLinkedProcessCount()
{
    std::lock_guard<std::mutex> lock(listLock_);
    return processList_.size();
}
} // namespace AudioStandard
} // namespace OHOS
