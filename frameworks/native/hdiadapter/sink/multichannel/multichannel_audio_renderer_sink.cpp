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
#define LOG_TAG "MultiChannelRendererSinkInner"
#endif

#include "multichannel_audio_renderer_sink.h"

#include <atomic>
#include <cstring>
#include <cinttypes>
#include <condition_variable>
#include <dlfcn.h>
#include <string>
#include <unistd.h>
#include <mutex>

#include "securec.h"
#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#include "running_lock.h"
#include "audio_running_lock_manager.h"
#endif
#include "v4_0/iaudio_manager.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "parameters.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t HALF_FACTOR = 2;
const int32_t MAX_AUDIO_ADAPTER_NUM = 5;
const float DEFAULT_VOLUME_LEVEL = 1.0f;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
const uint32_t MULTICHANNEL_OUTPUT_STREAM_ID = 61; // 13 + 6 * 8
const uint32_t STEREO_CHANNEL_COUNT = 2;
const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;

#ifdef FEATURE_POWER_MANAGER
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif
const int32_t SLEEP_TIME_FOR_RENDER_EMPTY = 120;
}
class MultiChannelRendererSinkInner : public MultiChannelRendererSink {
public:
    int32_t Init(const IAudioSinkAttr &attr) override;
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

    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetTransactionId(uint64_t *transactionId) override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;

    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterParameterCallback(IAudioSinkCallback* callback) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;

    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    int32_t SetOutputRoute(DeviceType outputDevice, AudioPortPin &outputPortPin);

    int32_t Preload(const std::string &usbInfoStr) override;
    float GetMaxAmplitude() override;

    void ResetOutputRouteForDisconnect(DeviceType device) override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    explicit MultiChannelRendererSinkInner(const std::string &halName = "multichannel");
    ~MultiChannelRendererSinkInner();
private:
    IAudioSinkAttr attr_ = {};
    bool sinkInited_ = false;
    bool adapterInited_ = false;
    bool renderInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    float leftVolume_ = 0;
    float rightVolume_ = 0;
    int32_t routeHandle_ = -1;
    int32_t logMode_ = 0;
    uint32_t openSpeaker_ = 0;
    uint32_t renderId_ = 0;
    std::string adapterNameCase_ = "";
    struct IAudioManager *audioManager_ = nullptr;
    struct IAudioAdapter *audioAdapter_ = nullptr;
    struct IAudioRender *audioRender_ = nullptr;
    std::string halName_;
    struct AudioAdapterDescriptor adapterDesc_ = {};
    struct AudioPort audioPort_ = {};
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
#endif
    // for device switch
    std::atomic<bool> inSwitch_ = false;
    std::atomic<int32_t> renderEmptyFrameCount_ = 0;
    std::mutex switchMutex_;
    std::condition_variable switchCV_;

private:
    int32_t CreateRender(const struct AudioPort &renderPort);
    int32_t InitAudioManager();
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);

    int32_t UpdateUsbAttrs(const std::string &usbInfoStr);
    int32_t InitAdapter();
    int32_t InitRender();

    void CheckUpdateState(char *frame, uint64_t replyBytes);

    FILE *dumpFile_ = nullptr;
    DeviceType currentActiveDevice_ = DEVICE_TYPE_NONE;
    AudioScene currentAudioScene_ = AudioScene::AUDIO_SCENE_INVALID;
};

MultiChannelRendererSinkInner::MultiChannelRendererSinkInner(const std::string &halName)
    : sinkInited_(false), adapterInited_(false), renderInited_(false), started_(false), paused_(false),
      leftVolume_(DEFAULT_VOLUME_LEVEL), rightVolume_(DEFAULT_VOLUME_LEVEL), openSpeaker_(0),
      audioManager_(nullptr), audioAdapter_(nullptr), audioRender_(nullptr), halName_(halName)
{
    AUDIO_INFO_LOG("MultiChannelRendererSinkInner");
}

MultiChannelRendererSinkInner::~MultiChannelRendererSinkInner()
{
    AUDIO_INFO_LOG("~MultiChannelRendererSinkInner");
}

MultiChannelRendererSink *MultiChannelRendererSink::GetInstance(const std::string &halName)
{
    static MultiChannelRendererSinkInner audioRenderer;
    return &audioRenderer;
}

// LCOV_EXCL_START
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


void MultiChannelRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("SetAudioParameter: key %{public}d, condition: %{public}s, value: %{public}s", key,
        condition.c_str(), value.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    if (audioAdapter_ == nullptr) {
        AUDIO_ERR_LOG("SetAudioParameter failed, audioAdapter_ is null");
        return;
    }
    int32_t ret = audioAdapter_->SetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value.c_str());
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetAudioParameter failed, error code: %d", ret);
    }
}

std::string MultiChannelRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_INFO_LOG("GetAudioParameter: key %{public}d, condition: %{public}s", key,
        condition.c_str());
    if (condition == "get_usb_info") {
        // Init adapter to get parameter before load sink module (need fix)
        adapterNameCase_ = "usb";
        int32_t ret = InitAdapter();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "Init adapter failed for get usb info param");
    }

    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    char value[PARAM_VALUE_LENTH];
    if (audioAdapter_ == nullptr) {
        AUDIO_ERR_LOG("GetAudioParameter failed, audioAdapter_ is null");
        return "";
    }
    int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value, PARAM_VALUE_LENTH);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetAudioParameter failed, error code: %d", ret);
        return "";
    }
    return value;
}

void MultiChannelRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void MultiChannelRendererSinkInner::SetAudioBalanceValue(float audioBalance)
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

void MultiChannelRendererSinkInner::AdjustStereoToMono(char *data, uint64_t len)
{
    if (attr_.channel != STEREO_CHANNEL_COUNT) {
        // only stereo is surpported now (stereo channel count is 2)
        AUDIO_ERR_LOG("AdjustStereoToMono: Unsupported channel number: %{public}d", attr_.channel);
        return;
    }

    switch (attr_.format) {
        case SAMPLE_U8: {
            // this function needs to be further tested for usability
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        }
        case SAMPLE_S16: {
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        }
        case SAMPLE_S24: {
            // this function needs to be further tested for usability
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<uint8_t *>(data), len);
            break;
        }
        case SAMPLE_S32: {
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        }
        default: {
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("AdjustStereoToMono: Unsupported audio format: %{public}d", attr_.format);
            break;
        }
    }
}

void MultiChannelRendererSinkInner::AdjustAudioBalance(char *data, uint64_t len)
{
    if (attr_.channel != STEREO_CHANNEL_COUNT) {
        // only stereo is surpported now (stereo channel count is 2)
        AUDIO_ERR_LOG("Unsupported channel number: %{public}d", attr_.channel);
        return;
    }

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
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<uint8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
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

bool MultiChannelRendererSinkInner::IsInited()
{
    return sinkInited_;
}

void MultiChannelRendererSinkInner::RegisterParameterCallback(IAudioSinkCallback* callback)
{
    AUDIO_ERR_LOG("RegisterParameterCallback not supported.");
}

int32_t MultiChannelRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_ERR_LOG("not supported.");
    return ERR_INVALID_HANDLE;
}

void MultiChannelRendererSinkInner::DeInit()
{
    AUDIO_INFO_LOG("Mch DeInit.");
    started_ = false;
    sinkInited_ = false;

    if (audioAdapter_ != nullptr) {
        AUDIO_INFO_LOG("DestroyRender rendererid: %{public}u", renderId_);
        audioAdapter_->DestroyRender(audioAdapter_, renderId_);
    }
    audioRender_ = nullptr;
    renderInited_ = false;
    audioManager_ = nullptr;
    adapterInited_ = false;

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

void InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.channelCount = CHANNEL_6;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = MULTICHANNEL_OUTPUT_STREAM_ID;
    attrs.type = AUDIO_MULTI_CHANNEL;
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

int32_t MultiChannelRendererSinkInner::InitAudioManager()
{
    AUDIO_INFO_LOG("Initialize audio proxy manager");

    audioManager_ = IAudioManagerGet(false);
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

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
            AUDIO_INFO_LOG("Unkown format type,set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat MultiChannelRendererSinkInner::ConvertToHdiFormat(HdiAdapterFormat format)
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

int32_t MultiChannelRendererSinkInner::CreateRender(const struct AudioPort &renderPort)
{
    int32_t ret;
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAttrs(param);
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.channelLayout = attr_.channelLayout;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBits(param.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    deviceDesc.portId = renderPort.portId;
    deviceDesc.desc = const_cast<char *>("");
    deviceDesc.pins = PIN_OUT_SPEAKER;
    if (halName_ == "usb") {
        deviceDesc.pins = PIN_OUT_USB_HEADSET;
    }
    AUDIO_INFO_LOG("Create render halname: %{public}s format: %{public}d, sampleRate:%{public}u channel%{public}u",
        halName_.c_str(), param.format, param.sampleRate, param.channelCount);
    ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_, &renderId_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("AudioDeviceCreateRender failed.");
        return ERR_NOT_STARTED;
    }
    AUDIO_INFO_LOG("Create success rendererid: %{public}u", renderId_);

    return 0;
}

int32_t MultiChannelRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;
    openSpeaker_ = attr_.openMicSpeaker;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);
    int32_t ret = InitAdapter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init adapter failed");

    ret = InitRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init render failed");

    sinkInited_ = true;

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    int32_t ret;
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("Audio Render Handle is nullptr!");
        return ERR_INVALID_HANDLE;
    }

    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }

    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }

    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
    CheckUpdateState(&data, len);

    if (inSwitch_) {
        Trace traceInSwitch("AudioRendererSinkInner::RenderFrame::inSwitch");
        writeLen = len;
        return SUCCESS;
    }
    if (renderEmptyFrameCount_ > 0) {
        Trace traceEmpty("MchSinkInner::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void*>(&data), static_cast<size_t>(len), 0,
            static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s failed");
        }
        renderEmptyFrameCount_--;
        if (renderEmptyFrameCount_ == 0) {
            switchCV_.notify_all();
        }
    }
    Trace::CountVolume("MultiChannelRendererSinkInner::RenderFrame", static_cast<uint8_t>(data));
    Trace trace("MchSinkInner::RenderFrame");

    ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t*>(&data), static_cast<uint32_t>(len),
        &writeLen);
    if (ret != 0) {
        AUDIO_ERR_LOG("RenderFrame failed ret: %{public}x", ret);
        return ERR_WRITE_FAILED;
    }
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_) {
        AUDIO_DEBUG_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

void MultiChannelRendererSinkInner::CheckUpdateState(char *frame, uint64_t replyBytes)
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

float MultiChannelRendererSinkInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t MultiChannelRendererSinkInner::Start(void)
{
    Trace trace("MCHSink::Start");
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (runningLockManager_ == nullptr) {
        keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioMultiChannelBackgroundPlay",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        if (keepRunningLock) {
            runningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }

    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock lock");
        runningLockManager_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING); // -1 for lasting.
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
#endif
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_MCH_SINK_FILENAME, &dumpFile_);

    if (!started_) {
        int32_t ret = audioRender_->Start(audioRender_);
        if (ret) {
            AUDIO_ERR_LOG("Mch Start failed!");
            return ERR_NOT_STARTED;
        }
        started_ = true;
        uint64_t frameSize = 0;
        uint64_t frameCount = 0;
        ret = audioRender_->GetFrameSize(audioRender_, &frameSize);
        if (ret) {
            AUDIO_ERR_LOG("Mch GetFrameSize failed!");
            return ERR_NOT_STARTED;
        }
        ret = audioRender_->GetFrameCount(audioRender_, &frameCount);
        if (ret) {
            AUDIO_ERR_LOG("Mch GetFrameCount failed!");
            return ERR_NOT_STARTED;
        }
        ret = audioRender_->SetVolume(audioRender_, 1);
        if (ret) {
            AUDIO_ERR_LOG("Mch setvolume failed!");
            return ERR_NOT_STARTED;
        }
    }

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::SetVolume(float left, float right)
{
    int32_t ret;
    float volume;

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    leftVolume_ = left;
    rightVolume_ = right;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    ret = audioRender_->SetVolume(audioRender_, volume);
    if (ret) {
        AUDIO_ERR_LOG("Set volume failed!");
    }

    return ret;
}

int32_t MultiChannelRendererSinkInner::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::SetVoiceVolume(float volume)
{
    Trace trace("AudioRendererSinkInner::SetVoiceVolume");
    if (audioAdapter_ == nullptr) {
        AUDIO_ERR_LOG("SetVoiceVolume failed, audioAdapter_ is null");
        return ERR_INVALID_HANDLE;
    }
    AUDIO_DEBUG_LOG("SetVoiceVolume %{public}f", volume);
    return audioAdapter_->SetVoiceVolume(audioAdapter_, volume);
}

int32_t MultiChannelRendererSinkInner::GetLatency(uint32_t *latency)
{
    Trace trace("MultiChannelRendererSinkInner::GetLatency");
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("GetLatency failed audio render null");
        return ERR_INVALID_HANDLE;
    }

    if (!latency) {
        AUDIO_ERR_LOG("GetLatency failed latency null");
        return ERR_INVALID_PARAM;
    }

    uint32_t hdiLatency;
    if (audioRender_->GetLatency(audioRender_, &hdiLatency) == 0) {
        *latency = hdiLatency;
        return SUCCESS;
    } else {
        return ERR_OPERATION_FAILED;
    }
}

static AudioCategory GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AUDIO_IN_MEDIA;
            break;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_VOICE_RINGING:
            audioCategory = AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AUDIO_IN_COMMUNICATION;
            break;
        default:
            audioCategory = AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("Audio category returned is: %{public}d", audioCategory);

    return audioCategory;
}

static int32_t SetOutputPortPin(DeviceType outputDevice, AudioRouteNode &sink)
{
    int32_t ret = SUCCESS;
    switch (outputDevice) {
        case DEVICE_TYPE_EARPIECE:
            sink.ext.device.type = PIN_OUT_EARPIECE;
            sink.ext.device.desc = (char *)"pin_out_earpiece";
            break;
        case DEVICE_TYPE_SPEAKER:
            sink.ext.device.type = PIN_OUT_SPEAKER;
            sink.ext.device.desc = (char *)"pin_out_speaker";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            sink.ext.device.type = PIN_OUT_HEADSET;
            sink.ext.device.desc = (char *)"pin_out_headset";
            break;
        case DEVICE_TYPE_USB_ARM_HEADSET:
            sink.ext.device.type = PIN_OUT_USB_HEADSET;
            sink.ext.device.desc = (char *)"pin_out_usb_headset";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            sink.ext.device.type = PIN_OUT_USB_EXT;
            sink.ext.device.desc = (char *)"pin_out_usb_ext";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            sink.ext.device.type = PIN_OUT_BLUETOOTH_SCO;
            sink.ext.device.desc = (char *)"pin_out_bluetooth_sco";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            sink.ext.device.type = PIN_OUT_BLUETOOTH_A2DP;
            sink.ext.device.desc = (char *)"pin_out_bluetooth_a2dp";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t MultiChannelRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType outputDevice = outputDevices.front();
    AudioPortPin outputPortPin = PIN_OUT_SPEAKER;
    return SetOutputRoute(outputDevice, outputPortPin);
}

int32_t MultiChannelRendererSinkInner::SetOutputRoute(DeviceType outputDevice, AudioPortPin &outputPortPin)
{
    if (outputDevice == currentActiveDevice_) {
        AUDIO_INFO_LOG("SetOutputRoute output device not change");
        return SUCCESS;
    }
    currentActiveDevice_ = outputDevice;

    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetOutputPortPin(outputDevice, sink);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetOutputRoute FAILED: %{public}d", ret);

    outputPortPin = sink.ext.device.type;
    AUDIO_INFO_LOG("Output PIN is: 0x%{public}X", outputPortPin);
    source.portId = 0;
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_MIX_TYPE;
    source.ext.mix.moduleId = 0;
    source.ext.mix.streamId = MULTICHANNEL_OUTPUT_STREAM_ID;
    source.ext.device.desc = (char *)"";

    sink.portId = static_cast<int32_t>(audioPort_.portId);
    sink.role = AUDIO_PORT_SINK_ROLE;
    sink.type = AUDIO_PORT_DEVICE_TYPE;
    sink.ext.device.moduleId = 0;
    sink.ext.device.desc = (char *)"";

    AudioRoute route = {
        .sources = &source,
        .sourcesLen = 1,
        .sinks = &sink,
        .sinksLen = 1,
    };

    renderEmptyFrameCount_ = 5; // preRender 5 frames
    std::unique_lock<std::mutex> lock(switchMutex_);
    switchCV_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME_FOR_RENDER_EMPTY), [this] {
        if (renderEmptyFrameCount_ == 0) {
            AUDIO_INFO_LOG("Wait for preRender end.");
            return true;
        }
        AUDIO_DEBUG_LOG("Current renderEmptyFrameCount_ is %{public}d", renderEmptyFrameCount_.load());
        return false;
    });
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE, "SetOutputRoute failed with null adapter");
    inSwitch_.store(true);
    ret = audioAdapter_->UpdateAudioRoute(audioAdapter_, &route, &routeHandle_);
    inSwitch_.store(false);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("UpdateAudioRoute cost[%{public}" PRId64 "]ms", stamp);
    renderEmptyFrameCount_ = 5; // render 5 empty frame
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UpdateAudioRoute failed");

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType activeDevice = activeDevices.front();
    AUDIO_INFO_LOG("SetAudioScene scene: %{public}d, device: %{public}d", audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "invalid audioScene");
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("SetAudioScene failed audio render handle is null!");
        return ERR_INVALID_HANDLE;
    }
    if (openSpeaker_) {
        AudioPortPin audioSceneOutPort = PIN_OUT_SPEAKER;
        if (halName_ == "usb") {
            audioSceneOutPort = PIN_OUT_USB_HEADSET;
        }

        AUDIO_DEBUG_LOG("OUTPUT port is %{public}d", audioSceneOutPort);
        int32_t ret = SUCCESS;
        if (audioScene != currentAudioScene_) {
            struct AudioSceneDescriptor scene;
            scene.scene.id = GetAudioCategory(audioScene);
            scene.desc.pins = audioSceneOutPort;
            scene.desc.desc = (char *)"";

            ret = audioRender_->SelectScene(audioRender_, &scene);
            if (ret < 0) {
                AUDIO_ERR_LOG("Select scene FAILED: %{public}d", ret);
                return ERR_OPERATION_FAILED;
            }
            currentAudioScene_ = audioScene;
        }

        ret = SetOutputRoute(activeDevice, audioSceneOutPort);
        if (ret < 0) {
            AUDIO_ERR_LOG("Update route FAILED: %{public}d", ret);
        }
    }
    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    AUDIO_INFO_LOG("MultiChannelRendererSinkInner::GetTransactionId");

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("GetTransactionId failed audio render null");
        return ERR_INVALID_HANDLE;
    }

    if (!transactionId) {
        AUDIO_ERR_LOG("GetTransactionId failed transactionId null");
        return ERR_INVALID_PARAM;
    }

    *transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::Stop(void)
{
    Trace trace("MCHSink::Stop");
    AUDIO_INFO_LOG("Stop.");
#ifdef FEATURE_POWER_MANAGER
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unLock");
        runningLockManager_->UnLock();
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
#endif

    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("Stop failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    if (started_) {
        int32_t ret = audioRender_->Stop(audioRender_);
        if (!ret) {
            started_ = false;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Stop failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::Pause(void)
{
    Trace trace("MCHSink::Pause");
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("Pause failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    if (!started_) {
        AUDIO_ERR_LOG("Pause invalid state!");
        return ERR_OPERATION_FAILED;
    }

    if (!paused_) {
        int32_t ret = audioRender_->Pause(audioRender_);
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

int32_t MultiChannelRendererSinkInner::Resume(void)
{
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("Resume failed audioRender_ null");
        return ERR_INVALID_HANDLE;
    }

    if (!started_) {
        AUDIO_ERR_LOG("Resume invalid state!");
        return ERR_OPERATION_FAILED;
    }

    if (paused_) {
        int32_t ret = audioRender_->Resume(audioRender_);
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

int32_t MultiChannelRendererSinkInner::Reset(void)
{
    if (started_ && audioRender_ != nullptr) {
        int32_t ret = audioRender_->Flush(audioRender_);
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Reset failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t MultiChannelRendererSinkInner::Flush(void)
{
    Trace trace("MCHSink::Flush");
    if (started_ && audioRender_ != nullptr) {
        int32_t ret = audioRender_->Flush(audioRender_);
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Flush failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t MultiChannelRendererSinkInner::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::RestoreRenderSink(void)
{
    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::Preload(const std::string &usbInfoStr)
{
    CHECK_AND_RETURN_RET_LOG(halName_ == "usb", ERR_INVALID_OPERATION, "Preload only supported for usb");

    int32_t ret = UpdateUsbAttrs(usbInfoStr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Preload failed when init attr");

    ret = InitAdapter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Preload failed when init adapter");

    ret = InitRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Preload failed when init render");

    return SUCCESS;
}

static HdiAdapterFormat ParseAudioFormat(const std::string &format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return HdiAdapterFormat::SAMPLE_S16;
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT") {
        return HdiAdapterFormat::SAMPLE_S24;
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return HdiAdapterFormat::SAMPLE_S32;
    } else {
        return HdiAdapterFormat::SAMPLE_S16;
    }
}

int32_t MultiChannelRendererSinkInner::UpdateUsbAttrs(const std::string &usbInfoStr)
{
    CHECK_AND_RETURN_RET_LOG(usbInfoStr != "", ERR_INVALID_PARAM, "usb info string error");

    auto sinkRate_begin = usbInfoStr.find("sink_rate:");
    auto sinkRate_end = usbInfoStr.find_first_of(";", sinkRate_begin);
    std::string sampleRateStr = usbInfoStr.substr(sinkRate_begin + std::strlen("sink_rate:"),
        sinkRate_end - sinkRate_begin - std::strlen("sink_rate:"));
    auto sinkFormat_begin = usbInfoStr.find("sink_format:");
    auto sinkFormat_end = usbInfoStr.find_first_of(";", sinkFormat_begin);
    std::string formatStr = usbInfoStr.substr(sinkFormat_begin + std::strlen("sink_format:"),
        sinkFormat_end - sinkFormat_begin - std::strlen("sink_format:"));

    // usb default config
    attr_.sampleRate = static_cast<uint32_t>((stoi(sampleRateStr)));
    attr_.channel = STEREO_CHANNEL_COUNT;
    attr_.format = ParseAudioFormat(formatStr);

    adapterNameCase_ = "usb";
    openSpeaker_ = 0;

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::InitAdapter()
{
    AUDIO_INFO_LOG("MultiChannelRendererSinkInner::InitAdapter");

    if (adapterInited_) {
        AUDIO_INFO_LOG("Adapter already inited");
        return SUCCESS;
    }

    if (InitAudioManager() != 0) {
        AUDIO_ERR_LOG("Init audio manager Fail.");
        return ERR_NOT_STARTED;
    }

    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    if (audioManager_ == nullptr) {
        AUDIO_ERR_LOG("The audioManager is nullptr.");
        return ERROR;
    }
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, (struct AudioAdapterDescriptor *)&descs, &size);
    if (size > MAX_AUDIO_ADAPTER_NUM || size == 0 || ret != 0) {
        AUDIO_ERR_LOG("Get adapters failed");
        return ERR_NOT_STARTED;
    }

    enum AudioPortDirection port = PORT_OUT;
    int32_t index =
        SwitchAdapterRender((struct AudioAdapterDescriptor *)&descs, "primary", port, audioPort_, size);
    CHECK_AND_RETURN_RET_LOG((index >= 0), ERR_NOT_STARTED, "Switch Adapter failed");

    adapterDesc_ = descs[index];
    CHECK_AND_RETURN_RET_LOG((audioManager_->LoadAdapter(audioManager_, &adapterDesc_, &audioAdapter_) == SUCCESS),
        ERR_NOT_STARTED, "Load Adapter Fail.");

    adapterInited_ = true;

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::InitRender()
{
    AUDIO_INFO_LOG("MultiChannelRendererSinkInner::InitRender");

    if (renderInited_) {
        AUDIO_INFO_LOG("Render already inited");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG((audioAdapter_ != nullptr), ERR_NOT_STARTED, "Audio device not loaded");

    // Initialization port information, can fill through mode and other parameters
    CHECK_AND_RETURN_RET_LOG((audioAdapter_->InitAllPorts(audioAdapter_) == SUCCESS),
        ERR_NOT_STARTED, "Init ports failed");

    if (CreateRender(audioPort_) != 0) {
        AUDIO_ERR_LOG("Create render failed, Audio Port: %{public}d", audioPort_.portId);
        return ERR_NOT_STARTED;
    }

    renderInited_ = true;

    return SUCCESS;
}

void MultiChannelRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
    if (currentActiveDevice_ == device) {
        currentActiveDevice_ = DEVICE_TYPE_NONE;
    }
}

int32_t MultiChannelRendererSinkInner::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t MultiChannelRendererSinkInner::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

int32_t MultiChannelRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    if (!runningLockManager_) {
        return ERROR;
    }

    return runningLockManager_->UpdateAppsUid(appsUid, appsUid + size);
#endif

    return SUCCESS;
}

int32_t MultiChannelRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported.");
    return SUCCESS;
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
