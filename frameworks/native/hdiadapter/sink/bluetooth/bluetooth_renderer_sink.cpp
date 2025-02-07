/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "BluetoothRendererSinkInner"
#endif

#include "bluetooth_renderer_sink.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <list>
#include <cinttypes>

#include <dlfcn.h>
#include <unistd.h>

#include "audio_proxy_manager.h"
#include "audio_attribute.h"
#ifdef FEATURE_POWER_MANAGER
#include "running_lock.h"
#include "power_mgr_client.h"
#include "audio_running_lock_manager.h"
#endif

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include "parameters.h"
#include "audio_log_utils.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"

using namespace std;
using namespace OHOS::HDI::Audio_Bluetooth;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t HALF_FACTOR = 2;
const int32_t MAX_AUDIO_ADAPTER_NUM = 5;
const int32_t MAX_GET_POSITOIN_TRY_COUNT = 50;
const int32_t MAX_GET_POSITION_HANDLE_TIME = 10000000; // 1000000us
const int32_t MAX_GET_POSITION_WAIT_TIME = 2000000; // 2000000us
const int32_t RENDER_FRAME_NUM = -4;
const float DEFAULT_VOLUME_LEVEL = 1.0f;
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
const uint32_t RENDER_FRAME_INTERVAL_IN_MICROSECONDS = 10000;
const uint32_t SECOND_TO_NANOSECOND = 1000000000;
const uint32_t SECOND_TO_MILLISECOND = 1000;
const uint32_t  WAIT_TIME_FOR_RETRY_IN_MICROSECOND = 50000;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
const uint32_t STEREO_CHANNEL_COUNT = 2;
constexpr uint32_t BIT_TO_BYTES = 8;
constexpr int64_t STAMP_THRESHOLD_MS = 20;
#ifdef FEATURE_POWER_MANAGER
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif
const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
}

typedef struct {
    HDI::Audio_Bluetooth::AudioFormat format;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
} BluetoothSinkAttr;

class BluetoothRendererSinkInner : public BluetoothRendererSink {
public:
    int32_t Init(const IAudioSinkAttr &attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;
    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t SuspendRenderSink(void) override;
    int32_t RestoreRenderSink(void) override;
    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetTransactionId(uint64_t *transactionId) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    int32_t SetVoiceVolume(float volume) override;
    int32_t GetAudioScene() override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;
    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterParameterCallback(IAudioSinkCallback* callback) override;
    float GetMaxAmplitude() override;

    void ResetOutputRouteForDisconnect(DeviceType device) override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    bool GetAudioMonoState();
    float GetAudioBalanceValue();

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS],
        const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t SetSinkMuteForSwitchDevice(bool mute) final;

    explicit BluetoothRendererSinkInner(bool isBluetoothLowLatency = false);
    ~BluetoothRendererSinkInner();
private:
    BluetoothSinkAttr attr_;
    bool rendererInited_;
    bool started_;
    bool paused_;
    std::atomic<bool> suspend_ = false;
    float leftVolume_;
    float rightVolume_;
    struct HDI::Audio_Bluetooth::AudioProxyManager *audioManager_;
    struct HDI::Audio_Bluetooth::AudioAdapter *audioAdapter_;
    struct HDI::Audio_Bluetooth::AudioRender *audioRender_;
    struct HDI::Audio_Bluetooth::AudioPort audioPort = {};
    void *handle_;
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
    bool signalDetected_ = false;
    bool latencyMeasEnabled_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    int32_t initCount_ = 0;
    int32_t logMode_ = 0;
    AudioSampleFormat audioSampleFormat_ = SAMPLE_S16LE;

    // for device switch
    std::mutex switchDeviceMutex_;
    int32_t muteCount_ = 0;
    std::atomic<bool> switchDeviceMute_ = false;

    // Low latency
    int32_t PrepareMmapBuffer();
    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    int32_t CheckPositionTime();

    bool isBluetoothLowLatency_ = false;
    uint32_t bufferTotalFrameSize_ = 0;
    int32_t bufferFd_ = INVALID_FD;
    uint32_t frameSizeInByte_ = 1;
    uint32_t eachReadFrameSize_ = 0;
    size_t bufferSize_ = 0;

    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
    void UnlockRunningLock();
    void UpdateAppsUid();
#endif

    int32_t CreateRender(struct HDI::Audio_Bluetooth::AudioPort &renderPort);
    int32_t InitAudioManager();
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    ConvertHdiFormat ConvertToHdiAdapterFormat(AudioFormat format);
    int64_t BytesToNanoTime(size_t lens);
    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckLatencySignal(uint8_t *data, size_t len);
    void CheckUpdateState(char *frame, uint64_t replyBytes);
    void DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel) const;
    FILE *dumpFile_ = nullptr;
    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "";
    std::string dumpFileName_ = "";
};

BluetoothRendererSinkInner::BluetoothRendererSinkInner(bool isBluetoothLowLatency)
    : rendererInited_(false), started_(false), paused_(false), suspend_(false), leftVolume_(DEFAULT_VOLUME_LEVEL),
      rightVolume_(DEFAULT_VOLUME_LEVEL), audioManager_(nullptr), audioAdapter_(nullptr),
      audioRender_(nullptr), handle_(nullptr), isBluetoothLowLatency_(isBluetoothLowLatency)
{
    attr_ = {};
}

BluetoothRendererSinkInner::~BluetoothRendererSinkInner()
{
    BluetoothRendererSinkInner::DeInit();
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
}

BluetoothRendererSink *BluetoothRendererSink::GetInstance()
{
    static BluetoothRendererSinkInner audioRenderer;

    return &audioRenderer;
}

IMmapAudioRendererSink *BluetoothRendererSink::GetMmapInstance()
{
    static BluetoothRendererSinkInner audioRenderer(true);

    return &audioRenderer;
}

bool BluetoothRendererSinkInner::IsInited(void)
{
    return rendererInited_;
}

int32_t BluetoothRendererSinkInner::SetVoiceVolume(float volume)
{
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothRendererSinkInner::GetAudioScene()
{
    AUDIO_DEBUG_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    AUDIO_DEBUG_LOG("SetOutputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

void BluetoothRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("key %{public}d, condition: %{public}s, value: %{public}s", key,
        condition.c_str(), value.c_str());
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("SetAudioParameter for render failed, audioRender_ is null");
        return;
    } else {
        int32_t ret = audioRender_->attr.SetExtraParams(reinterpret_cast<AudioHandle>(audioRender_), value.c_str());
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("SetAudioParameter for render failed, error code: %d", ret);
        }
    }
}

std::string BluetoothRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_ERR_LOG("BluetoothRendererSink GetAudioParameter not supported.");
    return "";
}

void BluetoothRendererSinkInner::RegisterParameterCallback(IAudioSinkCallback* callback)
{
    AUDIO_ERR_LOG("BluetoothRendererSink RegisterParameterCallback not supported.");
}

void BluetoothRendererSinkInner::DeInit()
{
    Trace trace("BluetoothRendererSinkInner::DeInit");
    AUDIO_INFO_LOG("DeInit.");
    if (--initCount_ > 0) {
        AUDIO_WARNING_LOG("Sink is still being used, count: %{public}d", initCount_);
        return;
    }
    started_ = false;
    rendererInited_ = false;
    if ((audioRender_ != nullptr) && (audioAdapter_ != nullptr)) {
        audioAdapter_->DestroyRender(audioAdapter_, audioRender_);
    }
    audioRender_ = nullptr;

    if ((audioManager_ != nullptr) && (audioAdapter_ != nullptr)) {
        audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;

    if (handle_ != nullptr) {
#ifndef TEST_COVERAGE
        dlclose(handle_);
#endif
        handle_ = nullptr;
    }

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

void InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = 0;
    // Bluetooth HDI use adapterNameCase to choose lowLatency / normal
    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

static int32_t SwitchAdapter(struct AudioAdapterDescriptor *descs, string adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &renderPort, int32_t size)
{
    AUDIO_INFO_LOG("SwitchAdapter: adapterNameCase: %{public}s", adapterNameCase.c_str());
    CHECK_AND_RETURN_RET(descs != nullptr, ERROR);

    for (int32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr) {
            continue;
        }
        AUDIO_DEBUG_LOG("SwitchAdapter: adapter name for %{public}d: %{public}s", index, desc->adapterName);
        if (!strcmp(desc->adapterName, adapterNameCase.c_str())) {
            for (uint32_t port = 0; port < desc->portNum; port++) {
                // Only find out the port of out in the sound card
                if (desc->ports[port].dir == portFlag) {
                    renderPort = desc->ports[port];
                    AUDIO_DEBUG_LOG("SwitchAdapter: index found %{public}d", index);
                    return index;
                }
            }
        }
    }
    AUDIO_ERR_LOG("SwitchAdapter Fail");

    return ERR_INVALID_INDEX;
}

int32_t BluetoothRendererSinkInner::InitAudioManager()
{
    AUDIO_INFO_LOG("Initialize audio proxy manager");

#if (defined(__aarch64__) || defined(__x86_64__))
    char resolvedPath[100] = "/vendor/lib64/chipsetsdk/libaudio_bluetooth_hdi_proxy_server.z.so";
#else
    char resolvedPath[100] = "/vendor/lib/chipsetsdk/libaudio_bluetooth_hdi_proxy_server.z.so";
#endif
    struct AudioProxyManager *(*getAudioManager)() = nullptr;

    handle_ = dlopen(resolvedPath, 1);
    CHECK_AND_RETURN_RET_LOG(handle_ != nullptr, ERR_INVALID_HANDLE, "Open so Fail");
    AUDIO_DEBUG_LOG("dlopen successful");

    getAudioManager = (struct AudioProxyManager *(*)())(dlsym(handle_, "GetAudioProxyManagerFuncs"));
    CHECK_AND_RETURN_RET(getAudioManager != nullptr, ERR_INVALID_HANDLE);
    AUDIO_DEBUG_LOG("getaudiomanager done");

    audioManager_ = getAudioManager();
    CHECK_AND_RETURN_RET(audioManager_ != nullptr, ERR_INVALID_HANDLE);
    AUDIO_DEBUG_LOG("audio manager created");

    return 0;
}

uint32_t PcmFormatToBits(AudioFormat format)
{
    switch (format) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            return PCM_8_BIT;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            return PCM_24_BIT;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            return PCM_32_BIT;
        default:
            return PCM_24_BIT;
    };
}

int32_t BluetoothRendererSinkInner::CreateRender(struct AudioPort &renderPort)
{
    struct AudioSampleAttributes param;
    InitAttrs(param);
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = attr_.format;
    param.frameSize = PcmFormatToBits(param.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = renderPort.portId;
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = nullptr;

    AUDIO_INFO_LOG("Create render rate:%{public}u channel:%{public}u format:%{public}u",
        param.sampleRate, param.channelCount, param.format);
    int32_t ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("AudioDeviceCreateRender failed");
        audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
        return ERR_NOT_STARTED;
    }

    return 0;
}

AudioFormat BluetoothRendererSinkInner::ConvertToHdiFormat(HdiAdapterFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }

    return hdiFormat;
}

int32_t BluetoothRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("Init: %{public}d", attr.format);
    if (rendererInited_) {
        AUDIO_WARNING_LOG("Already inited");
        initCount_++;
        return true;
    }
    audioSampleFormat_ = static_cast<AudioSampleFormat>(attr.format);

    attr_.format = ConvertToHdiFormat(attr.format);
    attr_.sampleRate = attr.sampleRate;
    attr_.channel = attr.channel;
    attr_.volume = attr.volume;

    string adapterNameCase = isBluetoothLowLatency_ ? "bt_a2dp_fast" : "bt_a2dp";  // Set sound card information
    enum AudioPortDirection port = PORT_OUT; // Set port information

    CHECK_AND_RETURN_RET_LOG(InitAudioManager() == 0, ERR_NOT_STARTED,
        "Init audio manager Fail");

    int32_t size = 0;
    struct AudioAdapterDescriptor *descs = nullptr;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, &descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && descs != nullptr && ret == 0,
        ERR_NOT_STARTED, "Get adapters Fail");

    // Get qualified sound card and port
    int32_t index = SwitchAdapter(descs, adapterNameCase, port, audioPort, size);
    CHECK_AND_RETURN_RET_LOG(index >= 0, ERR_NOT_STARTED, "Switch Adapter Fail");

    struct AudioAdapterDescriptor *desc = &descs[index];
    int32_t loadAdapter = audioManager_->LoadAdapter(audioManager_, desc, &audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(loadAdapter == 0, ERR_NOT_STARTED, "Load Adapter Fail");
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device failed");

    // Initialization port information, can fill through mode and other parameters
    ret = audioAdapter_->InitAllPorts(audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "InitAllPorts failed");

    int32_t result = CreateRender(audioPort);
    CHECK_AND_RETURN_RET_LOG(result == 0, ERR_NOT_STARTED, "Create render failed");

    if (isBluetoothLowLatency_) {
        result = PrepareMmapBuffer();
        CHECK_AND_RETURN_RET_LOG(result == 0, ERR_NOT_STARTED, "Prepare mmap buffer failed");
    }

    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);
    logUtilsTag_ = "A2dpSink";

    rendererInited_ = true;
    initCount_++;

    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int32_t ret = SUCCESS;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Bluetooth Render Handle is nullptr!");

    if (audioMonoState_) { AdjustStereoToMono(&data, len); }
    if (audioBalanceState_) { AdjustAudioBalance(&data, len); }

    CheckLatencySignal(reinterpret_cast<uint8_t*>(&data), len);
    CheckUpdateState(&data, len);
    if (suspend_) { return ret; }

    Trace trace("BluetoothRendererSinkInner::RenderFrame");
    if (switchDeviceMute_) {
        Trace traceEmpty("BluetoothRendererSinkInner::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void*>(&data), static_cast<size_t>(len), 0,
            static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s failed");
        }
    }

    BufferDesc buffer = { reinterpret_cast<uint8_t*>(&data), len, len };
    DfxOperation(buffer, audioSampleFormat_, static_cast<AudioChannel>(attr_.channel));
    if (AudioDump::GetInstance().GetVersionType() == BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), len);
    }

    while (true) {
        Trace trace("audioRender_->RenderFrame");
        int64_t stamp = ClockTime::GetCurNano();
        ret = audioRender_->RenderFrame(audioRender_, (void*)&data, len, &writeLen);
        stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
        if (logMode_ || stamp >= STAMP_THRESHOLD_MS) {
            AUDIO_PRERELEASE_LOGW("A2dp RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms " \
                "writeLen[%{public}" PRIu64 "] returns: %{public}x", len, stamp, writeLen, ret);
        }
        if (ret == RENDER_FRAME_NUM) {
            AUDIO_ERR_LOG("retry render frame...");
            usleep(RENDER_FRAME_INTERVAL_IN_MICROSECONDS);
            continue;
        }
        if (ret != 0) {
            AUDIO_ERR_LOG("A2dp RenderFrame failed ret: %{public}x", ret);
            ret = ERR_WRITE_FAILED;
        }

        break;
    }

#ifdef FEATURE_POWER_MANAGER
    UpdateAppsUid();
#endif

    return ret;
}

#ifdef FEATURE_POWER_MANAGER
void BluetoothRendererSinkInner::UpdateAppsUid()
{
    if (runningLockManager_) {
        runningLockManager_->UpdateAppsUidToPowerMgr();
    } else {
        AUDIO_ERR_LOG("runningLockManager_ is nullptr");
    }
}
#endif

void BluetoothRendererSinkInner::DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel) const
{
    ChannelVolumes vols = VolumeTools::CountVolumeLevel(buffer, format, channel);
    if (channel == MONO) {
        Trace::Count(logUtilsTag_, vols.volStart[0]);
    } else {
        Trace::Count(logUtilsTag_, (vols.volStart[0] + vols.volStart[1]) / HALF_FACTOR);
    }
    AudioLogUtils::ProcessVolumeData(logUtilsTag_, vols, volumeDataCount_);
}

ConvertHdiFormat BluetoothRendererSinkInner::ConvertToHdiAdapterFormat(AudioFormat format)
{
    ConvertHdiFormat hdiFormat;
    switch (format) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            hdiFormat = SAMPLE_U8_C;
            break;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            hdiFormat = SAMPLE_S16_C;
            break;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            hdiFormat = SAMPLE_S24_C;
            break;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            hdiFormat = SAMPLE_S32_C;
            break;
        default:
            hdiFormat = SAMPLE_S16_C;
            break;
    }

    return hdiFormat;
}

void BluetoothRendererSinkInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(ConvertToHdiAdapterFormat(attr_.format), frame, replyBytes);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float BluetoothRendererSinkInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t BluetoothRendererSinkInner::Start(void)
{
    Trace trace("BluetoothRendererSinkInner::Start");
    AUDIO_INFO_LOG("Start.");
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (runningLockManager_ == nullptr) {
        WatchTimeout guard("PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock:Start");
        keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioBluetoothBackgroundPlay",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        guard.CheckCurrTimeout();
        if (keepRunningLock) {
            runningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }

    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock lock result: %{public}d",
            runningLockManager_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING)); // -1 for lasting.
    } else {
        AUDIO_ERR_LOG("keepRunningLock is null, playback can not work well!");
    }
#endif
    dumpFileName_ = "bluetooth_audiosink_" + std::to_string(attr_.sampleRate) + "_"
        + std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    InitLatencyMeasurement();

    int32_t tryCount = 3; // try to start bluetooth render up to 3 times;
    if (!started_) {
        while (tryCount-- > 0) {
            AUDIO_INFO_LOG("Try to start bluetooth render");
            CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERROR, "Bluetooth renderer is nullptr");
            int32_t ret = audioRender_->control.Start(reinterpret_cast<AudioHandle>(audioRender_));
            if (!ret) {
                started_ = true;
                CHECK_AND_RETURN_RET_LOG(!isBluetoothLowLatency_ || CheckPositionTime() == SUCCESS,
                    ERR_NOT_STARTED, "CheckPositionTime failed!");
                return SUCCESS;
            } else {
                AUDIO_ERR_LOG("Start failed, remaining %{public}d attempt(s)", tryCount);
                usleep(WAIT_TIME_FOR_RETRY_IN_MICROSECOND);
            }
        }
        AUDIO_ERR_LOG("Start bluetooth render failed for three times, return");
#ifdef FEATURE_POWER_MANAGER
        UnlockRunningLock();
#endif
        return ERR_NOT_STARTED;
    }
    return SUCCESS;
}

#ifdef FEATURE_POWER_MANAGER
void BluetoothRendererSinkInner::UnlockRunningLock()
{
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unLock");
        runningLockManager_->UnLock();
    } else {
        AUDIO_ERR_LOG("running lock is null");
    }
}
#endif

int32_t BluetoothRendererSinkInner::CheckPositionTime()
{
    int32_t tryCount = MAX_GET_POSITOIN_TRY_COUNT;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(MAX_GET_POSITION_WAIT_TIME);
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > MAX_GET_POSITION_HANDLE_TIME) {
            AUDIO_WARNING_LOG("Try count %{public}d, ret %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("Finished.");
            return SUCCESS;
        }
    }
    return ERROR;
}

int32_t BluetoothRendererSinkInner::SetVolume(float left, float right)
{
    float volume;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "SetVolume failed audioRender_ null");

    leftVolume_ = left;
    rightVolume_ = right;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioRender_->volume.SetVolume(reinterpret_cast<AudioHandle>(audioRender_), volume);
    if (ret) {
        AUDIO_WARNING_LOG("Set volume failed!");
    }

    return ret;
}

int32_t BluetoothRendererSinkInner::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::GetLatency(uint32_t *latency)
{
    Trace trace("BluetoothRendererSinkInner::GetLatency");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetLatency failed audio render null");

    CHECK_AND_RETURN_RET_LOG(latency, ERR_INVALID_PARAM, "GetLatency failed latency null");

    uint32_t hdiLatency;
    if (audioRender_->GetLatency(audioRender_, &hdiLatency) == 0) {
        *latency = hdiLatency;
        return SUCCESS;
    } else {
        return ERR_OPERATION_FAILED;
    }
}

int32_t BluetoothRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetTransactionId failed audio render null");

    CHECK_AND_RETURN_RET_LOG(transactionId, ERR_INVALID_PARAM,
        "GetTransactionId failed transactionId null");

    *transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::Stop(void)
{
    AUDIO_INFO_LOG("in");

    Trace trace("BluetoothRendererSinkInner::Stop");

    DeinitLatencyMeasurement();
#ifdef FEATURE_POWER_MANAGER
    UnlockRunningLock();
#endif

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Stop failed audioRender_ null");

    if (started_) {
        Trace trace("audioRender_->control.Stop");
        AUDIO_DEBUG_LOG("Stop control before");
        int32_t ret = audioRender_->control.Stop(reinterpret_cast<AudioHandle>(audioRender_));
        AUDIO_DEBUG_LOG("Stop control after");
        if (!ret) {
            started_ = false;
            paused_ = false;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Stop failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::Pause(void)
{
    AUDIO_INFO_LOG("in");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Pause failed audioRender_ null");

    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Pause invalid state!");

    if (!paused_) {
        int32_t ret = audioRender_->control.Pause(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            paused_ = true;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Pause failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::Resume(void)
{
    AUDIO_INFO_LOG("in");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Resume failed audioRender_ null");

    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Resume invalid state!");

    if (paused_) {
        int32_t ret = audioRender_->control.Resume(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            paused_ = false;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Resume failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::Reset(void)
{
    AUDIO_INFO_LOG("in");

    if (started_ && audioRender_ != nullptr) {
        int32_t ret = audioRender_->control.Flush(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Reset failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t BluetoothRendererSinkInner::Flush(void)
{
    AUDIO_INFO_LOG("in");

    if (started_ && audioRender_ != nullptr) {
        int32_t ret = audioRender_->control.Flush(reinterpret_cast<AudioHandle>(audioRender_));
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Flush failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t BluetoothRendererSinkInner::SuspendRenderSink(void)
{
    AUDIO_INFO_LOG("in");
    Trace trace("BluetoothRendererSinkInner::SuspendRenderSink");
    suspend_ = true;
    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::RestoreRenderSink(void)
{
    AUDIO_INFO_LOG("in");
    Trace trace("BluetoothRendererSinkInner::RestoreRenderSink");
    suspend_ = false;
    return SUCCESS;
}

void BluetoothRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void BluetoothRendererSinkInner::SetAudioBalanceValue(float audioBalance)
{
    // reset the balance coefficient value firstly
    leftBalanceCoef_ = 1.0f;
    rightBalanceCoef_ = 1.0f;

    if (std::abs(audioBalance) <= std::numeric_limits<float>::epsilon()) {
        // audioBalance is equal to 0.0f
        audioBalanceState_ = false;
    } else {
        // audioBalance is not equal to 0.0f
        audioBalanceState_ = true;
        // calculate the balance coefficient
        if (audioBalance > 0.0f) {
            leftBalanceCoef_ -= audioBalance;
        } else if (audioBalance < 0.0f) {
            rightBalanceCoef_ += audioBalance;
        }
    }
}

int32_t BluetoothRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_ERR_LOG("BluetoothRendererSink GetPresentationPosition not supported.");
    return ERR_NOT_SUPPORTED;
}

void BluetoothRendererSinkInner::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is surpported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT,
        "AdjustStereoToMono: Unsupported channel number: %{public}d", attr_.channel);

    switch (attr_.format) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT: {
            // this function needs to be further tested for usability
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        }
        case AUDIO_FORMAT_TYPE_PCM_16_BIT: {
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        }
        case AUDIO_FORMAT_TYPE_PCM_24_BIT: {
            // this function needs to be further tested for usability
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        }
        case AUDIO_FORMAT_TYPE_PCM_32_BIT: {
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        }
        default: {
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("AdjustStereoToMono: Unsupported audio format: %{public}d",
                attr_.format);
            break;
        }
    }
}

void BluetoothRendererSinkInner::AdjustAudioBalance(char *data, uint64_t len)
{
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT,
        "Unsupported channel number: %{public}d", attr_.channel);

    switch (attr_.format) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT: {
            // this function needs to be further tested for usability
            AdjustAudioBalanceForPCM8Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        case AUDIO_FORMAT_TYPE_PCM_16_BIT: {
            AdjustAudioBalanceForPCM16Bit(reinterpret_cast<int16_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        case AUDIO_FORMAT_TYPE_PCM_24_BIT: {
            // this function needs to be further tested for usability
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        case AUDIO_FORMAT_TYPE_PCM_32_BIT: {
            AdjustAudioBalanceForPCM32Bit(reinterpret_cast<int32_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        default: {
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("Unsupported audio format: %{public}d",
                attr_.format);
            break;
        }
    }
}

void BluetoothRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
    AUDIO_WARNING_LOG("not supported.");
}

int32_t BluetoothRendererSinkInner::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothRendererSinkInner::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

static uint32_t HdiFormatToByte(HDI::Audio_Bluetooth::AudioFormat format)
{
    return PcmFormatToBits(format) / BIT_TO_BYTES;
}

int64_t BluetoothRendererSinkInner::BytesToNanoTime(size_t lens)
{
    int64_t res = static_cast<int64_t>(AUDIO_NS_PER_SECOND * lens /
        (attr_.sampleRate * attr_.channel * HdiFormatToByte(attr_.format)));
    return res;
}

int32_t BluetoothRendererSinkInner::PrepareMmapBuffer()
{
    uint32_t totalBufferInMs = 40; // 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency.
    frameSizeInByte_ = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    uint32_t reqBufferFrameSize = totalBufferInMs * (attr_.sampleRate / SECOND_TO_MILLISECOND);

    struct AudioMmapBufferDescriptor desc = {0};
    // reqBufferFrameSize means frames in total, for example, 40ms * 48K = 1920
    // transferFrameSize means frames in one block, for example 5ms per block, 5ms * 48K = 240
    int32_t ret = audioRender_->attr.ReqMmapBuffer(audioRender_, reqBufferFrameSize, &desc);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "ReqMmapBuffer failed, ret:%{public}d", ret);
    AUDIO_INFO_LOG("AudioMmapBufferDescriptor memoryAddress[%{private}p] memoryFd[%{public}d] totalBufferFrames"
        "[%{public}d] transferFrameSize[%{public}d] isShareable[%{public}d] offset[%{public}d]", desc.memoryAddress,
        desc.memoryFd, desc.totalBufferFrames, desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030,3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "ReqMmapBuffer invalid values: totalBufferFrames[%{public}d] transferFrameSize[%{public}d]",
        desc.totalBufferFrames, desc.transferFrameSize);
    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames); // 1440 ~ 3840
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize); // 240

    CHECK_AND_RETURN_RET_LOG(frameSizeInByte_ <= ULLONG_MAX / bufferTotalFrameSize_, ERR_OPERATION_FAILED,
        "BufferSize will overflow!");
    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte_;
    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
    uint32_t &byteSizePerFrame)
{
    CHECK_AND_RETURN_RET_LOG(bufferFd_ != INVALID_FD, ERR_INVALID_HANDLE, "buffer fd has been released!");
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Audio render is null!");

    struct AudioTimeStamp timestamp = {};
    int32_t ret = audioRender_->attr.GetMmapPosition(audioRender_, &frames, &timestamp);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Hdi GetMmapPosition filed, ret:%{public}d!", ret);

    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it.
    CHECK_AND_RETURN_RET_LOG(timestamp.tvSec >= 0 && timestamp.tvSec <= maxSec && timestamp.tvNSec >= 0 &&
        timestamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "Hdi GetMmapPosition get invaild second:%{public}" PRId64 " or nanosecond:%{public}" PRId64 " !",
        timestamp.tvSec, timestamp.tvNSec);
    timeSec = timestamp.tvSec;
    timeNanoSec = timestamp.tvNSec;

    return SUCCESS;
}

void BluetoothRendererSinkInner::InitLatencyMeasurement()
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    AUDIO_INFO_LOG("BlueTooth RendererSinkInit");
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
}

void BluetoothRendererSinkInner::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
}

void BluetoothRendererSinkInner::CheckLatencySignal(uint8_t *data, size_t len)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetected_ = signalDetectAgent_->CheckAudioData(data, len);
    if (signalDetected_) {
        AUDIO_INFO_LOG("LatencyMeas BTSink signal detected");
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true,
            signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().ShowBluetoothTimestamp();
    }
}

int32_t BluetoothRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    if (!runningLockManager_) {
        return ERROR;
    }

    return runningLockManager_->UpdateAppsUid(appsUid, appsUid + size);
#endif

    return SUCCESS;
}

int32_t BluetoothRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    if (!runningLockManager_) {
        return ERROR;
    }

    runningLockManager_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLockManager_->UpdateAppsUidToPowerMgr();
#endif

    return SUCCESS;
}

// LCOV_EXCL_START
int32_t BluetoothRendererSinkInner::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set a2dp mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("a2dp already muted");
            return SUCCESS;
        }
        switchDeviceMute_ = true;
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("a2dp not all unmuted");
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
    }

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
