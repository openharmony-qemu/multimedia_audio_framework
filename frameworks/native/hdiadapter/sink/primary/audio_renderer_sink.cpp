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
#ifndef LOG_TAG
#define LOG_TAG "AudioRendererSinkInner"
#endif

#include "audio_renderer_sink.h"

#include <atomic>
#include <cstring>
#include <cinttypes>
#include <condition_variable>
#include <dlfcn.h>
#include <string>
#include <unistd.h>
#include <mutex>
#include "ctime"
#include <thread>

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
#include "media_monitor_manager.h"

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
const uint32_t DIRECT_OUTPUT_STREAM_ID = 69;  // 13 + 7 * 8
const uint32_t VOIP_OUTPUT_STREAM_ID = 77;    // 13 + 8 * 8
const uint32_t STEREO_CHANNEL_COUNT = 2;
const unsigned int BUFFER_CALC_20MS = 20;
const unsigned int BUFFER_CALC_1000MS = 1000;
const unsigned int FORMAT_1_BYTE = 1;
const unsigned int FORMAT_2_BYTE = 2;
const unsigned int FORMAT_3_BYTE = 3;
const unsigned int FORMAT_4_BYTE = 4;
#ifdef FEATURE_POWER_MANAGER
const unsigned int TIME_OUT_SECONDS = 10;
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif

const int64_t SECOND_TO_NANOSECOND = 1000000000;

static int32_t g_paStatus = 1;
const double INTREVAL = 3.0;

const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
const uint32_t DEVICE_PARAM_MAX_LEN = 40;

const std::string VOIP_HAL_NAME = "voip";
const std::string DIRECT_HAL_NAME = "direct";
const std::string PRIMARY_HAL_NAME = "primary";
#ifdef FEATURE_POWER_MANAGER
const std::string PRIMARY_LOCK_NAME_BASE = "AudioBackgroundPlay";
#endif
}

int32_t ConvertByteToAudioFormat(int32_t format)
{
    int32_t audioSampleFormat = 0;
    switch (format) {
        case FORMAT_1_BYTE:
            audioSampleFormat = SAMPLE_U8;
            break;
        case FORMAT_2_BYTE:
            audioSampleFormat = SAMPLE_S16LE;
            break;
        case FORMAT_3_BYTE:
            audioSampleFormat = SAMPLE_S24LE;
            break;
        case FORMAT_4_BYTE:
            audioSampleFormat = SAMPLE_S32LE;
            break;
        default:
            audioSampleFormat = SAMPLE_S16LE;
    }
    return audioSampleFormat;
}

static string ParseAudioFormatToStr(int32_t format)
{
    switch (format) {
        case FORMAT_1_BYTE:
            return "u8";
        case FORMAT_2_BYTE:
            return "s16";
        case FORMAT_3_BYTE:
            return "s24";
        case FORMAT_4_BYTE:
            return "s32";
        default:
            return "s16";
    }
    return "";
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

    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    int32_t SetOutputRoutes(std::vector<std::pair<DeviceType, AudioPortPin>> &outputDevices);

    int32_t Preload(const std::string &usbInfoStr) override;

    void ResetOutputRouteForDisconnect(DeviceType device) override;
    float GetMaxAmplitude() override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS],
        const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t SetSinkMuteForSwitchDevice(bool mute) final;

    std::string GetDPDeviceAttrInfo(const std::string &condition);

    explicit AudioRendererSinkInner(const std::string &halName = "primary");
    ~AudioRendererSinkInner();
private:
    IAudioSinkAttr attr_ = {};
    bool sinkInited_ = false;
    bool adapterInited_ = false;
    bool renderInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    float leftVolume_ = 0.0f;
    float rightVolume_ = 0.0f;
    int32_t routeHandle_ = -1;
    int32_t logMode_ = 0;
    uint32_t openSpeaker_ = 0;
    uint32_t renderId_ = 0;
    std::string adapterNameCase_ = "";
    struct IAudioManager *audioManager_ = nullptr;
    struct IAudioAdapter *audioAdapter_ = nullptr;
    struct IAudioRender *audioRender_ = nullptr;
    const std::string halName_ = "";
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
    bool signalDetected_ = false;
    size_t detectedTime_ = 0;
    bool latencyMeasEnabled_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    time_t startTime = time(nullptr);
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
#endif
    std::string audioAttrInfo_ = "";

    // for device switch
    std::mutex switchDeviceMutex_;
    int32_t muteCount_ = 0;
    std::atomic<bool> switchDeviceMute_ = false;

private:
    int32_t CreateRender(const struct AudioPort &renderPort);
    int32_t InitAudioManager();
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    void AdjustStereoToMono(char *data, uint64_t len);
    void AdjustAudioBalance(char *data, uint64_t len);
    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckLatencySignal(uint8_t *data, size_t len);

    int32_t UpdateUsbAttrs(const std::string &usbInfoStr);
    int32_t InitAdapter();
    int32_t InitRender();
    void ReleaseRunningLock();
    void CheckUpdateState(char *frame, uint64_t replyBytes);

    int32_t UpdateDPAttrs(const std::string &usbInfoStr);
    bool AttributesCheck(AudioSampleAttributes &attrInfo);
    int32_t SetAudioAttrInfo(AudioSampleAttributes &attrInfo);
    std::string GetAudioAttrInfo();
    int32_t GetCurDeviceParam(char *keyValueList, size_t len);

    AudioPortPin GetAudioPortPin() const noexcept;
    int32_t SetAudioRoute(DeviceType outputDevice, AudioRoute route);

    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    DeviceType currentActiveDevice_ = DEVICE_TYPE_NONE;
    AudioScene currentAudioScene_ = AUDIO_SCENE_INVALID;
    int32_t currentDevicesSize_ = 0;
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
    } else if (halName == "dp") {
        static AudioRendererSinkInner audioRendererDp(halName);
        return &audioRendererDp;
    } else if (halName == VOIP_HAL_NAME) {
        static AudioRendererSinkInner audioRendererVoip(halName);
        return &audioRendererVoip;
    } else if (halName == DIRECT_HAL_NAME) {
        static AudioRendererSinkInner audioRendererDirect(halName);
        return &audioRendererDirect;
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
        AUDIO_INFO_LOG("size: %{public}d, adapterNameCase %{public}s, adapterName %{public}s",
            size, adapterNameCase.c_str(), desc->adapterName);
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

bool AudioRendererSinkInner::AttributesCheck(AudioSampleAttributes &attrInfo)
{
    CHECK_AND_RETURN_RET_LOG(attrInfo.sampleRate > 0, false, "rate failed %{public}d", attrInfo.sampleRate);
    CHECK_AND_RETURN_RET_LOG(attrInfo.format > 0, false, "format failed %{public}d", attrInfo.format);
    CHECK_AND_RETURN_RET_LOG(attrInfo.channelCount > 0, false, "channel failed %{public}d", attrInfo.channelCount);
    return true;
}

int32_t AudioRendererSinkInner::SetAudioAttrInfo(AudioSampleAttributes &attrInfo)
{
    CHECK_AND_RETURN_RET_LOG(AttributesCheck(attrInfo), ERROR, "AttributesCheck failed");
    uint32_t bufferSize = attrInfo.sampleRate * attrInfo.format * attrInfo.channelCount *
        BUFFER_CALC_20MS / BUFFER_CALC_1000MS;
    audioAttrInfo_ = "rate="+to_string(attrInfo.sampleRate)+" format=" + ParseAudioFormatToStr(attrInfo.format) +
        " channels=" + to_string(attrInfo.channelCount) + " buffer_size="+to_string(bufferSize);
    AUDIO_INFO_LOG("audio attributes %{public}s ", audioAttrInfo_.c_str());
    return SUCCESS;
}

std::string AudioRendererSinkInner::GetAudioAttrInfo()
{
    return audioAttrInfo_;
}

std::string AudioRendererSinkInner::GetDPDeviceAttrInfo(const std::string &condition)
{
    int32_t ret = UpdateDPAttrs(condition);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "GetDPDeviceAttrInfo failed when init attr");

    adapterNameCase_ = "dp";
    ret = InitAdapter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "GetDPDeviceAttrInfo failed when init adapter");

    ret = InitRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "GetDPDeviceAttrInfo failed when init render");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, "", "GetDPDeviceAttrInfo failed when audioRender_ is null");
    struct AudioSampleAttributes attrInfo = {};
    ret = audioRender_->GetSampleAttributes(audioRender_, &attrInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "GetDPDeviceAttrInfo failed when GetSampleAttributes");
    AUDIO_DEBUG_LOG("GetSampleAttributes: sampleRate %{public}d, format: %{public}d, channelCount: %{public}d," \
        "size: %{public}d", attrInfo.sampleRate, attrInfo.format, attrInfo.channelCount, attrInfo.frameSize);
    ret = SetAudioAttrInfo(attrInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "SetAudioAttrInfo failed when SetAudioAttrInfo");

    return GetAudioAttrInfo();
}

std::string AudioRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_INFO_LOG("GetAudioParameter: key %{public}d, condition: %{public}s, halName: %{public}s",
        key, condition.c_str(), halName_.c_str());
    if (condition == "get_usb_info") {
        // Init adapter to get parameter before load sink module (need fix)
        adapterNameCase_ = "usb";
        int32_t ret = InitAdapter();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "Init adapter failed for get usb info param");
    }
    if (key == AudioParamKey::GET_DP_DEVICE_INFO) {
        // Init adapter and render to get parameter before load sink module (need fix)
        return GetDPDeviceAttrInfo(condition);
    }

    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    char value[PARAM_VALUE_LENTH];
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, "", "GetAudioParameter failed, audioAdapter_ is null");
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
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("failed audioRender_ is NULL");
        return ERR_INVALID_HANDLE;
    }
    struct AudioTimeStamp timestamp = {};
    int32_t ret = audioRender_->GetRenderPosition(audioRender_, &frames, &timestamp);
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
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        }
        case SAMPLE_S16: {
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        }
        case SAMPLE_S24: {
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
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<uint8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
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
        AUDIO_INFO_LOG("DestroyRender rendererid: %{public}u", renderId_);
        audioAdapter_->DestroyRender(audioAdapter_, renderId_);
    }
    audioRender_ = nullptr;
    renderInited_ = false;

    if (audioManager_ != nullptr) {
        AUDIO_INFO_LOG("UnloadAdapter");
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
    if (halName_ == "dp") {
        param.type = AUDIO_DP;
    } else if (halName_ == DIRECT_HAL_NAME) {
        param.type = AUDIO_DIRECT;
        param.streamId = DIRECT_OUTPUT_STREAM_ID;
    } else if (halName_ == VOIP_HAL_NAME) {
        param.type = AUDIO_IN_COMMUNICATION;
        param.streamId = VOIP_OUTPUT_STREAM_ID;
    }
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBits(param.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    deviceDesc.portId = renderPort.portId;
    deviceDesc.desc = const_cast<char *>(attr_.address.c_str());
    deviceDesc.pins = PIN_OUT_SPEAKER;
    if (halName_ == "usb") {
        deviceDesc.pins = PIN_OUT_USB_HEADSET;
    } else if (halName_ == "dp") {
        deviceDesc.pins = PIN_OUT_DP;
    } else {
        deviceDesc.pins = GetAudioPortPin();
    }

    AUDIO_INFO_LOG("Create render sinkName:%{public}s, rate:%{public}u channel:%{public}u format:%{public}u, " \
        "devicePin:%{public}u",
        halName_.c_str(), param.sampleRate, param.channelCount, param.format, deviceDesc.pins);
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "CreateRender failed, audioAdapter_ is null");
    int32_t ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_, &renderId_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("AudioDeviceCreateRender failed.");
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
        adapterInited_ = false;
        return ERR_NOT_STARTED;
    }
    AUDIO_INFO_LOG("Create success rendererid: %{public}u desc: %{public}s", renderId_, deviceDesc.desc);

    return 0;
}

int32_t AudioRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;
    AUDIO_INFO_LOG("adapterNameCase_ :%{public}s", adapterNameCase_.c_str());
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
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Audio Render Handle is nullptr!");

    if (!started_) {
        AUDIO_WARNING_LOG("AudioRendererSinkInner::RenderFrame invalid state! not started");
    }

    if (audioMonoState_) {AdjustStereoToMono(&data, len);}

    if (audioBalanceState_) {AdjustAudioBalance(&data, len);}

    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
    if (AudioDump::GetInstance().GetVersionType() == BETA_VERSION) {
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteAudioBuffer(dumpFileName_,
            static_cast<void *>(&data), len);
    }
    CheckUpdateState(&data, len);

    if (switchDeviceMute_) {
        Trace traceEmpty("AudioRendererSinkInner::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void*>(&data), static_cast<size_t>(len), 0,
            static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s failed");
        }
    }

    Trace::CountVolume("AudioRendererSinkInner::RenderFrame", static_cast<uint8_t>(data));
    CheckLatencySignal(reinterpret_cast<uint8_t*>(&data), len);

    Trace traceRenderFrame("AudioRendererSinkInner::RenderFrame");
    int32_t ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t*>(&data), static_cast<uint32_t>(len),
        &writeLen);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_FAILED, "RenderFrame failed ret: %{public}x", ret);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t stampThreshold = 50; // 50ms
    if (logMode_ || stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", len, stamp);
    }

#ifdef FEATURE_POWER_MANAGER
    if (runningLockManager_) {
        runningLockManager_->UpdateAppsUidToPowerMgr();
    }
#endif

    return SUCCESS;
}

void AudioRendererSinkInner::CheckUpdateState(char *frame, uint64_t replyBytes)
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

float AudioRendererSinkInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t AudioRendererSinkInner::Start(void)
{
    AUDIO_INFO_LOG("sinkName %{public}s", halName_.c_str());

    Trace trace("AudioRendererSinkInner::Start");
#ifdef FEATURE_POWER_MANAGER
    AudioXCollie audioXCollie("AudioRendererSinkInner::CreateRunningLock", TIME_OUT_SECONDS);
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (runningLockManager_ == nullptr) {
        std::string lockName = PRIMARY_LOCK_NAME_BASE + halName_;
        keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock(
            lockName, PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        if (keepRunningLock) {
            runningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock lock result: %{public}d",
            runningLockManager_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING)); // -1 for lasting.
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
    audioXCollie.CancelXCollieTimer();
#endif
    dumpFileName_ = halName_ + "_audiosink_" + std::to_string(attr_.sampleRate) + "_"
        + std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    InitLatencyMeasurement();
    if (!started_) {
        int32_t ret = audioRender_->Start(audioRender_);
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

    int32_t ret = audioRender_->SetVolume(audioRender_, volume);
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
    AUDIO_INFO_LOG("Set modem call volume %{public}f", volume);
    return audioAdapter_->SetVoiceVolume(audioAdapter_, volume);
}

int32_t AudioRendererSinkInner::GetLatency(uint32_t *latency)
{
    Trace trace("AudioRendererSinkInner::GetLatency");
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

AudioPortPin AudioRendererSinkInner::GetAudioPortPin() const noexcept
{
    switch (attr_.deviceType) {
        case DEVICE_TYPE_EARPIECE:
            return PIN_OUT_EARPIECE;
        case DEVICE_TYPE_SPEAKER:
            return PIN_OUT_SPEAKER;
        case DEVICE_TYPE_WIRED_HEADSET:
            return PIN_OUT_HEADSET;
        case DEVICE_TYPE_WIRED_HEADPHONES:
            return PIN_OUT_HEADPHONE;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return PIN_OUT_BLUETOOTH_SCO;
        case DEVICE_TYPE_USB_HEADSET:
            return PIN_OUT_USB_EXT;
        case DEVICE_TYPE_NONE:
            return PIN_NONE;
        default:
            return PIN_OUT_SPEAKER;
    }
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
        case DEVICE_TYPE_NONE:
            sink.ext.device.type = PIN_NONE;
            sink.ext.device.desc = (char *)"pin_out_none";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t AudioRendererSinkInner::SetAudioRoute(DeviceType outputDevice, AudioRoute route)
{
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE, "SetOutputRoutes failed with null adapter");
    int32_t ret = audioAdapter_->UpdateAudioRoute(audioAdapter_, &route, &routeHandle_);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("deviceType : %{public}d UpdateAudioRoute cost[%{public}" PRId64 "]ms", outputDevice, stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UpdateAudioRoute failed");

    return SUCCESS;
}

int32_t AudioRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType outputDevice = outputDevices.front();
    if (outputDevice == currentActiveDevice_ &&
        outputDevices.size() == static_cast<uint32_t>(currentDevicesSize_)) {
        AUDIO_INFO_LOG("SetOutputRoutes output device not change, type:%{public}d", outputDevice);
        return SUCCESS;
    }
    AudioPortPin outputPortPin = GetAudioPortPin();
    std::vector<std::pair<DeviceType, AudioPortPin>> outputDevicesPortPin = {};
    for (size_t i = 0; i < outputDevices.size(); i++) {
        outputDevicesPortPin.push_back(std::make_pair(outputDevices[i], outputPortPin));
    }
    return SetOutputRoutes(outputDevicesPortPin);
}

int32_t AudioRendererSinkInner::SetOutputRoutes(std::vector<std::pair<DeviceType, AudioPortPin>> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType outputDevice = outputDevices.front().first;
    AudioPortPin outputPortPin = outputDevices.front().second;
    Trace trace("AudioRendererSinkInner::SetOutputRoutes pin " + std::to_string(outputPortPin) + " device " +
        std::to_string(outputDevice));
    currentActiveDevice_ = outputDevice;
    currentDevicesSize_ = static_cast<int32_t>(outputDevices.size());

    AudioRouteNode source = {};
    source.portId = static_cast<int32_t>(0);
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_MIX_TYPE;
    source.ext.mix.moduleId = static_cast<int32_t>(0);
    source.ext.mix.streamId = PRIMARY_OUTPUT_STREAM_ID;
    source.ext.device.desc = (char *)"";

    int32_t sinksSize = static_cast<int32_t>(outputDevices.size());
    AudioRouteNode* sinks = new AudioRouteNode[sinksSize];

    for (size_t i = 0; i < outputDevices.size(); i++) {
        int32_t ret = SetOutputPortPin(outputDevices[i].first, sinks[i]);
        if (ret != SUCCESS) {
            delete [] sinks;
            AUDIO_ERR_LOG("SetOutputRoutes FAILED: %{public}d", ret);
            return ret;
        }
        outputDevices[i].second = sinks[i].ext.device.type;
        AUDIO_INFO_LOG("Output[%{public}zu] PIN is: 0x%{public}X DeviceType is %{public}d", i, outputDevices[i].second,
            outputDevices[i].first);
        sinks[i].portId = static_cast<int32_t>(audioPort_.portId);
        sinks[i].role = AUDIO_PORT_SINK_ROLE;
        sinks[i].type = AUDIO_PORT_DEVICE_TYPE;
        sinks[i].ext.device.moduleId = static_cast<int32_t>(0);
        sinks[i].ext.device.desc = (char *)"";
    }

    AudioRoute route = {};
    route.sources = &source;
    route.sourcesLen = 1;
    route.sinks = sinks;
    route.sinksLen = static_cast<uint32_t>(sinksSize);

    int32_t result = SetAudioRoute(outputDevice, route);
    if (sinks != nullptr) {
        delete [] sinks;
        sinks = nullptr;
    }
    return result;
}

int32_t AudioRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType activeDevice = activeDevices.front();
    AUDIO_INFO_LOG("SetAudioScene scene: %{public}d, device: %{public}d", audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "invalid audioScene");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "SetAudioScene failed audio render handle is null!");
    if (openSpeaker_) {
        AudioPortPin audioSceneOutPort = GetAudioPortPin();
        if (halName_ == "usb") {
            audioSceneOutPort = PIN_OUT_USB_HEADSET;
        } else if (halName_ == "dp") {
            audioSceneOutPort = PIN_OUT_DP;
        }
        AUDIO_DEBUG_LOG("OUTPUT port is %{public}d", audioSceneOutPort);
        bool isAudioSceneUpdate = false;
        if (audioScene != currentAudioScene_) {
            struct AudioSceneDescriptor scene;
            scene.scene.id = GetAudioCategory(audioScene);
            if (halName_ == DIRECT_HAL_NAME) {
                scene.scene.id = AUDIO_DIRECT;
            } else if (halName_ == VOIP_HAL_NAME) {
                scene.scene.id = AUDIO_IN_COMMUNICATION;
            }
            scene.desc.pins = audioSceneOutPort;
            scene.desc.desc = const_cast<char *>("");
            int32_t ret = audioRender_->SelectScene(audioRender_, &scene);
            CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED,
                "Select scene FAILED: %{public}d", ret);
            currentAudioScene_ = audioScene;
            isAudioSceneUpdate = true;
        }
        if (activeDevices.size() != static_cast<size_t>(currentDevicesSize_) || activeDevice != currentActiveDevice_ ||
            (isAudioSceneUpdate &&
            (currentAudioScene_ == AUDIO_SCENE_PHONE_CALL || currentAudioScene_ == AUDIO_SCENE_PHONE_CHAT))) {
            std::vector<std::pair<DeviceType, AudioPortPin>> activeDevicesPortPin = {};
            for (auto device : activeDevices) {
                activeDevicesPortPin.push_back(std::make_pair(device, audioSceneOutPort));
            }
            int32_t ret = SetOutputRoutes(activeDevicesPortPin);
            if (ret < 0) {
                AUDIO_ERR_LOG("Update route FAILED: %{public}d", ret);
            }
            currentDevicesSize_ = static_cast<int32_t>(activeDevices.size());
        }
    }
    return SUCCESS;
}

int32_t AudioRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
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
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unLock");
        std::thread runningLockThread([this] {
            runningLockManager_->UnLock();
        });
        runningLockThread.join();
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
#endif
}

int32_t AudioRendererSinkInner::Stop(void)
{
    AUDIO_INFO_LOG("sinkName %{public}s", halName_.c_str());

    Trace trace("AudioRendererSinkInner::Stop");

    DeinitLatencyMeasurement();

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Stop failed audioRender_ null");

    if (!started_) {
        return SUCCESS;
    }

    if (halName_ == PRIMARY_HAL_NAME) {
        const char keyValueList[] = "primary=stop";
        if (audioRender_->SetExtraParams(audioRender_, keyValueList) == 0) {
            AUDIO_INFO_LOG("set primary stream stop info to hal");
        }
    }

    int32_t ret = audioRender_->Stop(audioRender_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Stop failed!");
        return ERR_OPERATION_FAILED;
    }
    started_ = false;
    return SUCCESS;
}

int32_t AudioRendererSinkInner::Pause(void)
{
    AUDIO_INFO_LOG("sinkName %{public}s", halName_.c_str());

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Pause failed audioRender_ null");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Pause invalid state!");

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

int32_t AudioRendererSinkInner::Resume(void)
{
    AUDIO_INFO_LOG("sinkName %{public}s", halName_.c_str());

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Resume failed audioRender_ null");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Resume invalid state!");

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

int32_t AudioRendererSinkInner::Reset(void)
{
    AUDIO_INFO_LOG("sinkName %{public}s", halName_.c_str());

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

int32_t AudioRendererSinkInner::Flush(void)
{
    AUDIO_INFO_LOG("sinkName %{public}s", halName_.c_str());

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

int32_t AudioRendererSinkInner::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t AudioRendererSinkInner::RestoreRenderSink(void)
{
    return SUCCESS;
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
    attr_.sampleRate = static_cast<uint32_t>(stoi(sampleRateStr));
    attr_.channel = STEREO_CHANNEL_COUNT;
    attr_.format = ParseAudioFormat(formatStr);

    adapterNameCase_ = "usb";
    openSpeaker_ = 0;

    return SUCCESS;
}

int32_t AudioRendererSinkInner::UpdateDPAttrs(const std::string &dpInfoStr)
{
    CHECK_AND_RETURN_RET_LOG(dpInfoStr != "", ERR_INVALID_PARAM, "usb info string error");

    auto sinkRate_begin = dpInfoStr.find("rate=");
    auto sinkRate_end = dpInfoStr.find_first_of(" ", sinkRate_begin);
    std::string sampleRateStr = dpInfoStr.substr(sinkRate_begin + std::strlen("rate="),
        sinkRate_end - sinkRate_begin - std::strlen("rate="));

    auto sinkBuffer_begin = dpInfoStr.find("buffer_size=");
    auto sinkBuffer_end = dpInfoStr.find_first_of(" ", sinkBuffer_begin);
    std::string bufferSize = dpInfoStr.substr(sinkBuffer_begin + std::strlen("buffer_size="),
        sinkBuffer_end - sinkBuffer_begin - std::strlen("buffer_size="));

    auto sinkChannel_begin = dpInfoStr.find("channels=");
    auto sinkChannel_end = dpInfoStr.find_first_of(" ", sinkChannel_begin);
    std::string channeltStr = dpInfoStr.substr(sinkChannel_begin + std::strlen("channels="),
        sinkChannel_end - sinkChannel_begin - std::strlen("channels="));

    auto address_begin = dpInfoStr.find("address=");
    auto address_end = dpInfoStr.find_first_of(" ", address_begin);
    std::string addressStr = dpInfoStr.substr(address_begin + std::strlen("address="),
        address_end - address_begin - std::strlen("address="));

    attr_.sampleRate = stoi(sampleRateStr);
    attr_.channel = static_cast<uint32_t>(stoi(channeltStr));
    attr_.address = addressStr;
    uint32_t formatByte = 0;
    if (attr_.channel <= 0 || attr_.sampleRate <= 0) {
        AUDIO_ERR_LOG("check attr failed channel[%{public}d] sampleRate[%{public}d]", attr_.channel, attr_.sampleRate);
    } else {
        formatByte = static_cast<uint32_t>(stoi(bufferSize)) * BUFFER_CALC_1000MS / BUFFER_CALC_20MS
            / attr_.channel / attr_.sampleRate;
    }
    
    attr_.format = static_cast<HdiAdapterFormat>(ConvertByteToAudioFormat(formatByte));

    AUDIO_DEBUG_LOG("UpdateDPAttrs sampleRate %{public}d,format:%{public}d,channelCount:%{public}d,address:%{public}s",
        attr_.sampleRate, attr_.format, attr_.channel, addressStr.c_str());

    adapterNameCase_ = "dp";
    openSpeaker_ = 0;

    return SUCCESS;
}

int32_t AudioRendererSinkInner::InitAdapter()
{
    AUDIO_INFO_LOG("Init adapter start sinkName %{public}s", halName_.c_str());

    if (adapterInited_) {
        AUDIO_INFO_LOG("Adapter already inited");
        return SUCCESS;
    }

    int32_t err = InitAudioManager();
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_NOT_STARTED,
        "Init audio manager Fail.");

    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    if (audioManager_ == nullptr) {
        AUDIO_ERR_LOG("The audioManager is null");
        return ERROR;
    }
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
    AUDIO_INFO_LOG("Init render start sinkName %{public}s", halName_.c_str());

    Trace trace("AudioRendererSinkInner::InitRender");

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
        std::vector<DeviceType> outputDevices;
        if (halName_ == "usb") {
            outputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
            ret = SetOutputRoutes(outputDevices);
        } else if (halName_ == "dp") {
            outputDevices.push_back(DEVICE_TYPE_DP);
            ret = SetOutputRoutes(outputDevices);
        } else {
            DeviceType type = static_cast<DeviceType>(attr_.deviceType);
            if (type == DEVICE_TYPE_INVALID) {
                type = DEVICE_TYPE_SPEAKER;
            }
            outputDevices.push_back(type);
            ret = SetOutputRoutes(outputDevices);
        }
        if (ret < 0) {
            AUDIO_WARNING_LOG("Update route FAILED: %{public}d", ret);
        }
    }

    renderInited_ = true;

    return SUCCESS;
}

void AudioRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
    if (currentActiveDevice_ == device) {
        currentActiveDevice_ = DEVICE_TYPE_NONE;
    }
}

void AudioRendererSinkInner::InitLatencyMeasurement()
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }

    AUDIO_INFO_LOG("LatencyMeas PrimaryRendererSinkInit");

    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
}

void AudioRendererSinkInner::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
}

void AudioRendererSinkInner::CheckLatencySignal(uint8_t *data, size_t len)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    uint32_t byteSize = static_cast<uint32_t>(GetFormatByteSize(attr_.format));
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
        AUDIO_INFO_LOG("LatencyMeas primarySink signal detected");
        detectedTime_ = 0;
    }
}

int32_t AudioRendererSinkInner::GetCurDeviceParam(char *keyValueList, size_t len)
{
    int32_t ret = ERROR;
    switch (currentActiveDevice_) {
        case DEVICE_TYPE_EARPIECE:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=1");
            break;
        case DEVICE_TYPE_SPEAKER:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=2");
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=4");
            break;
        case DEVICE_TYPE_USB_ARM_HEADSET:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=67108864");
            break;
        case DEVICE_TYPE_USB_HEADSET:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=545259520");
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=16");
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=128");
            break;
        default:
            ret = snprintf_s(keyValueList, len, len - 1,
                "zero_volume=true;routing=-100");
            break;
    }
    return ret;
}

int32_t AudioRendererSinkInner::SetPaPower(int32_t flag)
{
    Trace trace("AudioRendererSinkInner::SetPaPower flag:" + std::to_string(flag));
    int32_t ret = ERROR;
    char keyValueList[DEVICE_PARAM_MAX_LEN] = {0};
    const char keyValueList1[] = "zero_volume=false";

    if (flag == 0 && g_paStatus == 1) {
        ret = snprintf_s(keyValueList, sizeof(keyValueList), sizeof(keyValueList) - 1,
            "zero_volume=true;routing=0");
        if (ret > 0 && ret < sizeof(keyValueList)) {
            CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERROR);
            ret = audioRender_->SetExtraParams(audioRender_, keyValueList);
        }
        if (ret == 0) {
            g_paStatus = 0;
        }
        return ret;
    } else if (flag == 0 && g_paStatus == 0) {
        return SUCCESS;
    }

    AUDIO_INFO_LOG("Get keyValueList %{public}s before get.", keyValueList);
    GetCurDeviceParam(keyValueList, DEVICE_PARAM_MAX_LEN);
    AUDIO_INFO_LOG("Get keyValueList for openpa: %{public}s", keyValueList);

    if (flag == 1 && g_paStatus == 0) {
        ret = audioRender_->SetExtraParams(audioRender_, keyValueList);
        ret = audioRender_->SetExtraParams(audioRender_, keyValueList1) + ret;
        if (ret == 0) {
            g_paStatus = 1;
        }
        return ret;
    } else if (flag == 1 && g_paStatus == 1) {
        return SUCCESS;
    }

    AUDIO_INFO_LOG("receive invalid flag");
    return ret;
}

int32_t AudioRendererSinkInner::SetPriPaPower()
{
    time_t nowTime = time(nullptr);
    int32_t ret = ERROR;
    const char keyValueList[] = "primary=start";
    double diff = difftime(nowTime, startTime);
    if (diff > INTREVAL) {
        CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERROR);
        ret = audioRender_->SetExtraParams(audioRender_, keyValueList);
        if (ret == 0) {
            AUDIO_INFO_LOG("set primary stream start info to hal");
        }
        time(&startTime);
    }
    return ret;
}

int32_t AudioRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS],
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

int32_t AudioRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
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
int32_t AudioRendererSinkInner::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set %{public}s mute %{public}d", halName_.c_str(), mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("%{public}s already muted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = true;
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("%{public}s not all unmuted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
    }

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS