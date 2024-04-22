/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "audio_renderer_sink.h"

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
#endif
#include "v2_0/iaudio_manager.h"

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
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
const uint32_t PRIMARY_OUTPUT_STREAM_ID = 13; // 13 + 0 * 8
const uint32_t STEREO_CHANNEL_COUNT = 2;
const unsigned int TIME_OUT_SECONDS = 10;
#ifdef FEATURE_POWER_MANAGER
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif
const int32_t SLEEP_TIME_FOR_RENDER_EMPTY = 300;

const int64_t SECOND_TO_NANOSECOND = 1000000000;
}
class AudioRendererSinkInner : public AudioRendererSink {
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

    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetTransactionId(uint64_t *transactionId) override;
    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice) override;

    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterParameterCallback(IAudioSinkCallback* callback) override;

    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    int32_t SetOutputRoute(DeviceType outputDevice) override;
    int32_t SetOutputRoute(DeviceType outputDevice, AudioPortPin &outputPortPin);

    int32_t Preload(const std::string &usbInfoStr) override;

    explicit AudioRendererSinkInner(const std::string &halName = "primary");
    ~AudioRendererSinkInner();
private:
    IAudioSinkAttr attr_;
    bool sinkInited_;
    bool adapterInited_;
    bool renderInited_;
    bool started_;
    bool paused_;
    float leftVolume_;
    float rightVolume_;
    int32_t routeHandle_ = -1;
    int32_t logMode_ = 0;
    uint32_t openSpeaker_;
    uint32_t renderId_ = 0;
    std::string adapterNameCase_;
    struct IAudioManager *audioManager_;
    struct IAudioAdapter *audioAdapter_;
    struct IAudioRender *audioRender_;
    std::string halName_;
    struct AudioAdapterDescriptor adapterDesc_;
    struct AudioPort audioPort_ = {};
    bool audioMonoState_ = false;
    bool audioBalanceState_ = false;
    float leftBalanceCoef_ = 1.0f;
    float rightBalanceCoef_ = 1.0f;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock_;
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
    void ReleaseRunningLock();

    FILE *dumpFile_ = nullptr;
    DeviceType currentActiveDevice_ = DEVICE_TYPE_NONE;
    AudioScene currentAudioScene_;
};

AudioRendererSinkInner::AudioRendererSinkInner(const std::string &halName)
    : sinkInited_(false), adapterInited_(false), renderInited_(false), started_(false), paused_(false),
      leftVolume_(DEFAULT_VOLUME_LEVEL), rightVolume_(DEFAULT_VOLUME_LEVEL), openSpeaker_(0),
      audioManager_(nullptr), audioAdapter_(nullptr), audioRender_(nullptr), halName_(halName)
{
    attr_ = {};
}

AudioRendererSinkInner::~AudioRendererSinkInner()
{
    AUDIO_WARNING_LOG("~AudioRendererSinkInner");
}

AudioRendererSink *AudioRendererSink::GetInstance(std::string halName)
{
    if (halName == "usb") {
        static AudioRendererSinkInner audioRendererUsb(halName);
        return &audioRendererUsb;
    }
    static AudioRendererSinkInner audioRenderer;
    return &audioRenderer;
}

static int32_t SwitchAdapterRender(struct AudioAdapterDescriptor *descs, string adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &renderPort, uint32_t size)
{
    CHECK_AND_RETURN_RET(descs != nullptr, ERROR);
    for (uint32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        if (!strcmp(desc->adapterName, adapterNameCase.c_str())) {
            for (uint32_t port = 0; port < desc->portsLen; port++) {
                // Only find out the port of out in the sound card
                if (desc->ports[port].dir == portFlag) {
                    renderPort = desc->ports[port];
                    return index;
                }
            }
        }
    }
    AUDIO_ERR_LOG("SwitchAdapterRender Fail");

    return ERR_INVALID_INDEX;
}

void AudioRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("SetAudioParameter: key %{public}d, condition: %{public}s, value: %{public}s", key,
        condition.c_str(), value.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);

    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "SetAudioParameter failed, audioAdapter_ is null");
    int32_t ret = audioAdapter_->SetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value.c_str());
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("SetAudioParameter failed, error code: %d", ret);
    }
}

std::string AudioRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
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
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, "",
        "GetAudioParameter failed, audioAdapter_ is null");
    int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value, PARAM_VALUE_LENTH);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "",
        "GetAudioParameter failed, error code: %d", ret);
    return value;
}

void AudioRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void AudioRendererSinkInner::SetAudioBalanceValue(float audioBalance)
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

int32_t AudioRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    int32_t ret;
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("failed audioRender_ is NULL");
        return ERR_INVALID_HANDLE;
    }
    struct AudioTimeStamp timestamp = {};
    ret = audioRender_->GetRenderPosition(audioRender_, &frames, &timestamp);
    if (ret != 0) {
        AUDIO_ERR_LOG("GetRenderPosition from hdi failed");
        return ERR_OPERATION_FAILED;
    }
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it;
    if (timestamp.tvSec < 0 || timestamp.tvSec > maxSec || timestamp.tvNSec < 0 ||
        timestamp.tvNSec > SECOND_TO_NANOSECOND) {
        AUDIO_ERR_LOG(
            "Hdi GetRenderPosition get invaild second:%{public}" PRIu64 " or nanosecond:%{public}" PRIu64 " !",
            timestamp.tvSec, timestamp.tvNSec);
        return ERR_OPERATION_FAILED;
    }

    timeSec = timestamp.tvSec;
    timeNanoSec = timestamp.tvNSec;
    return ret;
}

void AudioRendererSinkInner::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is surpported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT,
        "AdjustStereoToMono: Unsupported channel number: %{public}d", attr_.channel);

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
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<int8_t *>(data), len);
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

void AudioRendererSinkInner::AdjustAudioBalance(char *data, uint64_t len)
{
    // only stereo is surpported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT,
        "AdjustAudioBalance: Unsupported channel number: %{public}d", attr_.channel);

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
            AUDIO_ERR_LOG("AdjustAudioBalance: Unsupported audio format: %{public}d", attr_.format);
            break;
        }
    }
}

bool AudioRendererSinkInner::IsInited()
{
    return sinkInited_;
}

void AudioRendererSinkInner::RegisterParameterCallback(IAudioSinkCallback* callback)
{
    AUDIO_WARNING_LOG("RegisterParameterCallback not supported.");
}

void AudioRendererSinkInner::DeInit()
{
    AUDIO_INFO_LOG("DeInit.");
    started_ = false;
    sinkInited_ = false;

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyRender(audioAdapter_, renderId_);
    }
    audioRender_ = nullptr;
    renderInited_ = false;

    if (audioManager_ != nullptr) {
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;
    adapterInited_ = false;

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

void InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = PRIMARY_OUTPUT_STREAM_ID;
    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

int32_t AudioRendererSinkInner::InitAudioManager()
{
    AUDIO_INFO_LOG("Initialize audio proxy manager");

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
            AUDIO_DEBUG_LOG("Unkown format type,set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat AudioRendererSinkInner::ConvertToHdiFormat(HdiAdapterFormat format)
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

int32_t AudioRendererSinkInner::CreateRender(const struct AudioPort &renderPort)
{
    Trace trace("AudioRendererSinkInner::CreateRender");
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
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBits(param.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    AUDIO_DEBUG_LOG("Create render format: %{public}d", param.format);
    deviceDesc.portId = renderPort.portId;
    deviceDesc.desc = const_cast<char *>("");
    deviceDesc.pins = PIN_OUT_SPEAKER;
    if (halName_ == "usb") {
        deviceDesc.pins = PIN_OUT_USB_HEADSET;
    }
    ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_, &renderId_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("AudioDeviceCreateRender failed.");
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
        return ERR_NOT_STARTED;
    }

    return 0;
}

int32_t AudioRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;
    openSpeaker_ = attr_.openMicSpeaker;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);
    Trace trace("AudioRendererSinkInner::Init " + adapterNameCase_);

    int32_t ret = InitAdapter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init adapter failed");

    ret = InitRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init render failed");

    sinkInited_ = true;

    return SUCCESS;
}

int32_t AudioRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    int32_t ret;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Audio Render Handle is nullptr!");

    if (!started_) {
        AUDIO_WARNING_LOG("AudioRendererSinkInner::RenderFrame invalid state! not started");
    }

    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }

    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }

    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);

    if (inSwitch_) {
        Trace traceInSwitch("AudioRendererSinkInner::RenderFrame::inSwitch");
        writeLen = len;
        return SUCCESS;
    }
    if (renderEmptyFrameCount_ > 0) {
        Trace traceEmpty("AudioRendererSinkInner::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void*>(&data), static_cast<size_t>(len), 0,
            static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s failed");
        }
        renderEmptyFrameCount_--;
        if (renderEmptyFrameCount_ == 0) {
            switchCV_.notify_all();
        }
    }
    if (*reinterpret_cast<int8_t*>(&data) == 0) {
        Trace::Count("AudioRendererSinkInner::RenderFrame", PCM_MAYBE_SILENT);
    } else {
        Trace::Count("AudioRendererSinkInner::RenderFrame", PCM_MAYBE_NOT_SILENT);
    }
    ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t*>(&data), static_cast<uint32_t>(len),
        &writeLen);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_FAILED,
        "RenderFrame failed ret: %{public}x", ret);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_) {
        AUDIO_DEBUG_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

int32_t AudioRendererSinkInner::Start(void)
{
    AUDIO_INFO_LOG("Start.");
    Trace trace("AudioRendererSinkInner::Start");
#ifdef FEATURE_POWER_MANAGER
    AudioXCollie audioXCollie("AudioRendererSinkInner::CreateRunningLock", TIME_OUT_SECONDS);
    if (keepRunningLock_ == nullptr) {
        keepRunningLock_ = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioPrimaryBackgroundPlay",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    }

    if (keepRunningLock_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock lock");
        keepRunningLock_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING); // -1 for lasting.
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
    audioXCollie.CancelXCollieTimer();
#endif
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_RENDER_SINK_FILENAME, &dumpFile_);

    int32_t ret;
    if (!started_) {
        ret = audioRender_->Start(audioRender_);
        if (!ret) {
            started_ = true;
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Start failed!");
            return ERR_NOT_STARTED;
        }
    }

    return SUCCESS;
}

int32_t AudioRendererSinkInner::SetVolume(float left, float right)
{
    int32_t ret;
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

    ret = audioRender_->SetVolume(audioRender_, volume);
    if (ret) {
        AUDIO_WARNING_LOG("Set volume failed!");
    }

    return ret;
}

int32_t AudioRendererSinkInner::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t AudioRendererSinkInner::SetVoiceVolume(float volume)
{
    Trace trace("AudioRendererSinkInner::SetVoiceVolume");
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "SetVoiceVolume failed, audioAdapter_ is null");
    AUDIO_DEBUG_LOG("SetVoiceVolume %{public}f", volume);
    return audioAdapter_->SetVoiceVolume(audioAdapter_, volume);
}

int32_t AudioRendererSinkInner::GetLatency(uint32_t *latency)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetLatency failed audio render null");

    CHECK_AND_RETURN_RET_LOG(latency, ERR_INVALID_PARAM,
        "GetLatency failed latency null");

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

int32_t AudioRendererSinkInner::SetOutputRoute(DeviceType outputDevice)
{
    if (outputDevice == currentActiveDevice_) {
        AUDIO_INFO_LOG("SetOutputRoute output device not change");
        return SUCCESS;
    }
    AudioPortPin outputPortPin = PIN_OUT_SPEAKER;
    return SetOutputRoute(outputDevice, outputPortPin);
}

int32_t AudioRendererSinkInner::SetOutputRoute(DeviceType outputDevice, AudioPortPin &outputPortPin)
{
    Trace trace("AudioRendererSinkInner::SetOutputRoute pin " + std::to_string(outputPortPin) + " device " +
        std::to_string(outputDevice));
    currentActiveDevice_ = outputDevice;

    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetOutputPortPin(outputDevice, sink);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetOutputRoute FAILED: %{public}d", ret);

    outputPortPin = sink.ext.device.type;
    AUDIO_INFO_LOG("Output PIN is: 0x%{public}X DeviceType is %{public}d", outputPortPin, outputDevice);
    source.portId = 0;
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_MIX_TYPE;
    source.ext.mix.moduleId = 0;
    source.ext.mix.streamId = PRIMARY_OUTPUT_STREAM_ID;
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

    renderEmptyFrameCount_ = 3; // preRender 3 frames
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
    renderEmptyFrameCount_ = 3; // render 3 empty frame
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UpdateAudioRoute failed");

    return SUCCESS;
}

int32_t AudioRendererSinkInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    AUDIO_INFO_LOG("SetAudioScene scene: %{public}d, device: %{public}d",
        audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene <= AUDIO_SCENE_PHONE_CHAT,
        ERR_INVALID_PARAM, "invalid audioScene");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "SetAudioScene failed audio render handle is null!");
    if (openSpeaker_) {
        AudioPortPin audioSceneOutPort = PIN_OUT_SPEAKER;
        if (halName_ == "usb") {
            audioSceneOutPort = PIN_OUT_USB_HEADSET;
        }

        AUDIO_DEBUG_LOG("OUTPUT port is %{public}d", audioSceneOutPort);
        bool isAudioSceneUpdate = false;
        if (audioScene != currentAudioScene_) {
            struct AudioSceneDescriptor scene;
            scene.scene.id = GetAudioCategory(audioScene);
            scene.desc.pins = audioSceneOutPort;
            scene.desc.desc = const_cast<char *>("");

            int32_t ret = audioRender_->SelectScene(audioRender_, &scene);
            CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED,
                "Select scene FAILED: %{public}d", ret);
            currentAudioScene_ = audioScene;
            isAudioSceneUpdate = true;
        }

        if (activeDevice != currentActiveDevice_ ||
            (isAudioSceneUpdate &&
                (currentAudioScene_ == AUDIO_SCENE_PHONE_CALL || currentAudioScene_ == AUDIO_SCENE_PHONE_CHAT))) {
            int32_t ret = SetOutputRoute(activeDevice, audioSceneOutPort);
            if (ret < 0) {
                AUDIO_ERR_LOG("Update route FAILED: %{public}d", ret);
            }
        }
    }
    return SUCCESS;
}

int32_t AudioRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    AUDIO_DEBUG_LOG("GetTransactionId in");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetTransactionId failed audio render null");
    CHECK_AND_RETURN_RET_LOG(transactionId, ERR_INVALID_PARAM,
        "GetTransactionId failed transactionId null");

    *transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

void AudioRendererSinkInner::ReleaseRunningLock()
{
#ifdef FEATURE_POWER_MANAGER
    if (keepRunningLock_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unLock");
        keepRunningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
#endif
}

int32_t AudioRendererSinkInner::Stop(void)
{
    Trace trace("AudioRendererSinkInner::Stop");
    AUDIO_INFO_LOG("Stop.");
    int32_t ret;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Stop failed audioRender_ null");

    if (started_) {
        ret = audioRender_->Stop(audioRender_);
        if (!ret) {
            started_ = false;
            ReleaseRunningLock();
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Stop failed!");
            ReleaseRunningLock();
            return ERR_OPERATION_FAILED;
        }
    }
    ReleaseRunningLock();
    return SUCCESS;
}

int32_t AudioRendererSinkInner::Pause(void)
{
    int32_t ret;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Pause failed audioRender_ null");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Pause invalid state!");

    if (!paused_) {
        ret = audioRender_->Pause(audioRender_);
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

int32_t AudioRendererSinkInner::Resume(void)
{
    int32_t ret;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Resume failed audioRender_ null");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Resume invalid state!");

    if (paused_) {
        ret = audioRender_->Resume(audioRender_);
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

int32_t AudioRendererSinkInner::Reset(void)
{
    int32_t ret;

    if (started_ && audioRender_ != nullptr) {
        ret = audioRender_->Flush(audioRender_);
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Reset failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t AudioRendererSinkInner::Flush(void)
{
    int32_t ret;

    if (started_ && audioRender_ != nullptr) {
        ret = audioRender_->Flush(audioRender_);
        if (!ret) {
            return SUCCESS;
        } else {
            AUDIO_ERR_LOG("Flush failed!");
            return ERR_OPERATION_FAILED;
        }
    }

    return ERR_OPERATION_FAILED;
}

int32_t AudioRendererSinkInner::Preload(const std::string &usbInfoStr)
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

int32_t AudioRendererSinkInner::UpdateUsbAttrs(const std::string &usbInfoStr)
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
    attr_.sampleRate = stoi(sampleRateStr);
    attr_.channel = STEREO_CHANNEL_COUNT;
    attr_.format = ParseAudioFormat(formatStr);

    adapterNameCase_ = "usb";
    openSpeaker_ = 0;

    return SUCCESS;
}

int32_t AudioRendererSinkInner::InitAdapter()
{
    AUDIO_INFO_LOG("Init adapter start");

    if (adapterInited_) {
        AUDIO_INFO_LOG("Adapter already inited");
        return SUCCESS;
    }

    int32_t err = InitAudioManager();
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_NOT_STARTED,
        "Init audio manager Fail.");

    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, (struct AudioAdapterDescriptor *)&descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && ret == 0,
        ERR_NOT_STARTED, "Get adapters failed");

    enum AudioPortDirection port = PORT_OUT;
    int32_t index =
        SwitchAdapterRender((struct AudioAdapterDescriptor *)&descs, adapterNameCase_, port, audioPort_, size);
    CHECK_AND_RETURN_RET_LOG((index >= 0), ERR_NOT_STARTED, "Switch Adapter failed");

    adapterDesc_ = descs[index];
    CHECK_AND_RETURN_RET_LOG((audioManager_->LoadAdapter(audioManager_, &adapterDesc_, &audioAdapter_) == SUCCESS),
        ERR_NOT_STARTED, "Load Adapter Fail.");

    adapterInited_ = true;

    return SUCCESS;
}

int32_t AudioRendererSinkInner::InitRender()
{
    Trace trace("AudioRendererSinkInner::InitRender");
    AUDIO_INFO_LOG("Init render start");

    if (renderInited_) {
        AUDIO_INFO_LOG("Render already inited");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG((audioAdapter_ != nullptr), ERR_NOT_STARTED, "Audio device not loaded");

    // Initialization port information, can fill through mode and other parameters
    CHECK_AND_RETURN_RET_LOG((audioAdapter_->InitAllPorts(audioAdapter_) == SUCCESS),
        ERR_NOT_STARTED, "Init ports failed");

    int32_t err = CreateRender(audioPort_);
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_NOT_STARTED,
        "Create render failed, Audio Port: %{public}d", audioPort_.portId);

    if (openSpeaker_) {
        int32_t ret = SUCCESS;
        if (halName_ == "usb") {
            ret = SetOutputRoute(DEVICE_TYPE_USB_ARM_HEADSET);
        } else {
            ret = SetOutputRoute(DEVICE_TYPE_SPEAKER);
        }
        if (ret < 0) {
            AUDIO_WARNING_LOG("Update route FAILED: %{public}d", ret);
        }
    }

    renderInited_ = true;

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
