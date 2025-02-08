/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "OffloadAudioRendererSinkInner"
#endif

#include "offload_audio_renderer_sink.h"

#include <cstring>
#include <cinttypes>
#include <dlfcn.h>
#include <string>
#include <unistd.h>
#include <future>

#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#include "running_lock.h"
#include "audio_running_lock_manager.h"
#endif
#include "v4_0/iaudio_manager.h"

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include "audio_log_utils.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t HALF_FACTOR = 2;
const int32_t MAX_AUDIO_ADAPTER_NUM = 5;
const float DEFAULT_VOLUME_LEVEL = 1.0f;
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
const uint32_t STEREO_CHANNEL_COUNT = 2;
#ifdef FEATURE_POWER_MANAGER
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif
const uint64_t SECOND_TO_NANOSECOND = 1000000000;
const uint64_t SECOND_TO_MICROSECOND = 1000000;
const uint64_t SECOND_TO_MILLISECOND = 1000;
const uint64_t MICROSECOND_TO_MILLISECOND = 1000;
const uint32_t BIT_IN_BYTE = 8;
const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
const unsigned int TIME_OUT_SECONDS = 10;
const std::string LOG_UTILS_TAG = "Offload";
constexpr size_t OFFLOAD_DFX_SPLIT = 2;
}

struct AudioCallbackService {
    struct IAudioCallback interface;
    void *cookie;
    OnRenderCallback* renderCallback;
    void *userdata;
    bool registered = false;
};

class OffloadAudioRendererSinkInner : public OffloadRendererSink {
public:
    int32_t Init(const IAudioSinkAttr& attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Flush(void) override;
    int32_t Pause(void) override;
    int32_t Reset(void) override;
    int32_t Resume(void) override;
    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t SuspendRenderSink(void) override;
    int32_t RestoreRenderSink(void) override;
    int32_t Drain(AudioDrainType type) override;
    int32_t SetBufferSize(uint32_t sizeMs) override;

    int32_t OffloadRunningLockInit(void) override;
    int32_t OffloadRunningLockLock(void) override;
    int32_t OffloadRunningLockUnlock(void) override;

    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetVolumeInner(float &left, float &right);
    int32_t SetVoiceVolume(float volume) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetTransactionId(uint64_t *transactionId) override;
    int32_t GetAudioScene() override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;

    void SetAudioParameter(const AudioParamKey key, const std::string& condition, const std::string& value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string& condition) override;
    void RegisterParameterCallback(IAudioSinkCallback* callback) override;
    int32_t RegisterRenderCallback(OnRenderCallback (*callback), int8_t *userdata) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    static int32_t RenderEventCallback(struct IAudioCallback *self, RenderCallbackType type, int8_t *reserved,
        int8_t *cookie);

    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    void ResetOutputRouteForDisconnect(DeviceType device) override;
    float GetMaxAmplitude() override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;

    OffloadAudioRendererSinkInner();
    ~OffloadAudioRendererSinkInner();
private:
    IAudioSinkAttr attr_ = {};
    bool rendererInited_ = false;
    bool started_ = false;
    bool isFlushing_ = false;
    bool startDuringFlush_ = false;
    uint64_t renderPos_ = 0;
    float leftVolume_ = 0.0f;
    float rightVolume_ = 0.0f;
    std::mutex volumeMutex_;
    int32_t muteCount_ = 0;
    bool switchDeviceMute_ = false;
    uint32_t renderId_ = 0;
    std::string adapterNameCase_ = "";
    struct IAudioManager *audioManager_ = nullptr;
    struct IAudioAdapter *audioAdapter_ = nullptr;
    struct IAudioRender *audioRender_ = nullptr;
    struct AudioAdapterDescriptor adapterDesc_ = {};
    struct AudioPort audioPort_ = {};
    struct AudioCallbackService callbackServ = {};
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
    bool signalDetected_ = false;
    size_t detectedTime_ = 0;
    bool latencyMeasEnabled_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
    std::mutex renderMutex_;

    int32_t CreateRender(const struct AudioPort &renderPort);
    int32_t InitAudioManager();
    AudioFormat ConverToHdiFormat(HdiAdapterFormat format);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckLatencySignal(uint8_t *data, size_t len);
    void CheckUpdateState(char *frame, uint64_t replyBytes);
    void DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel, AudioSamplingRate rate) const;
 
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> offloadRunningLockManager_;
    bool runninglocked;
#endif

    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    mutable int64_t volumeDataCount_ = 0;
};
    
OffloadAudioRendererSinkInner::OffloadAudioRendererSinkInner()
    : rendererInited_(false), started_(false), isFlushing_(false), startDuringFlush_(false), renderPos_(0),
      leftVolume_(DEFAULT_VOLUME_LEVEL), rightVolume_(DEFAULT_VOLUME_LEVEL),
      audioManager_(nullptr), audioAdapter_(nullptr), audioRender_(nullptr)
{
#ifdef FEATURE_POWER_MANAGER
    runninglocked = false;
#endif
}

OffloadAudioRendererSinkInner::~OffloadAudioRendererSinkInner()
{
    AUDIO_DEBUG_LOG("~OffloadAudioRendererSinkInner");
    AUDIO_INFO_LOG("[Offload] volume data counts: %{public}" PRId64, volumeDataCount_);
}

OffloadRendererSink *OffloadRendererSink::GetInstance()
{
    static OffloadAudioRendererSinkInner audioRenderer;

    return &audioRenderer;
}

// LCOV_EXCL_START
int32_t OffloadAudioRendererSinkInner::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    AUDIO_INFO_LOG("set offload mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("offload already muted");
            return SUCCESS;
        }
        switchDeviceMute_ = true;
        float left = 0.0f;
        float right = 0.0f;
        SetVolumeInner(left, right);
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("offload not all unmuted");
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
        SetVolumeInner(leftVolume_, rightVolume_);
    }

    return SUCCESS;
}

void OffloadAudioRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string& condition,
    const std::string& value)
{
    AUDIO_INFO_LOG("key %{public}d, condition: %{public}s, value: %{public}s", key,
        condition.c_str(), value.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "SetAudioParameter failed, audioAdapter_ is null");
    int32_t ret = audioAdapter_->SetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value.c_str());
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetAudioParameter failed, error code: %{public}d", ret);
    }
}

std::string OffloadAudioRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string& condition)
{
    AUDIO_INFO_LOG("key %{public}d, condition: %{public}s", key,
        condition.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    char value[PARAM_VALUE_LENTH];
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, "", "GetAudioParameter failed, audioAdapter_ is null");
    int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value, PARAM_VALUE_LENTH);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "GetAudioParameter failed, error code: %{public}d", ret);
    return value;
}

void OffloadAudioRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void OffloadAudioRendererSinkInner::SetAudioBalanceValue(float audioBalance)
{
    // reset the balance coefficient value firstly
    leftBalanceCoef_ = 1.0f;
    rightBalanceCoef_ = 1.0f;

    if (std::abs(audioBalance - 0.0f) <= std::numeric_limits<float>::epsilon()) {
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

void OffloadAudioRendererSinkInner::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is surpported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "Unspport channel number: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8: {
            // this function needs to be further tested for usability
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        }
        case SAMPLE_S16LE: {
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        }
        case SAMPLE_S24LE: {
            // this function needs to be further tested for usability
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        }
        case SAMPLE_S32LE: {
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        }
        default: {
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("Unsupported audio format: %{public}d", attr_.format);
            break;
        }
    }
}

void OffloadAudioRendererSinkInner::AdjustAudioBalance(char *data, uint64_t len)
{
    // only stereo is surpported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "Unspport channel number: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8: {
            // this function needs to be further tested for usability
            AdjustAudioBalanceForPCM8Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        case SAMPLE_S16LE: {
            AdjustAudioBalanceForPCM16Bit(reinterpret_cast<int16_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        case SAMPLE_S24LE: {
            // this function needs to be further tested for usability
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        case SAMPLE_S32LE: {
            AdjustAudioBalanceForPCM32Bit(reinterpret_cast<int32_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        }
        default: {
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("Unsupported audio format: %{public}d", attr_.format);
            break;
        }
    }
}

bool OffloadAudioRendererSinkInner::IsInited()
{
    return rendererInited_;
}

void OffloadAudioRendererSinkInner::RegisterParameterCallback(IAudioSinkCallback* callback)
{
    AUDIO_WARNING_LOG("not supported.");
}

typedef int32_t (*RenderCallback)(struct IAudioCallback *self, enum AudioCallbackType type, int8_t* reserved,
    int8_t* cookie);

int32_t OffloadAudioRendererSinkInner::RegisterRenderCallback(OnRenderCallback (*callback), int8_t *userdata)
{
    callbackServ.renderCallback = callback;
    callbackServ.userdata = userdata;
    if (callbackServ.registered) {
        AUDIO_DEBUG_LOG("update callback");
        return SUCCESS;
    }
    // register to adapter
    auto renderCallback = (RenderCallback) &OffloadAudioRendererSinkInner::RenderEventCallback;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "audioRender_ is null");
    callbackServ.interface.RenderCallback = renderCallback;
    callbackServ.cookie = this;
    int32_t ret = audioRender_->RegCallback(audioRender_, &callbackServ.interface, (int8_t)0);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("failed, error code: %{public}d", ret);
    } else {
        callbackServ.registered = true;
    }
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::RenderEventCallback(struct IAudioCallback* self, RenderCallbackType type,
    int8_t* reserved, int8_t* cookie)
{
    // reserved and cookie should be null
    if (self == nullptr) {
        AUDIO_WARNING_LOG("self is null!");
    }
    auto *impl = reinterpret_cast<struct AudioCallbackService *>(self);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, ERROR, "The impl is null");
    if (!impl->registered || impl->cookie == nullptr || impl->renderCallback == nullptr) {
        AUDIO_ERR_LOG("impl invalid, %{public}d, %{public}d, %{public}d",
            impl->registered, impl->cookie == nullptr, impl->renderCallback == nullptr);
    }
    CHECK_AND_RETURN_RET_LOG(impl->cookie != nullptr, ERROR, "The impl->cookie is null");
    auto *sink = reinterpret_cast<OffloadAudioRendererSinkInner *>(impl->cookie);
    if (!sink->started_ || sink->isFlushing_) {
        AUDIO_DEBUG_LOG("invalid renderCallback call, started_ %d, isFlushing_ %d", sink->started_, sink->isFlushing_);
        return 0;
    }

    auto cbType = RenderCallbackType(type);
    impl->renderCallback(cbType, reinterpret_cast<int8_t*>(impl->userdata));
    return 0;
}

int32_t OffloadAudioRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    Trace trace("OffloadSink::GetPresentationPosition");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "failed! during flushing");
    int32_t ret;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "failed audioRender_ is NULL");
    uint64_t frames_;
    struct AudioTimeStamp timestamp = {};
    ret = audioRender_->GetRenderPosition(audioRender_, &frames_, &timestamp);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "offload failed");
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it;
    CHECK_AND_RETURN_RET_LOG(timestamp.tvSec >= 0 && timestamp.tvSec <= maxSec && timestamp.tvNSec >= 0 &&
        timestamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "Hdi GetRenderPosition get invaild second:%{public}" PRIu64 " or nanosecond:%{public}" PRIu64 " !",
        timestamp.tvSec, timestamp.tvNSec);
    frames = frames_ * SECOND_TO_MICROSECOND / attr_.sampleRate;
    timeSec = timestamp.tvSec;
    timeNanoSec = timestamp.tvNSec;
    // check hdi timestamp out of range 40 * 1000 * 1000 ns
    struct timespec time;
    clockid_t clockId = CLOCK_MONOTONIC;
    if (clock_gettime(clockId, &time) >= 0) {
        int64_t curNs = time.tv_sec * AUDIO_NS_PER_SECOND + time.tv_nsec;
        int64_t hdiNs = timestamp.tvSec * AUDIO_NS_PER_SECOND + timestamp.tvNSec;
        int64_t outNs = 40 * 1000 * 1000; // 40 * 1000 * 1000 ns
        if (curNs <= hdiNs || curNs > hdiNs + outNs) {
            AUDIO_PRERELEASE_LOGW("HDI time is not in the range, timestamp: %{public}" PRId64
                ", now: %{public}" PRId64, hdiNs, curNs);
            timeSec = time.tv_sec;
            timeNanoSec = time.tv_nsec;
        }
    }
    return ret;
}

void OffloadAudioRendererSinkInner::DeInit()
{
    Trace trace("OffloadSink::DeInit");
    std::lock_guard<std::mutex> lock(renderMutex_);
    std::lock_guard<std::mutex> lockVolume(volumeMutex_);
    AUDIO_INFO_LOG("DeInit.");
    started_ = false;
    rendererInited_ = false;
    if (audioAdapter_ != nullptr) {
        AUDIO_INFO_LOG("DestroyRender rendererid: %{public}u", renderId_);
        audioAdapter_->DestroyRender(audioAdapter_, renderId_);
    }
    audioRender_ = nullptr;
    audioManager_ = nullptr;
    callbackServ = {};
    muteCount_ = 0;
    switchDeviceMute_ = false;

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

void InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback*/
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_OFFLOAD));
    attrs.type = AUDIO_OFFLOAD;
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
    // AudioOffloadInfo attr
    attrs.offloadInfo.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.offloadInfo.channelCount = AUDIO_CHANNELCOUNT;
    attrs.offloadInfo.bitRate = AUDIO_SAMPLE_RATE_48K * BIT_IN_BYTE;
    attrs.offloadInfo.bitWidth = PCM_32_BIT;
}

static int32_t SwitchAdapterRender(struct AudioAdapterDescriptor *descs, const string &adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &renderPort, uint32_t size)
{
    if (descs == nullptr) {
        return ERROR;
    }
    for (uint32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        AUDIO_DEBUG_LOG("index %{public}u, adapterName %{public}s", index, desc->adapterName);
        if (strcmp(desc->adapterName, adapterNameCase.c_str())) {
            continue;
        }
        for (uint32_t port = 0; port < desc->portsLen; port++) {
            // Only find out the port of out in the sound card
            if (desc->ports[port].dir == portFlag) {
                renderPort = desc->ports[port];
                return index;
            }
        }
    }
    AUDIO_ERR_LOG("switch adapter render fail");
    return ERR_INVALID_INDEX;
}

int32_t OffloadAudioRendererSinkInner::InitAudioManager()
{
    audioManager_ = IAudioManagerGet(false);
    CHECK_AND_RETURN_RET(audioManager_ != nullptr, ERR_INVALID_HANDLE);
    return 0;
}

uint32_t PcmFormatToBits(enum AudioFormat format)
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
            AUDIO_DEBUG_LOG("Unkown format type, set it to defalut");
            return PCM_24_BIT;
    }
}

AudioFormat OffloadAudioRendererSinkInner::ConverToHdiFormat(HdiAdapterFormat format)
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
        case SAMPLE_F32:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_FLOAT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }

    return hdiFormat;
}

int32_t OffloadAudioRendererSinkInner::CreateRender(const struct AudioPort &renderPort)
{
    Trace trace("OffloadSink::CreateRender");
    int32_t ret;
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAttrs(param);
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    if (param.channelCount == MONO) {
        param.channelLayout = CH_LAYOUT_MONO;
    } else if (param.channelCount == STEREO) {
        param.channelLayout = CH_LAYOUT_STEREO;
    }
    param.format = ConverToHdiFormat(attr_.format);
    param.offloadInfo.format = ConverToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBits(param.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);

    deviceDesc.portId = renderPort.portId;
    deviceDesc.desc = const_cast<char *>("");
    deviceDesc.pins = PIN_OUT_SPEAKER;
    AudioXCollie audioXCollie("audioAdapter_->CreateRender", TIME_OUT_SECONDS);

    AUDIO_INFO_LOG("Create offload render format: %{public}d, sampleRate:%{public}u channel%{public}u",
        param.format, param.sampleRate, param.channelCount);
    ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_, &renderId_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("not started failed.");
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
        return ERR_NOT_STARTED;
    }
    AUDIO_INFO_LOG("Create success rendererid: %{public}u", renderId_);

    return 0;
}

int32_t OffloadAudioRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    Trace trace("OffloadSink::Init");
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName; // Set sound card information
    enum AudioPortDirection port = PORT_OUT; // Set port information

    CHECK_AND_RETURN_RET_LOG(InitAudioManager() == 0, ERR_NOT_STARTED, "Init audio manager Fail.");

    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    int32_t ret;
    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    if (audioManager_ == nullptr) {
        AUDIO_ERR_LOG("The audioManager is null!");
        return ERROR;
    }
    ret = audioManager_->GetAllAdapters(audioManager_, (struct AudioAdapterDescriptor *)&descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && ret == 0, ERR_NOT_STARTED,
        "Get adapters Fail.");

    // Get qualified sound card and port
    int32_t index =
        SwitchAdapterRender((struct AudioAdapterDescriptor *)&descs, adapterNameCase_, port, audioPort_, size);
    CHECK_AND_RETURN_RET_LOG(index >= 0, ERR_NOT_STARTED, "Switch Adapter Fail.");

    adapterDesc_ = descs[index];
    ret = audioManager_->LoadAdapter(audioManager_, &adapterDesc_, &audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "Load Adapter Fail.");

    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device failed.");

    // Initialization port information, can fill through mode and other parameters
    int32_t result = audioAdapter_->InitAllPorts(audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(result == 0, ERR_NOT_STARTED, "InitAllPorts failed.");

    int32_t tmp = CreateRender(audioPort_);
    CHECK_AND_RETURN_RET_LOG(tmp == 0, ERR_NOT_STARTED,
        "Create render failed, Audio Port: %{public}d", audioPort_.portId);
    rendererInited_ = true;

    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();

    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "failed! during flushing");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "failed! state not in started");
    int32_t ret;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Audio Render Handle is nullptr!");

    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }

    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }

    Trace::CountVolume("OffloadAudioRendererSinkInner::RenderFrame", static_cast<uint8_t>(data));
    Trace trace("OffloadSink::RenderFrame");
    CheckLatencySignal(reinterpret_cast<uint8_t*>(&data), len);
    ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t*>(&data), static_cast<uint32_t>(len),
        &writeLen);
    if (ret == 0 && writeLen != 0) {
        BufferDesc buffer = {reinterpret_cast<uint8_t *>(&data), len, len};
        DfxOperation(buffer, static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel),
            static_cast<AudioSamplingRate>(attr_.sampleRate));
        if (AudioDump::GetInstance().GetVersionType() == BETA_VERSION) {
            DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), writeLen);
            AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), writeLen);
        }
        CheckUpdateState(&data, len);
    }

#ifdef FEATURE_POWER_MANAGER
    offloadRunningLockManager_->UpdateAppsUidToPowerMgr();
#endif

    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_FAILED, "RenderFrameOffload failed! ret: %{public}x", ret);
    renderPos_ += writeLen;

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t stampThreshold = 50;  // 50ms
    if (stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

void OffloadAudioRendererSinkInner::DfxOperation(BufferDesc &buffer, AudioSampleFormat format,
    AudioChannel channel, AudioSamplingRate rate) const
{
    size_t byteSizePerData = VolumeTools::GetByteSize(format);
    size_t frameLen =  byteSizePerData * static_cast<size_t>(channel) * static_cast<size_t>(rate) * 0.02; // 20ms
    
    int32_t minVolume = INT_32_MAX;
    for (size_t index = 0; index < (buffer.bufLength + frameLen - 1) / frameLen; index++) {
        BufferDesc temp = {buffer.buffer + frameLen * index,
            min(buffer.bufLength - frameLen * index, frameLen), min(buffer.dataLength - frameLen * index, frameLen)};
        ChannelVolumes vols = VolumeTools::CountVolumeLevel(temp, format, channel, OFFLOAD_DFX_SPLIT);
        if (channel == MONO) {
            minVolume = min(minVolume, vols.volStart[0]);
        } else {
            minVolume = min(minVolume, (vols.volStart[0] + vols.volStart[1]) / HALF_FACTOR);
        }
        AudioLogUtils::ProcessVolumeData(LOG_UTILS_TAG, vols, volumeDataCount_);
    }
    Trace::Count(LOG_UTILS_TAG, minVolume);
}

void OffloadAudioRendererSinkInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float OffloadAudioRendererSinkInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t OffloadAudioRendererSinkInner::Start(void)
{
    Trace trace("OffloadSink::Start");
    AUDIO_INFO_LOG("Start");
    InitLatencyMeasurement();
    if (started_) {
        if (isFlushing_) {
            AUDIO_ERR_LOG("start failed! during flushing");
            startDuringFlush_ = true;
            return ERR_OPERATION_FAILED;
        } else {
            AUDIO_WARNING_LOG("start duplicate!"); // when start while flushing, this will use
            return SUCCESS;
        }
    }

    AudioXCollie audioXCollie("audioRender_->Start", TIME_OUT_SECONDS);
    int32_t ret = audioRender_->Start(audioRender_);
    if (ret) {
        AUDIO_ERR_LOG("Start failed! ret %d", ret);
        return ERR_NOT_STARTED;
    }

    dumpFileName_ = "offload_audiosink_" + std::to_string(attr_.sampleRate) + "_"
        + std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    started_ = true;
    renderPos_ = 0;
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::SetVolume(float left, float right)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    Trace trace("OffloadSink::SetVolume");

    leftVolume_ = left;
    rightVolume_ = right;
    if (switchDeviceMute_) {
        AUDIO_WARNING_LOG("switch device muted, volume in store left:%{public}f, right:%{public}f", left, right);
        return SUCCESS;
    }

    return SetVolumeInner(left, right);
}

int32_t OffloadAudioRendererSinkInner::SetVolumeInner(float &left, float &right)
{
    AUDIO_INFO_LOG("set offload vol left is %{public}f, right is %{public}f", left, right);
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "failed! during flushing");
    float thevolume;
    int32_t ret;
    if (audioRender_ == nullptr) {
        AUDIO_PRERELEASE_LOGW("OffloadAudioRendererSinkInner::SetVolume failed, audioRender_ null, "
                          "this will happen when set volume on devices which offload not available");
        return ERR_INVALID_HANDLE;
    }

    if ((left == 0) && (right != 0)) {
        thevolume = right;
    } else if ((left != 0) && (right == 0)) {
        thevolume = left;
    } else {
        thevolume = (left + right) / HALF_FACTOR;
    }

    ret = audioRender_->SetVolume(audioRender_, thevolume);
    if (ret) {
        AUDIO_ERR_LOG("Set volume failed!");
    }
    return ret;
}

int32_t OffloadAudioRendererSinkInner::GetVolume(float &left, float &right)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::SetVoiceVolume(float volume)
{
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "failed, audioAdapter_ is null");
    AUDIO_DEBUG_LOG("Set void volume %{public}f", volume);
    return audioAdapter_->SetVoiceVolume(audioAdapter_, volume);
}

int32_t OffloadAudioRendererSinkInner::GetLatency(uint32_t *latency)
{
    Trace trace("OffloadSink::GetLatency");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetLatency failed audio render null");

    CHECK_AND_RETURN_RET_LOG(latency, ERR_INVALID_PARAM,
        "GetLatency failed latency null");

    // bytewidth is 4
    uint64_t hdiLatency = renderPos_ * SECOND_TO_MICROSECOND / (AUDIO_SAMPLE_RATE_48K * 4 * STEREO_CHANNEL_COUNT);
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    CHECK_AND_RETURN_RET_LOG(GetPresentationPosition(frames, timeSec, timeNanoSec) == SUCCESS, ERR_OPERATION_FAILED,
        "get latency failed!");

    *latency = hdiLatency > frames ? (hdiLatency - frames) / MICROSECOND_TO_MILLISECOND : 0;
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    AUDIO_DEBUG_LOG("SetOutputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::GetAudioScene()
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        " failed audio render null");

    CHECK_AND_RETURN_RET_LOG(transactionId, ERR_INVALID_PARAM,
        "failed transaction Id null");

    *transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::Drain(AudioDrainType type)
{
    Trace trace("OffloadSink::Drain");
    int32_t ret;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "failed audio render null");

    ret = audioRender_->DrainBuffer(audioRender_, (AudioDrainNotifyType*)&type);
    if (!ret) {
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("DrainBuffer failed!");
        return ERR_OPERATION_FAILED;
    }

    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::Stop(void)
{
    Trace trace("OffloadSink::Stop");
    AUDIO_INFO_LOG("Stop");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "failed audio render null");

    DeinitLatencyMeasurement();

    if (started_) {
        CHECK_AND_RETURN_RET_LOG(!Flush(), ERR_OPERATION_FAILED, "Flush failed!");
        AudioXCollie audioXCollie("audioRender_->Stop", TIME_OUT_SECONDS);
        int32_t ret = audioRender_->Stop(audioRender_);
        if (!ret) {
            started_ = false;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Stop failed!");
            return ERR_OPERATION_FAILED;
        }
    }
    OffloadRunningLockUnlock();
    AUDIO_WARNING_LOG("Stop duplicate");

    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::Pause(void)
{
    AUDIO_ERR_LOG("Pause not use yet");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::Resume(void)
{
    AUDIO_ERR_LOG("Resume not use yet");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::Reset(void)
{
    Trace trace("OffloadSink::Reset");
    if (started_ && audioRender_ != nullptr) {
        startDuringFlush_ = true;
        if (!Flush()) {
            return SUCCESS;
        } else {
            startDuringFlush_ = false;
            AUDIO_ERR_LOG("Reset failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t OffloadAudioRendererSinkInner::Flush(void)
{
    Trace trace("OffloadSink::Flush");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED,
        "failed call flush during flushing");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "failed audio render null");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_INVALID_HANDLE,
        "failed state is not started");
    isFlushing_ = true;
    thread([&] {
        auto future = async(launch::async, [&] {
            std::lock_guard<std::mutex> lock(renderMutex_);
            CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
                "failed audio render null");
            return audioRender_->Flush(audioRender_);
        });
        if (future.wait_for(250ms) == future_status::timeout) { // max wait 250ms
            AUDIO_ERR_LOG("Flush failed! timeout of 250ms");
        } else {
            int32_t ret = future.get();
            if (ret) {
                AUDIO_ERR_LOG("Flush failed! ret %{public}d", ret);
            }
        }
        isFlushing_ = false;
        if (startDuringFlush_) {
            startDuringFlush_ = false;
            Start();
        }
    }).detach();
    renderPos_ = 0;
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::RestoreRenderSink(void)
{
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::SetBufferSize(uint32_t sizeMs)
{
    Trace trace("OffloadSink::SetBufferSize");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "failed! during flushing");
    int32_t ret;
    // bytewidth is 4
    uint32_t size = (uint64_t) sizeMs * AUDIO_SAMPLE_RATE_48K * 4 * STEREO_CHANNEL_COUNT / SECOND_TO_MILLISECOND;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "failed audio render null");

    ret = audioRender_->SetBufferSize(audioRender_, size);
    CHECK_AND_RETURN_RET_LOG(!ret, ERR_OPERATION_FAILED,
        "SetBufferSize failed!");

    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::OffloadRunningLockInit(void)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(offloadRunningLockManager_ == nullptr, ERR_OPERATION_FAILED,
        "OffloadKeepRunningLock is not null, init failed!");
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    WatchTimeout guard("PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock:OffloadRunningLockInit");
    keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioOffloadBackgroudPlay",
        PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    guard.CheckCurrTimeout();

    CHECK_AND_RETURN_RET_LOG(keepRunningLock != nullptr, ERR_OPERATION_FAILED, "keepRunningLock is nullptr");
    offloadRunningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);

#endif
    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::OffloadRunningLockLock(void)
{
#ifdef FEATURE_POWER_MANAGER
    AUDIO_INFO_LOG("keepRunningLock Lock");
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (offloadRunningLockManager_ == nullptr) {
        WatchTimeout guard("PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock:OffloadRunningLockLock");
        keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioOffloadBackgroudPlay",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        guard.CheckCurrTimeout();
        if (keepRunningLock) {
            offloadRunningLockManager_ =
                std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }
    CHECK_AND_RETURN_RET_LOG(offloadRunningLockManager_ != nullptr, ERR_OPERATION_FAILED,
        "offloadRunningLockManager_ is null, playback can not work well!");
    CHECK_AND_RETURN_RET(!runninglocked, SUCCESS);
    runninglocked = true;
    offloadRunningLockManager_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING); // -1 for lasting.
#endif

    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::OffloadRunningLockUnlock(void)
{
#ifdef FEATURE_POWER_MANAGER
    AUDIO_INFO_LOG("keepRunningLock UnLock");
    CHECK_AND_RETURN_RET_LOG(offloadRunningLockManager_ != nullptr, ERR_OPERATION_FAILED,
        "OffloadKeepRunningLock is null, playback can not work well!");
    CHECK_AND_RETURN_RET(runninglocked, SUCCESS);
    runninglocked = false;
    offloadRunningLockManager_->UnLock();
#endif

    return SUCCESS;
}

void OffloadAudioRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
    AUDIO_WARNING_LOG("not supported.");
}

void OffloadAudioRendererSinkInner::InitLatencyMeasurement()
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    AUDIO_INFO_LOG("LatencyMeas OffloadRendererSinkInit");
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
}

void OffloadAudioRendererSinkInner::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
}

void OffloadAudioRendererSinkInner::CheckLatencySignal(uint8_t *data, size_t len)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    size_t byteSize = static_cast<size_t>(GetFormatByteSize(attr_.format));
    size_t newlyCheckedTime = len / (attr_.sampleRate / MILLISECOND_PER_SECOND) /
        (byteSize * sizeof(uint8_t) * attr_.channel);
    detectedTime_ += newlyCheckedTime;
    if (detectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_ &&
        !signalDetectAgent_->dspTimestampGot_) {
            char value[GET_EXTRA_PARAM_LEN];
            AudioParamKey key = NONE;
            AudioExtParamKey hdiKey = AudioExtParamKey(key);
            std::string condition = "debug_audio_latency_measurement";
            int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey,
                condition.c_str(), value, GET_EXTRA_PARAM_LEN);
            AUDIO_DEBUG_LOG("GetExtraParameter ret:%{public}d", ret);
            LatencyMonitor::GetInstance().UpdateDspTime(value);
            LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true,
                signalDetectAgent_->lastPeakBufferTime_);
            LatencyMonitor::GetInstance().ShowTimestamp(true);
            signalDetectAgent_->dspTimestampGot_ = true;
            signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(data, len);
    if (signalDetected_) {
        AUDIO_INFO_LOG("LatencyMeas offloadSink signal detected");
        detectedTime_ = 0;
    }
}

int32_t OffloadAudioRendererSinkInner::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    if (!offloadRunningLockManager_) {
        return ERROR;
    }

    return offloadRunningLockManager_->UpdateAppsUid(appsUid, appsUid + size);
#endif

    return SUCCESS;
}

int32_t OffloadAudioRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported.");
    return SUCCESS;
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
