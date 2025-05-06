/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioCapturerSourceInner"
#endif

#include "audio_capturer_source.h"

#include <cstring>
#include <dlfcn.h>
#include <string>
#include <cinttypes>
#include <thread>
#include <future>
#include <vector>

#include "securec.h"
#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#include "running_lock.h"
#include "audio_running_lock_manager.h"
#endif
#include "v4_0/iaudio_manager.h"

#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "volume_tools.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "parameters.h"
#include "media_monitor_manager.h"
#include "audio_enhance_chain_manager.h"
#include "hdi_utils_ringbuffer.h"
#include "audio_dump_pcm.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int64_t SECOND_TO_NANOSECOND = 1000000000;
const unsigned int DEINIT_TIME_OUT_SECONDS = 5;
const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
const int32_t BYTE_SIZE_SAMPLE_U8 = 1;
const int32_t BYTE_SIZE_SAMPLE_S16 = 2;
const int32_t BYTE_SIZE_SAMPLE_S24 = 3;
const int32_t BYTE_SIZE_SAMPLE_S32 = 4;
const uint32_t FRAME_TIME_LEN_MS = 20; // 20ms
const uint32_t MILLISECONDS_PER_SECOND_MS = 1000;
} // namespace

static int32_t GetByteSizeByFormat(HdiAdapterFormat format)
{
    int32_t byteSize = 0;
    switch (format) {
        case SAMPLE_U8:
            byteSize = BYTE_SIZE_SAMPLE_U8;
            break;
        case SAMPLE_S16:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
        case SAMPLE_S24:
            byteSize = BYTE_SIZE_SAMPLE_S24;
            break;
        case SAMPLE_S32:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        default:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
    }

    return byteSize;
}

static bool IsNonblockingSource(int32_t source, const std::string &adapterName)
{
    return (source == SOURCE_TYPE_EC && adapterName != "dp") || (source == SOURCE_TYPE_MIC_REF);
}

static uint32_t GenerateUniqueIDBySource(int32_t source)
{
    uint32_t sourceId = 0;
    switch (source) {
        case SOURCE_TYPE_EC:
            sourceId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_EC);
            break;
        case SOURCE_TYPE_MIC_REF:
            sourceId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_MIC_REF);
            break;
        case SOURCE_TYPE_WAKEUP:
            sourceId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_WAKEUP);
            break;
        default:
            sourceId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_PRIMARY);
            break;
    }
    return sourceId;
}

static uint64_t GetChannelLayoutByCount(uint32_t channlCount)
{
    uint64_t channelLayout = 0;
    switch (channlCount) {
        case MONO:
            channelLayout = CH_LAYOUT_MONO;
            break;
        case STEREO:
            channelLayout = CH_LAYOUT_STEREO;
            break;
        case CHANNEL_4:
            channelLayout = CH_LAYOUT_QUAD;
            break;
        case CHANNEL_8:
            channelLayout = CH_LAYOUT_7POINT1;
            break;
        default:
            channelLayout = CH_LAYOUT_STEREO;
            break;
    }
    return channelLayout;
}

// inner class definations
class AudioCapturerSourceInner : public AudioCapturerSource {
public:
    int32_t Init(const IAudioSourceAttr &attr) override;
    int32_t InitWithoutAttr() override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t CaptureFrameWithEc(
        FrameDesc *fdesc, uint64_t &replyBytes,
        FrameDesc *fdescEc, uint64_t &replyBytesEc) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;

    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice, const std::string &deviceName = "") override;

    int32_t SetInputRoute(DeviceType inputDevice, const std::string &deviceName = "") override;
    uint64_t GetTransactionId() override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;

    int32_t Preload(const std::string &usbInfoStr) override;
    float GetMaxAmplitude() override;
    int32_t GetCaptureId(uint32_t &captureId) const override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
        const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    int32_t UpdateSourceType(SourceType sourceType) final;

    void SetAddress(const std::string &address) override;

    explicit AudioCapturerSourceInner(const std::string &halName = "primary");
    explicit AudioCapturerSourceInner(CaptureAttr *attr);
    ~AudioCapturerSourceInner();

private:
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr float MAX_VOLUME_LEVEL = 15.0f;
    static constexpr uint32_t USB_DEFAULT_BUFFERSIZE = 3840;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;

    int32_t CreateCapture(struct AudioPort &capturePort);
    int32_t InitAudioManager();
    void SetEcSampleAttributes(struct AudioSampleAttributes &attrs);
    void InitAttrsCapture(struct AudioSampleAttributes &attrs);
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);

    int32_t UpdateUsbAttrs(const std::string &usbInfoStr);
    int32_t InitManagerAndAdapter();
    int32_t InitAdapterAndCapture();

    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckLatencySignal(uint8_t *frame, size_t replyBytes);

    void CheckUpdateState(char *frame, uint64_t replyBytes);
    int32_t SetAudioRouteInfoForEnhanceChain(const DeviceType &inputDevice, const std::string &deviceName = "");
    int32_t SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin, const std::string &deviceName = "");
    int32_t DoSetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin, const std::string &deviceName = "");

    void CaptureThreadLoop();
    void CaptureFrameEcInternal(std::vector<uint8_t> &ecData);
    int32_t ProcessCaptureBlockingEc(FrameDesc *fdescEc, uint64_t &replyBytesEc);
    int32_t StartNonblockingCapture();
    int32_t StopNonblockingCapture();
    void DumpCapturerSourceData(char *frame, uint64_t &replyBytes);

    int32_t DoStop();
    int32_t StartCapture();

    bool GetMuteState();
    void SetMuteState(bool isMute);

    CaptureAttr *hdiAttr_ = nullptr;
    IAudioSourceAttr attr_ = {};
    bool sourceInited_ = false;
    bool captureInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    float leftVolume_ = 0.0f;
    float rightVolume_ = 0.0f;

    int32_t routeHandle_ = -1;
    int32_t logMode_ = 0;
    uint32_t openMic_ = 0;
    uint32_t captureId_ = 0;
    std::string adapterNameCase_ = "";
    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "AudioSource";

    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int capFrameNum_ = 0;

    struct IAudioManager *audioManager_ = nullptr;
    std::atomic<bool> adapterLoaded_ = false;
    struct IAudioAdapter *audioAdapter_ = nullptr;
    struct IAudioCapture *audioCapture_ = nullptr;
    std::string halName_ = "";
    struct AudioAdapterDescriptor adapterDesc_ = {};
    struct AudioPort audioPort_ = {};
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
#endif
    IAudioSourceCallback* wakeupCloseCallback_ = nullptr;
    std::mutex wakeupClosecallbackMutex_;

    std::unique_ptr<ICapturerStateCallback> audioCapturerSourceCallback_ = nullptr;
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    std::mutex muteStateMutex_;
    bool muteState_ = false;
    DeviceType currentActiveDevice_ = DEVICE_TYPE_INVALID;
    AudioScene currentAudioScene_ = AUDIO_SCENE_INVALID;
    bool latencyMeasEnabled_ = false;
    bool signalDetected_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    std::mutex signalDetectAgentMutex_;

    std::mutex statusMutex_;

    // ec and mic ref feature
    std::unique_ptr<std::thread> captureThread_ = nullptr;
    bool threadRunning_ = false;
    std::shared_ptr<HdiRingBuffer> ringBuffer_ = nullptr;

    std::string address_;
};

class AudioCapturerSourceWakeup : public AudioCapturerSource {
public:
    int32_t Init(const IAudioSourceAttr &attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t CaptureFrameWithEc(
        FrameDesc *fdesc, uint64_t &replyBytes,
        FrameDesc *fdescEc, uint64_t &replyBytesEc) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;

    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice, const std::string &deviceName = "") override;

    int32_t SetInputRoute(DeviceType inputDevice, const std::string &deviceName = "") override;
    uint64_t GetTransactionId() override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;
    float GetMaxAmplitude() override;
    int32_t GetCaptureId(uint32_t &captureId) const override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
        const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    AudioCapturerSourceWakeup() = default;
    ~AudioCapturerSourceWakeup() = default;

private:
    static inline void MemcpysAndCheck(void *dest, size_t destMax, const void *src, size_t count)
    {
        if (memcpy_s(dest, destMax, src, count)) {
            AUDIO_ERR_LOG("memcpy_s error");
        }
    }
    class WakeupBuffer {
    public:
        explicit WakeupBuffer(size_t sizeMax = BUFFER_SIZE_MAX)
            : sizeMax_(sizeMax),
              buffer_(std::make_unique<char[]>(sizeMax))
        {
        }

        ~WakeupBuffer() = default;

        int32_t Poll(char *frame, uint64_t requestBytes, uint64_t &replyBytes, uint64_t &noStart)
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (noStart < headNum_) {
                noStart = headNum_;
            }

            if (noStart >= (headNum_ + size_)) {
                if (requestBytes > sizeMax_) {
                    requestBytes = sizeMax_;
                }

                int32_t res = audioCapturerSource_.CaptureFrame(frame, requestBytes, replyBytes);
                Offer(frame, replyBytes);

                return res;
            }

            if (requestBytes > size_) { // size_!=0
                replyBytes = size_;
            } else {
                replyBytes = requestBytes;
            }

            uint64_t tail = (head_ + size_) % sizeMax_;

            if (tail > head_) {
                MemcpysAndCheck(frame, replyBytes, buffer_.get() + head_, replyBytes);
                headNum_ += replyBytes;
                size_ -= replyBytes;
                head_ = (head_ + replyBytes) % sizeMax_;
            } else {
                uint64_t copySize = std::min((sizeMax_ - head_), replyBytes);
                if (copySize != 0) {
                    MemcpysAndCheck(frame, replyBytes, buffer_.get() + head_, copySize);
                    headNum_ += copySize;
                    size_ -= copySize;
                    head_ = (head_ + copySize) % sizeMax_;
                }

                uint64_t remainCopySize = replyBytes - copySize;
                if (remainCopySize != 0) {
                    MemcpysAndCheck(frame + copySize, remainCopySize, buffer_.get(), remainCopySize);
                    headNum_ += remainCopySize;
                    size_ -= remainCopySize;
                    head_ = (head_ + remainCopySize) % sizeMax_;
                }
            }

            return SUCCESS;
        }
    private:
        static constexpr size_t BUFFER_SIZE_MAX = 32000; // 2 seconds

        const size_t sizeMax_;
        size_t size_ = 0;

        std::unique_ptr<char[]> buffer_;
        std::mutex mutex_;

        uint64_t head_ = 0;

        uint64_t headNum_ = 0;

        void Offer(const char *frame, const uint64_t bufferBytes)
        {
            if ((size_ + bufferBytes) > sizeMax_) { // head_ need shift
                u_int64_t shift = (size_ + bufferBytes) - sizeMax_; // 1 to sizeMax_
                headNum_ += shift;
                if (size_ > shift) {
                    size_ -= shift;
                    head_ = ((head_ + shift) % sizeMax_);
                } else {
                    size_ = 0;
                    head_ = 0;
                }
            }

            uint64_t tail = (head_ + size_) % sizeMax_;
            if (tail < head_) {
                MemcpysAndCheck((buffer_.get() + tail), bufferBytes, frame, bufferBytes);
            } else {
                uint64_t copySize = std::min(sizeMax_ - tail, bufferBytes);
                MemcpysAndCheck((buffer_.get() + tail), sizeMax_ - tail, frame, copySize);

                if (copySize < bufferBytes) {
                    MemcpysAndCheck((buffer_.get()), bufferBytes - copySize, frame + copySize, bufferBytes - copySize);
                }
            }
            size_ += bufferBytes;
        }
    };

    uint64_t noStart_ = 0;
    std::atomic<bool> isInited = false;
    static inline int initCount = 0;

    std::atomic<bool> isStarted = false;
    static inline int startCount = 0;

    static inline std::unique_ptr<WakeupBuffer> wakeupBuffer_;
    static inline std::mutex wakeupMutex_;

    static inline AudioCapturerSourceInner audioCapturerSource_;
};
#ifdef FEATURE_POWER_MANAGER
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif

AudioCapturerSourceInner::AudioCapturerSourceInner(const std::string &halName)
    : sourceInited_(false), captureInited_(false), started_(false), paused_(false),
      leftVolume_(MAX_VOLUME_LEVEL), rightVolume_(MAX_VOLUME_LEVEL), openMic_(0),
      audioManager_(nullptr), audioAdapter_(nullptr), audioCapture_(nullptr), halName_(halName)
{
    attr_ = {};
}

AudioCapturerSourceInner::AudioCapturerSourceInner(CaptureAttr *attr)
    : sourceInited_(false), captureInited_(false), started_(false), paused_(false),
      leftVolume_(MAX_VOLUME_LEVEL), rightVolume_(MAX_VOLUME_LEVEL), openMic_(0),
      audioManager_(nullptr), audioAdapter_(nullptr), audioCapture_(nullptr), halName_("primary")
{
    hdiAttr_ = attr;
    attr_ = {};
}

AudioCapturerSourceInner::~AudioCapturerSourceInner()
{
    AUDIO_WARNING_LOG("~AudioCapturerSourceInner");
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);

    threadRunning_ = false;
    if (hdiAttr_ != nullptr) {
        free(hdiAttr_);
        hdiAttr_ = nullptr;
    }
}

AudioCapturerSource *AudioCapturerSource::Create(CaptureAttr *attr)
{
    AudioCapturerSource *captureSource = new AudioCapturerSourceInner(attr);
    return captureSource;
}

AudioCapturerSource *AudioCapturerSource::GetInstance(const std::string &halName,
    const SourceType sourceType, const char *sourceName)
{
    Trace trace("AudioCapturerSourceInner:GetInstance");
    if (halName == "usb") {
        static AudioCapturerSourceInner audioCapturerUsb(halName);
        return &audioCapturerUsb;
    }

    switch (sourceType) {
        case SourceType::SOURCE_TYPE_MIC:
        case SourceType::SOURCE_TYPE_VOICE_CALL:
        case SourceType::SOURCE_TYPE_CAMCORDER:
        case SourceType::SOURCE_TYPE_UNPROCESSED:
            return GetMicInstance();
        case SourceType::SOURCE_TYPE_WAKEUP:
            if (!strcmp(sourceName, "Built_in_wakeup_mirror")) {
                return GetWakeupInstance(true);
            } else {
                return GetWakeupInstance(false);
            }
        default:
            AUDIO_ERR_LOG("sourceType error %{public}d", sourceType);
            return GetMicInstance();
    }
}

static enum AudioInputType ConvertToHDIAudioInputType(const int32_t currSourceType)
{
    enum AudioInputType hdiAudioInputType;
    switch (currSourceType) {
        case SOURCE_TYPE_INVALID:
            hdiAudioInputType = AUDIO_INPUT_DEFAULT_TYPE;
            break;
        case SOURCE_TYPE_MIC:
        case SOURCE_TYPE_PLAYBACK_CAPTURE:
        case SOURCE_TYPE_ULTRASONIC:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
        case SOURCE_TYPE_WAKEUP:
            hdiAudioInputType = AUDIO_INPUT_SPEECH_WAKEUP_TYPE;
            break;
        case SOURCE_TYPE_VOICE_TRANSCRIPTION:
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            hdiAudioInputType = AUDIO_INPUT_VOICE_COMMUNICATION_TYPE;
            break;
        case SOURCE_TYPE_VOICE_RECOGNITION:
            hdiAudioInputType = AUDIO_INPUT_VOICE_RECOGNITION_TYPE;
            break;
        case SOURCE_TYPE_VOICE_CALL:
            hdiAudioInputType = AUDIO_INPUT_VOICE_CALL_TYPE;
            break;
        case SOURCE_TYPE_CAMCORDER:
            hdiAudioInputType = AUDIO_INPUT_CAMCORDER_TYPE;
            break;
        case SOURCE_TYPE_EC:
            hdiAudioInputType = AUDIO_INPUT_EC_TYPE;
            break;
        case SOURCE_TYPE_MIC_REF:
            hdiAudioInputType = AUDIO_INPUT_NOISE_REDUCTION_TYPE;
            break;
        case SOURCE_TYPE_UNPROCESSED:
            hdiAudioInputType = AUDIO_INPUT_RAW_TYPE;
            break;
        default:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
    }
    return hdiAudioInputType;
}

AudioCapturerSource *AudioCapturerSource::GetMicInstance()
{
    static AudioCapturerSourceInner audioCapturer;
    return &audioCapturer;
}

AudioCapturerSource *AudioCapturerSource::GetWakeupInstance(bool isMirror)
{
    if (isMirror) {
        static AudioCapturerSourceWakeup audioCapturerMirror;
        return &audioCapturerMirror;
    }
    static AudioCapturerSourceWakeup audioCapturer;
    return &audioCapturer;
}

bool AudioCapturerSourceInner::IsInited(void)
{
    return sourceInited_;
}

void AudioCapturerSourceInner::DeInit()
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    Trace trace("AudioCapturerSourceInner::DeInit");
    AudioXCollie sourceXCollie("AudioCapturerSourceInner::DeInit", DEINIT_TIME_OUT_SECONDS);
    AUDIO_INFO_LOG("Start deinit of source inner");
    started_ = false;
    sourceInited_ = false;

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyCapture(audioAdapter_, captureId_);
    }
    captureInited_ = false;

    IAudioSourceCallback* callback = nullptr;
    {
        std::lock_guard<std::mutex> lck(wakeupClosecallbackMutex_);
        callback = wakeupCloseCallback_;
    }
    if (callback != nullptr) {
        callback->OnWakeupClose();
    }

    audioCapture_ = nullptr;
    currentActiveDevice_ = DEVICE_TYPE_INVALID; // the current device must be rest when closing capturer.

    // Only the usb hal needs to be unloadadapter at the moment.
    if (halName_ == "usb") {
        adapterLoaded_ = false;
        if (audioManager_ != nullptr) {
            audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
        }
        audioAdapter_ = nullptr;
        audioManager_ = nullptr;
    }

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

void AudioCapturerSourceInner::InitAttrsCapture(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = static_cast<int32_t>(GenerateUniqueIDBySource(attr_.sourceType));
    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    /* 16 * 1024 */
    attrs.silenceThreshold = AUDIO_BUFF_SIZE;
    attrs.sourceType = SOURCE_TYPE_MIC;
}

int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size, const std::string &adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &capturePort)
{
    if (descs == nullptr) {
        return ERROR;
    }

    for (uint32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        AUDIO_INFO_LOG("size: %{public}d, adapterNameCase %{public}s, adapterName %{public}s",
            size, adapterNameCase.c_str(), desc->adapterName);
        if (!adapterNameCase.compare(desc->adapterName)) {
            for (uint32_t port = 0; port < desc->portsLen; port++) {
                // Only find out the port of out in the sound card
                if (desc->ports[port].dir == portFlag) {
                    capturePort = desc->ports[port];
                    return index;
                }
            }
        }
    }
    AUDIO_ERR_LOG("SwitchAdapterCapture Fail");

    return ERR_INVALID_INDEX;
}

int32_t AudioCapturerSourceInner::InitAudioManager()
{
    AUDIO_INFO_LOG("Initialize audio proxy manager");

    if (audioManager_ == nullptr) {
        audioManager_ = IAudioManagerGet(false);
    }

    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    return 0;
}

AudioFormat AudioCapturerSourceInner::ConvertToHdiFormat(HdiAdapterFormat format)
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

void AudioCapturerSourceInner::SetEcSampleAttributes(struct AudioSampleAttributes &attrs)
{
    attrs.ecSampleAttributes.ecInterleaved = true;
    attrs.ecSampleAttributes.ecFormat = ConvertToHdiFormat(attr_.formatEc);
    attrs.ecSampleAttributes.ecSampleRate = attr_.sampleRateEc;
    attrs.ecSampleAttributes.ecChannelCount = attr_.channelEc;
    attrs.ecSampleAttributes.ecChannelLayout = GetChannelLayoutByCount(attr_.channelEc);
    attrs.ecSampleAttributes.ecPeriod = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    attrs.ecSampleAttributes.ecFrameSize = PCM_16_BIT * attrs.ecSampleAttributes.ecChannelCount / PCM_8_BIT;
    attrs.ecSampleAttributes.ecIsBigEndian = false;
    attrs.ecSampleAttributes.ecIsSignedData = true;
    attrs.ecSampleAttributes.ecStartThreshold =
        DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (attrs.ecSampleAttributes.ecFrameSize);
    attrs.ecSampleAttributes.ecStopThreshold = INT_32_MAX;
    attrs.ecSampleAttributes.ecSilenceThreshold = AUDIO_BUFF_SIZE;
    AUDIO_INFO_LOG("Ec config ecSampleRate: %{public}d ecChannel: %{public}u ecFormat: %{public}u",
        attrs.ecSampleAttributes.ecSampleRate, attrs.ecSampleAttributes.ecChannelCount,
        attrs.ecSampleAttributes.ecFormat);
}

int32_t AudioCapturerSourceInner::CreateCapture(struct AudioPort &capturePort)
{
    Trace trace("AudioCapturerSourceInner:CreateCapture");

    struct AudioSampleAttributes param;
    // User needs to set
    InitAttrsCapture(param);
    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.channelLayout = GetChannelLayoutByCount(attr_.channel);
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    param.sourceType = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));

    if (attr_.hasEcConfig || attr_.sourceType == SOURCE_TYPE_EC) {
        SetEcSampleAttributes(param);
    }

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = PIN_IN_MIC;
    if (halName_ == "usb") {
        deviceDesc.pins = PIN_IN_USB_HEADSET;
    }
    std::string desc = address_;
    deviceDesc.desc = const_cast<char*>(desc.c_str());

    AUDIO_INFO_LOG("Create capture sourceName:%{public}s, hdisource:%{public}d, " \
        "rate:%{public}u channel:%{public}u format:%{public}u, devicePin:%{public}u desc:%{public}s",
        halName_.c_str(), param.sourceType, param.sampleRate, param.channelCount,
        param.format, deviceDesc.pins, deviceDesc.desc);
    int32_t ret = audioAdapter_->CreateCapture(audioAdapter_, &deviceDesc, &param, &audioCapture_, &captureId_);
    if (ret < 0 || audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("Create capture failed");
        currentActiveDevice_ = DEVICE_TYPE_INVALID;
        return ERR_NOT_STARTED;
    }

    return 0;
}

static bool IsFormalSourceType(int32_t sourceType)
{
    if (sourceType == SOURCE_TYPE_EC) {
        return false;
    }
    if (sourceType == SOURCE_TYPE_MIC_REF) {
        return false;
    }
    return true;
}

int32_t AudioCapturerSourceInner::Init(const IAudioSourceAttr &attr)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;
    openMic_ = attr_.openMicSpeaker;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);

    int32_t ret = InitAdapterAndCapture();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init adapter and capture failed");

    sourceInited_ = true;

    if (GetMuteState() && IsFormalSourceType(attr_.sourceType)) {
        SetMute(true);
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::InitWithoutAttr()
{
    // build attr
    IAudioSourceAttr attr = {};
    attr.adapterName = hdiAttr_->adapterName;
    attr.openMicSpeaker = hdiAttr_->openMicSpeaker;
    attr.format = hdiAttr_->format;
    attr.sampleRate = hdiAttr_->sampleRate;
    attr.channel = hdiAttr_->channelCount;
    attr.bufferSize = USB_DEFAULT_BUFFERSIZE;
    attr.isBigEndian = hdiAttr_->isBigEndian;
    attr.filePath = "";
    attr.deviceNetworkId = "LocalDevice";
    attr.deviceType = hdiAttr_->deviceType;
    attr.sourceType = hdiAttr_->sourceType;
    if (attr.sourceType == SOURCE_TYPE_EC) {
        attr.formatEc = hdiAttr_->format;
        attr.sampleRateEc = hdiAttr_->sampleRate;
        attr.channelEc = hdiAttr_->channelCount;
    }
    Init(attr);

    if (IsNonblockingSource(attr.sourceType, attr.adapterName)) {
        ringBuffer_ = std::make_shared<HdiRingBuffer>();
        ringBuffer_->Init(attr.sampleRate, attr.channel, GetByteSizeByFormat(attr.format));
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Audio capture Handle is nullptr!");

    Trace trace("AudioCapturerSourceInner::CaptureFrame");

    // only mic ref use this
    if (attr_.sourceType == SOURCE_TYPE_MIC_REF) {
        if (ringBuffer_ != nullptr) {
            Trace traceSec("CaptureRefOutput");
            int32_t ret = ringBuffer_->ReadDataFromRingBuffer(reinterpret_cast<uint8_t *>(frame), requestBytes);
            if (ret == SUCCESS) {
                replyBytes = requestBytes;
            } else {
                AUDIO_ERR_LOG("read micRef data from ringBuffer fail");
                replyBytes = 0;
            }
        }

        return SUCCESS;
    }

    int64_t stamp = ClockTime::GetCurNano();
    uint32_t frameLen = static_cast<uint32_t>(requestBytes);
    int32_t ret = audioCapture_->CaptureFrame(audioCapture_, reinterpret_cast<int8_t*>(frame), &frameLen, &replyBytes);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_READ_FAILED, "Capture Frame Fail");
    CheckLatencySignal(reinterpret_cast<uint8_t*>(frame), replyBytes);

    DumpCapturerSourceData(frame, replyBytes);

    CheckUpdateState(frame, requestBytes);

    int64_t stampThreshold = 50; // 50ms
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_ || stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("CaptureFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", requestBytes, stamp);
    }
    return SUCCESS;
}

int32_t AudioCapturerSourceInner::ProcessCaptureBlockingEc(FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    if (ringBuffer_ != nullptr) {
        Trace traceSec("CaptureEcOutput");
        int32_t ret = ringBuffer_->ReadDataFromRingBuffer(reinterpret_cast<uint8_t *>(fdescEc->frame),
            fdescEc->frameLen);
        if (ret == SUCCESS) {
            replyBytesEc = fdescEc->frameLen;
        } else {
            AUDIO_ERR_LOG("read ec data from ringBuffer fail");
            replyBytesEc = 0;
        }
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes,
    FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Audio capture Handle is nullptr!");

    // ec different adapter only check ec frame
    if (attr_.sourceType != SOURCE_TYPE_EC) {
        CHECK_AND_RETURN_RET_LOG(fdesc != nullptr && fdesc->frame != nullptr &&
            fdescEc != nullptr && fdescEc->frame != nullptr, ERR_INVALID_PARAM, "frame desc error");
    } else {
        CHECK_AND_RETURN_RET_LOG(fdescEc != nullptr && fdescEc->frame != nullptr,
            ERR_INVALID_PARAM, "frame desc error");
    }

    if (IsNonblockingSource(attr_.sourceType, adapterNameCase_)) {
        return ProcessCaptureBlockingEc(fdescEc, replyBytesEc);
    }

    struct AudioFrameLen frameLen = {fdesc->frameLen, fdescEc->frameLen};
    struct AudioCaptureFrameInfo frameInfo = {};

    int32_t ret = audioCapture_->CaptureFrameEc(audioCapture_, &frameLen, &frameInfo);
    if (ret < 0) {
        AUDIO_ERR_LOG("Capture Frame with ec fail");
        AudioCaptureFrameInfoFree(&frameInfo, false);
        return ERR_READ_FAILED;
    }

    // same adapter reply length is mic + ec, different adapter is only ec, so we can't use reply bytes to copy
    if (attr_.sourceType != SOURCE_TYPE_EC && frameInfo.frame != nullptr) {
        if (frameInfo.replyBytes - fdescEc->frameLen < fdesc->frameLen) {
            replyBytes = 0;
            return ERR_INVALID_READ;
        }
        if (memcpy_s(fdesc->frame, fdesc->frameLen, frameInfo.frame, fdesc->frameLen) != EOK) {
            AUDIO_ERR_LOG("memcpy error");
        } else {
            replyBytes = (attr_.sourceType == SOURCE_TYPE_EC) ? 0 : fdesc->frameLen;
            DumpCapturerSourceData(fdesc->frame, replyBytes);
        }
    }
    if (frameInfo.frameEc != nullptr) {
        if (memcpy_s(fdescEc->frame, fdescEc->frameLen, frameInfo.frameEc, fdescEc->frameLen) != EOK) {
            AUDIO_ERR_LOG("memcpy ec error");
        } else {
            replyBytesEc = (attr_.sourceType == SOURCE_TYPE_EC) ?
                frameInfo.replyBytesEc : fdescEc->frameLen;
        }
    }
    CheckUpdateState(fdesc->frame, replyBytes);
    AudioCaptureFrameInfoFree(&frameInfo, false);

    return SUCCESS;
}

void AudioCapturerSourceInner::DumpCapturerSourceData(char *frame, uint64_t &replyBytes)
{
    BufferDesc tmpBuffer = {reinterpret_cast<uint8_t*>(frame), replyBytes, replyBytes};
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate),
        AudioEncodingType::ENCODING_PCM, static_cast<AudioSampleFormat>(attr_.format),
        static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(tmpBuffer, streamInfo, logUtilsTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, frame, replyBytes);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteAudioBuffer(dumpFileName_,
            static_cast<void*>(frame), replyBytes);
    }
}

void AudioCapturerSourceInner::CaptureFrameEcInternal(std::vector<uint8_t> &ecData)
{
    CHECK_AND_RETURN_LOG(audioCapture_, "audioCapture_ is nullptr");
    // mic frame just used for check, ec frame must be right
    struct AudioFrameLen frameLen = {};
    frameLen.frameLen = static_cast<uint64_t>(ecData.size());
    frameLen.frameEcLen = static_cast<uint64_t>(ecData.size());
    struct AudioCaptureFrameInfo frameInfo = {};
    int32_t ret = audioCapture_->CaptureFrameEc(audioCapture_, &frameLen, &frameInfo);
    if (ret >= 0 && frameInfo.frameEc != nullptr) {
        if (memcpy_s(ecData.data(), ecData.size(), frameInfo.frameEc, frameInfo.replyBytesEc) != EOK) {
            AUDIO_ERR_LOG("memcpy ec error");
        }
    }
    AudioCaptureFrameInfoFree(&frameInfo, false);
}

void AudioCapturerSourceInner::CaptureThreadLoop()
{
    if (ringBuffer_ == nullptr) {
        AUDIO_ERR_LOG("ring buffer not init");
        return;
    }

    uint32_t captureDataLen = FRAME_TIME_LEN_MS * attr_.sampleRate / MILLISECONDS_PER_SECOND_MS *
        static_cast<uint32_t>(GetByteSizeByFormat(attr_.format)) * attr_.channel;
    AUDIO_INFO_LOG("non blocking capture thread start, source type: %{public}d, captureDataLen: %{public}u",
        attr_.sourceType, captureDataLen);
    std::vector<uint8_t> tempBuf;
    tempBuf.resize(captureDataLen);
    ScheduleThreadInServer(getpid(), gettid());
    while (threadRunning_) {
        Trace trace("CaptureRefInput");
        uint64_t replyBytes = 0;
        uint32_t requestBytes = static_cast<uint32_t>(tempBuf.size());
        if (attr_.sourceType == SOURCE_TYPE_MIC_REF) {
            CHECK_AND_RETURN_LOG(audioCapture_, "audioCapture_ is nullptr");
            int32_t ret = audioCapture_->CaptureFrame(
                audioCapture_, reinterpret_cast<int8_t *>(tempBuf.data()), &requestBytes, &replyBytes);
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("Capture frame failed");
            }
        } else {
            CaptureFrameEcInternal(tempBuf);
        }

        ringBuffer_->WriteDataToRingBuffer(tempBuf.data(), tempBuf.size());
    }
    UnscheduleThreadInServer(getpid(), gettid());
    AUDIO_INFO_LOG("non blocking capture thread exit, source type: %{public}d", attr_.sourceType);
}

void AudioCapturerSourceInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        std::lock_guard<std::mutex> lock(statusMutex_);
        if (capFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        capFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (capFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            capFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float AudioCapturerSourceInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t AudioCapturerSourceInner::StartNonblockingCapture()
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_, ERR_INVALID_HANDLE, "audioCapture_ is nullptr");
    if (!started_) {
        int32_t ret = audioCapture_->Start(audioCapture_);
        if (ret < 0) {
            return ERR_NOT_STARTED;
        }
        started_ = true;

        // start non-blocking capture frame thread
        threadRunning_ = true;
        captureThread_ = std::make_unique<std::thread>(&AudioCapturerSourceInner::CaptureThreadLoop, this);

        std::string threadName = "OS_Capture";
        threadName += (attr_.sourceType == SOURCE_TYPE_EC) ? "Ec" : "MicRef";
        pthread_setname_np(captureThread_->native_handle(), threadName.c_str());
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Start(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);

    AUDIO_INFO_LOG("halName: %{public}s, sourceType: %{public}d", halName_.c_str(), attr_.sourceType);
    Trace trace("AudioCapturerSourceInner::Start");

    if (IsNonblockingSource(attr_.sourceType, adapterNameCase_)) {
        return StartNonblockingCapture();
    }

    InitLatencyMeasurement();
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (runningLockManager_ == nullptr) {
        WatchTimeout guard("PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock:Start");
        switch (attr_.sourceType) {
            case SOURCE_TYPE_WAKEUP:
                keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioWakeupCapturer",
                    PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
                break;
            case SOURCE_TYPE_MIC:
            case SOURCE_TYPE_CAMCORDER:
            case SOURCE_TYPE_UNPROCESSED:
            default:
                keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioPrimaryCapturer",
                    PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        }
        guard.CheckCurrTimeout();
        if (keepRunningLock) {
            runningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock lock result: %{public}d",
            runningLockManager_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING)); // -1 for lasting.
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, capture can not work well!");
    }
#endif
    // eg: primary_0_20240527202236189_source_44100_2_1.pcm
    dumpFileName_ = halName_ + "_" + std::to_string(attr_.sourceType) + "_" + GetTime()
        + "_source_" + std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel)
        + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    return StartCapture();
}

int32_t AudioCapturerSourceInner::StartCapture()
{
    if (!started_) {
        if (audioCapturerSourceCallback_ != nullptr) {
            audioCapturerSourceCallback_->OnCapturerState(true);
        }
        CHECK_AND_RETURN_RET_LOG(audioCapture_, ERR_INVALID_HANDLE, "audioCapture_ is nullptr");
        int32_t ret = audioCapture_->Start(audioCapture_);
        CHECK_AND_RETURN_RET(ret >= 0, ERR_NOT_STARTED);
        started_ = true;
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::SetVolume(float left, float right)
{
    float volume;
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "SetVolume failed audioCapture_ null");

    rightVolume_ = right;
    leftVolume_ = left;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    audioCapture_->SetVolume(audioCapture_, volume);

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_, ERR_INVALID_HANDLE, "audioCapture_ is nullptr");
    float val = 0.0;
    audioCapture_->GetVolume(audioCapture_, &val);
    left = val;
    right = val;

    return SUCCESS;
}

void AudioCapturerSourceInner::SetMuteState(bool isMute)
{
    std::lock_guard<std::mutex> statusLock(muteStateMutex_);
    muteState_ = isMute;
}

bool AudioCapturerSourceInner::GetMuteState()
{
    std::lock_guard<std::mutex> statusLock(muteStateMutex_);
    return muteState_;
}

int32_t AudioCapturerSourceInner::SetMute(bool isMute)
{
    SetMuteState(isMute);

    if (IsInited() && audioCapture_) {
        int32_t ret = audioCapture_->SetMute(audioCapture_, isMute);
        if (ret != 0) {
            AUDIO_WARNING_LOG("SetMute for hdi capturer failed");
        } else {
            AUDIO_INFO_LOG("SetMute for hdi capture success");
        }
    }

    if ((halName_ == "primary") && !adapterLoaded_) {
        InitManagerAndAdapter();
    }

    if (audioAdapter_ != nullptr) {
        int32_t ret = audioAdapter_->SetMicMute(audioAdapter_, isMute);
        if (ret != 0) {
            AUDIO_WARNING_LOG("SetMicMute for hdi adapter failed");
        } else {
            AUDIO_INFO_LOG("SetMicMute for hdi adapter success");
        }
    }

    AUDIO_INFO_LOG("halName:%{public}s isMute=%{public}d", halName_.c_str(), isMute);

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "GetMute failed audioCapture_ handle is null!");

    bool isHdiMute = false;
    int32_t ret = audioCapture_->GetMute(audioCapture_, &isHdiMute);
    if (ret != 0) {
        AUDIO_WARNING_LOG("GetMute failed from hdi");
    }

    isMute = GetMuteState();

    return SUCCESS;
}

static AudioCategory GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AUDIO_IN_COMMUNICATION;
            break;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_VOICE_RINGING:
            audioCategory = AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AUDIO_IN_MEDIA;
            break;
        default:
            audioCategory = AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("Audio category returned is: %{public}d", audioCategory);

    return audioCategory;
}

static int32_t SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source)
{
    int32_t ret = SUCCESS;

    switch (inputDevice) {
        case DEVICE_TYPE_MIC:
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_BLUETOOTH_A2DP_IN:
            source.ext.device.type = PIN_IN_MIC;
            source.ext.device.desc = (char *)"pin_in_mic";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            source.ext.device.type = PIN_IN_HS_MIC;
            source.ext.device.desc = (char *)"pin_in_hs_mic";
            break;
        case DEVICE_TYPE_USB_ARM_HEADSET:
            source.ext.device.type = PIN_IN_USB_HEADSET;
            source.ext.device.desc = (char *)"pin_in_usb_headset";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            source.ext.device.type = PIN_IN_USB_EXT;
            source.ext.device.desc = (char *)"pin_in_usb_ext";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            source.ext.device.type = PIN_IN_BLUETOOTH_SCO_HEADSET;
            source.ext.device.desc = (char *)"pin_in_bluetooth_sco_headset";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t AudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice, const std::string &deviceName)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AudioPortPin inputPortPin = PIN_IN_MIC;
    return SetInputRoute(inputDevice, inputPortPin, deviceName);
}

int32_t AudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin,
    const std::string &deviceName)
{
    if (inputDevice == currentActiveDevice_) {
        AUDIO_INFO_LOG("input device not change. currentActiveDevice %{public}d sourceType %{public}d",
            currentActiveDevice_, attr_.sourceType);
        return SUCCESS;
    }

    return DoSetInputRoute(inputDevice, inputPortPin, deviceName);
}

int32_t AudioCapturerSourceInner::DoSetInputRoute(DeviceType inputDevice,
    AudioPortPin &inputPortPin, const std::string &deviceName)
{
    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetInputPortPin(inputDevice, source);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoSetInputRoute FAILED: %{public}d", ret);

    inputPortPin = source.ext.device.type;
    AUDIO_INFO_LOG("Input PIN is: 0x%{public}X", inputPortPin);
    source.portId = static_cast<int32_t>(audioPort_.portId);
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_DEVICE_TYPE;
    source.ext.device.moduleId = 0;
    source.ext.device.desc = (char *)"";

    sink.portId = 0;
    sink.role = AUDIO_PORT_SINK_ROLE;
    sink.type = AUDIO_PORT_MIX_TYPE;
    sink.ext.mix.moduleId = 0;
    sink.ext.mix.streamId = static_cast<int32_t>(GenerateUniqueIDBySource(attr_.sourceType));
    sink.ext.mix.source = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));
    sink.ext.device.desc = (char *)"";

    AudioRoute route = {
        .sources = &source,
        .sourcesLen = 1,
        .sinks = &sink,
        .sinksLen = 1,
    };

    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_OPERATION_FAILED,
        "AudioAdapter object is null.");

    ret = audioAdapter_->UpdateAudioRoute(audioAdapter_, &route, &routeHandle_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "UpdateAudioRoute failed");

    currentActiveDevice_ = inputDevice;
    return SUCCESS;
}

int32_t AudioCapturerSourceInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice,
    const std::string &deviceName)
{
    AUDIO_INFO_LOG("SetAudioScene scene: %{public}d, device: %{public}d",
        audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "invalid audioScene");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "SetAudioScene failed audioCapture_ handle is null!");
    if (openMic_) {
        AudioPortPin audioSceneInPort = PIN_IN_MIC;
        if (halName_ == "usb") {
            audioSceneInPort = PIN_IN_USB_HEADSET;
        }

        int32_t ret = SUCCESS;
        if (audioScene != currentAudioScene_) {
            struct AudioSceneDescriptor scene;
            scene.scene.id = GetAudioCategory(audioScene);
            scene.desc.pins = audioSceneInPort;
            scene.desc.desc = const_cast<char *>("");

            ret = audioCapture_->SelectScene(audioCapture_, &scene);
            CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED,
                "Select scene FAILED: %{public}d", ret);
            currentAudioScene_ = audioScene;
        }

        std::lock_guard<std::mutex> statusLock(statusMutex_);
        ret = SetInputRoute(activeDevice, audioSceneInPort, deviceName);
        if (ret < 0) {
            AUDIO_WARNING_LOG("Update route FAILED: %{public}d", ret);
        }
    }
    AUDIO_DEBUG_LOG("Select audio scene SUCCESS: %{public}d", audioScene);
    return SUCCESS;
}

uint64_t AudioCapturerSourceInner::GetTransactionId()
{
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t AudioCapturerSourceInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("failed audioCapture_ is NULL");
        return ERR_INVALID_HANDLE;
    }
    struct AudioTimeStamp timestamp = {};
    int32_t ret = audioCapture_->GetCapturePosition(audioCapture_, &frames, &timestamp);
    if (ret != 0) {
        AUDIO_ERR_LOG("get position failed");
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

int32_t AudioCapturerSourceInner::StopNonblockingCapture()
{
    threadRunning_ = false;
    if (captureThread_ && captureThread_->joinable()) {
        captureThread_->join();
    }

    if (started_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->Stop(audioCapture_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("hdi stop capture failed");
        }
    }
    started_ = false;

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::DoStop()
{
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());

    Trace trace("AudioCapturerSourceInner::DoStop");

    if (IsNonblockingSource(attr_.sourceType, adapterNameCase_)) {
        return StopNonblockingCapture();
    }

    DeinitLatencyMeasurement();

#ifdef FEATURE_POWER_MANAGER
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unlock");
        runningLockManager_->UnLock();
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, stop can not work well!");
    }
#endif

    if (started_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->Stop(audioCapture_);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "Stop capture Failed");
    }
    started_ = false;

    if (audioCapturerSourceCallback_ != nullptr) {
        audioCapturerSourceCallback_->OnCapturerState(false);
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Stop(void)
{
    Trace trace("AudioCapturerSourceInner::Stop");
    std::promise<void> promiseEnsueThreadLock;
    auto futureWaitThreadLock = promiseEnsueThreadLock.get_future();
    std::thread threadAsyncStop([&promiseEnsueThreadLock, this] {
        std::lock_guard<std::mutex> statusLock(statusMutex_);
        promiseEnsueThreadLock.set_value();
        DoStop();
    });
    futureWaitThreadLock.get();
    threadAsyncStop.detach();

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Pause(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());

    Trace trace("AudioCapturerSourceInner::Pause");
    if (started_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->Pause(audioCapture_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "pause capture Failed");
    }
    paused_ = true;

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Resume(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("AudioCapturerSourceInner::Resume");
    if (paused_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->Resume(audioCapture_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "resume capture Failed");
    }
    paused_ = false;

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Reset(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("AudioCapturerSourceInner::Reset");
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->Flush(audioCapture_);
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Flush(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("AudioCapturerSourceInner::Flush");
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->Flush(audioCapture_);
    }

    return SUCCESS;
}

void AudioCapturerSourceInner::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    AUDIO_INFO_LOG("Register WakeupClose Callback");
    std::lock_guard<std::mutex> lck(wakeupClosecallbackMutex_);
    wakeupCloseCallback_ = callback;
}

void AudioCapturerSourceInner::RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback)
{
    AUDIO_INFO_LOG("Register AudioCapturerSource Callback");
    audioCapturerSourceCallback_ = std::move(callback);
}

void AudioCapturerSourceInner::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("RegisterParameterCallback is not supported!");
}

int32_t AudioCapturerSourceInner::Preload(const std::string &usbInfoStr)
{
    CHECK_AND_RETURN_RET_LOG(halName_ == "usb", ERR_INVALID_OPERATION, "Preload only supported for usb");

    std::lock_guard<std::mutex> statusLock(statusMutex_);
    int32_t ret = UpdateUsbAttrs(usbInfoStr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Preload failed when init attr");

    ret = InitAdapterAndCapture();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Preload failed when init adapter and capture");

    return SUCCESS;
}

static HdiAdapterFormat ParseAudioFormat(const std::string &format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return HdiAdapterFormat::SAMPLE_S16;
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT" || format == "AUDIO_FORMAT_PCM_24_BIT_PACKED") {
        return HdiAdapterFormat::SAMPLE_S24;
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return HdiAdapterFormat::SAMPLE_S32;
    } else {
        return HdiAdapterFormat::SAMPLE_S16;
    }
}

int32_t AudioCapturerSourceInner::UpdateUsbAttrs(const std::string &usbInfoStr)
{
    CHECK_AND_RETURN_RET_LOG(usbInfoStr != "", ERR_INVALID_PARAM, "usb info string error");

    auto sourceRate_begin = usbInfoStr.find("source_rate:");
    auto sourceRate_end = usbInfoStr.find_first_of(";", sourceRate_begin);
    std::string sampleRateStr = usbInfoStr.substr(sourceRate_begin + std::strlen("source_rate:"),
        sourceRate_end - sourceRate_begin - std::strlen("source_rate:"));
    auto sourceFormat_begin = usbInfoStr.find("source_format:");
    auto sourceFormat_end = usbInfoStr.find_first_of(";", sourceFormat_begin);
    std::string formatStr = usbInfoStr.substr(sourceFormat_begin + std::strlen("source_format:"),
        sourceFormat_end - sourceFormat_begin - std::strlen("source_format:"));

    // usb default config
    CHECK_AND_RETURN_RET_LOG(StringConverter(sampleRateStr, attr_.sampleRate), ERR_INVALID_PARAM,
        "convert invalid sampleRate: %{public}s", sampleRateStr.c_str());
    attr_.channel = STEREO_CHANNEL_COUNT;
    attr_.format = ParseAudioFormat(formatStr);
    attr_.isBigEndian = false;
    attr_.bufferSize = USB_DEFAULT_BUFFERSIZE;
    attr_.sourceType = SOURCE_TYPE_MIC;

    adapterNameCase_ = "usb";
    openMic_ = 0;

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::InitManagerAndAdapter()
{
    int32_t err = InitAudioManager();
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_NOT_STARTED, "Init audio manager Fail");

    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, (struct AudioAdapterDescriptor *)&descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && ret == 0,
        ERR_NOT_STARTED, "Get adapters Fail");
    if (adapterNameCase_ == "" && halName_ == "primary") {
        adapterNameCase_ = "primary";
    }
    // Get qualified sound card and port
    int32_t index = SwitchAdapterCapture((struct AudioAdapterDescriptor *)&descs,
        size, adapterNameCase_, PORT_IN, audioPort_);
    CHECK_AND_RETURN_RET_LOG(index >= 0, ERR_NOT_STARTED, "Switch Adapter Capture Fail");
    adapterDesc_ = descs[index];

    if (audioAdapter_ == nullptr) {
        struct IAudioAdapter *iAudioAdapter = nullptr;
        int32_t loadAdapter = audioManager_->LoadAdapter(audioManager_, &adapterDesc_, &iAudioAdapter);
        CHECK_AND_RETURN_RET_LOG(loadAdapter == 0, ERR_NOT_STARTED, "Load Adapter Fail");
        CHECK_AND_RETURN_RET_LOG(iAudioAdapter != nullptr, ERR_NOT_STARTED, "Load audio device failed");

        // Inittialization port information, can fill through mode and other parameters
        int32_t initAllPorts = iAudioAdapter->InitAllPorts(iAudioAdapter);
        CHECK_AND_RETURN_RET_LOG(initAllPorts == 0, ERR_DEVICE_INIT, "InitAllPorts failed");
        audioAdapter_ = iAudioAdapter;
        adapterLoaded_ = true;
    }
    return SUCCESS;
}

int32_t AudioCapturerSourceInner::InitAdapterAndCapture()
{
    AUDIO_INFO_LOG("Init adapter start sourceName %{public}s", halName_.c_str());

    if (captureInited_) {
        AUDIO_INFO_LOG("Adapter already inited");
        return SUCCESS;
    }

    int32_t err = InitManagerAndAdapter();
    CHECK_AND_RETURN_RET_LOG(err == 0, err, "Init audio manager and adapater failed");

    int32_t createCapture = CreateCapture(audioPort_);
    CHECK_AND_RETURN_RET_LOG(createCapture == 0, ERR_NOT_STARTED, "Create capture failed");
    if (openMic_) {
        int32_t ret;
        AudioPortPin inputPortPin = PIN_IN_MIC;
        if (halName_ == "usb") {
            ret = SetInputRoute(DEVICE_TYPE_USB_ARM_HEADSET, inputPortPin);
        } else {
            DeviceType deviceType = static_cast<DeviceType>(attr_.deviceType);
            ret = SetInputRoute(deviceType, inputPortPin);
        }
        if (ret < 0) {
            AUDIO_WARNING_LOG("update route FAILED: %{public}d", ret);
        }
        ret = SetAudioRouteInfoForEnhanceChain(currentActiveDevice_, "");
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set device %{public}d failed", currentActiveDevice_);
        }
    }

    captureInited_ = true;

    return SUCCESS;
}

std::string AudioCapturerSourceInner::GetAudioParameter(const AudioParamKey key,
                                                        const std::string &condition)
{
    AUDIO_WARNING_LOG("not supported yet");
    return "";
}

void AudioCapturerSourceInner::InitLatencyMeasurement()
{
    std::lock_guard<std::mutex> lock(signalDetectAgentMutex_);

    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    latencyMeasEnabled_ = true;
}

void AudioCapturerSourceInner::DeinitLatencyMeasurement()
{
    std::lock_guard<std::mutex> lock(signalDetectAgentMutex_);

    signalDetected_ = false;
    signalDetectAgent_ = nullptr;
}

void AudioCapturerSourceInner::CheckLatencySignal(uint8_t *frame, size_t replyBytes)
{
    std::lock_guard<std::mutex> lock(signalDetectAgentMutex_);
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetected_ = signalDetectAgent_->CheckAudioData(frame, replyBytes);
    if (signalDetected_) {
        char value[GET_EXTRA_PARAM_LEN];
        AudioParamKey key = NONE;
        AudioExtParamKey hdiKey = AudioExtParamKey(key);
        std::string condition = "debug_audio_latency_measurement";
        int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey, condition.c_str(),
            value, DumpFileUtil::PARAM_VALUE_LENTH);
        AUDIO_INFO_LOG("GetExtraParam ret:%{public}d", ret);
        LatencyMonitor::GetInstance().UpdateDspTime(value);
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false,
            signalDetectAgent_->lastPeakBufferTime_);
        AUDIO_INFO_LOG("LatencyMeas primarySource signal detected");
        signalDetected_ = false;
    }
}

int32_t AudioCapturerSourceInner::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    if (!runningLockManager_) {
        return ERROR;
    }

    runningLockManager_->UpdateAppsUid(appsUid, appsUid + size);
    runningLockManager_->UpdateAppsUidToPowerMgr();
#endif

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
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

int32_t AudioCapturerSourceInner::GetCaptureId(uint32_t &captureId) const
{
    if (halName_ == "usb") {
        captureId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_USB);
    } else {
        captureId = GenerateUniqueIDBySource(attr_.sourceType);
    }
    return SUCCESS;
}

int32_t AudioCapturerSourceInner::SetAudioRouteInfoForEnhanceChain(const DeviceType &inputDevice,
    const std::string &deviceName)
{
    if (IsNonblockingSource(attr_.sourceType, attr_.adapterName)) {
        AUDIO_ERR_LOG("non blocking source not support SetAudioRouteInfoForEnhanceChain");
        return SUCCESS;
    }
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    uint32_t captureId = 0;
    int32_t ret = GetCaptureId(captureId);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("GetCaptureId failed");
    }
    if (halName_ == "usb") {
        audioEnhanceChainManager->SetInputDevice(captureId, DEVICE_TYPE_USB_ARM_HEADSET, deviceName);
    } else {
        audioEnhanceChainManager->SetInputDevice(captureId, inputDevice, deviceName);
    }
    return SUCCESS;
}

int32_t AudioCapturerSourceInner::UpdateSourceType(SourceType sourceType)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    if (attr_.sourceType == sourceType) {
        AUDIO_INFO_LOG("input sourceType not change. currentActiveDevice %{public}d sourceType %{public}d",
            currentActiveDevice_, attr_.sourceType);
        return SUCCESS;
    }

    attr_.sourceType = sourceType;
    AUDIO_INFO_LOG("change source type to %{public}d", attr_.sourceType);
    AudioPortPin inputPortPin = PIN_IN_MIC;
    return DoSetInputRoute(currentActiveDevice_, inputPortPin);
}

void AudioCapturerSourceInner::SetAddress(const std::string &address)
{
    address_ = address;
}

int32_t AudioCapturerSourceWakeup::Init(const IAudioSourceAttr &attr)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (isInited) {
        return res;
    }
    noStart_ = 0;
    if (initCount == 0) {
        if (wakeupBuffer_ == nullptr) {
            wakeupBuffer_ = std::make_unique<WakeupBuffer>();
        }
        res = audioCapturerSource_.Init(attr);
    }
    if (res == SUCCESS) {
        isInited = true;
        initCount++;
    }
    return res;
}

bool AudioCapturerSourceWakeup::IsInited(void)
{
    return isInited;
}

void AudioCapturerSourceWakeup::DeInit(void)
{
    AudioXCollie wakeupXCollie("AudioCapturerSourceWakeup::DeInit", DEINIT_TIME_OUT_SECONDS);
    AUDIO_INFO_LOG("Start deinit of source wakeup");
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (!isInited) {
        return;
    }
    isInited = false;
    initCount--;
    if (initCount == 0) {
        wakeupBuffer_.reset();
        audioCapturerSource_.DeInit();
    }
}

int32_t AudioCapturerSourceWakeup::Start(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (isStarted) {
        return res;
    }
    if (startCount == 0) {
        res = audioCapturerSource_.Start();
    }
    if (res == SUCCESS) {
        isStarted = true;
        startCount++;
    }
    return res;
}

int32_t AudioCapturerSourceWakeup::Stop(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (!isStarted) {
        return res;
    }
    if (startCount == 1) {
        res = audioCapturerSource_.Stop();
    }
    if (res == SUCCESS) {
        isStarted = false;
        startCount--;
    }
    return res;
}

int32_t AudioCapturerSourceWakeup::Flush(void)
{
    return audioCapturerSource_.Flush();
}

int32_t AudioCapturerSourceWakeup::Reset(void)
{
    return audioCapturerSource_.Reset();
}

int32_t AudioCapturerSourceWakeup::Pause(void)
{
    return audioCapturerSource_.Pause();
}

int32_t AudioCapturerSourceWakeup::Resume(void)
{
    return audioCapturerSource_.Resume();
}

int32_t AudioCapturerSourceWakeup::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int32_t res = wakeupBuffer_->Poll(frame, requestBytes, replyBytes, noStart_);
    noStart_ += replyBytes;
    return res;
}

int32_t AudioCapturerSourceWakeup::CaptureFrameWithEc(
    FrameDesc *fdesc, uint64_t &replyBytes,
    FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    AUDIO_ERR_LOG("not supported!");
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t AudioCapturerSourceWakeup::SetVolume(float left, float right)
{
    return audioCapturerSource_.SetVolume(left, right);
}

int32_t AudioCapturerSourceWakeup::GetVolume(float &left, float &right)
{
    return audioCapturerSource_.GetVolume(left, right);
}

int32_t AudioCapturerSourceWakeup::SetMute(bool isMute)
{
    return audioCapturerSource_.SetMute(isMute);
}

int32_t AudioCapturerSourceWakeup::GetMute(bool &isMute)
{
    return audioCapturerSource_.GetMute(isMute);
}

int32_t AudioCapturerSourceWakeup::SetAudioScene(AudioScene audioScene, DeviceType activeDevice,
    const std::string &deviceName)
{
    return audioCapturerSource_.SetAudioScene(audioScene, activeDevice);
}

int32_t AudioCapturerSourceWakeup::SetInputRoute(DeviceType inputDevice, const std::string &deviceName)
{
    return audioCapturerSource_.SetInputRoute(inputDevice);
}

uint64_t AudioCapturerSourceWakeup::GetTransactionId()
{
    return audioCapturerSource_.GetTransactionId();
}

int32_t AudioCapturerSourceWakeup::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    return audioCapturerSource_.GetPresentationPosition(frames, timeSec, timeNanoSec);
}

std::string AudioCapturerSourceWakeup::GetAudioParameter(const AudioParamKey key,
                                                         const std::string &condition)
{
    AUDIO_WARNING_LOG("not supported yet");
    return "";
}

void AudioCapturerSourceWakeup::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    audioCapturerSource_.RegisterWakeupCloseCallback(callback);
}

void AudioCapturerSourceWakeup::RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback)
{
    audioCapturerSource_.RegisterAudioCapturerSourceCallback(std::move(callback));
}

void AudioCapturerSourceWakeup::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("AudioCapturerSourceWakeup: RegisterParameterCallback is not supported!");
}

float AudioCapturerSourceWakeup::GetMaxAmplitude()
{
    return audioCapturerSource_.GetMaxAmplitude();
}

int32_t AudioCapturerSourceWakeup::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
    return audioCapturerSource_.UpdateAppsUid(appsUid, size);
}

int32_t AudioCapturerSourceWakeup::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return audioCapturerSource_.UpdateAppsUid(appsUid);
}

int32_t AudioCapturerSourceWakeup::GetCaptureId(uint32_t &captureId) const
{
    captureId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_WAKEUP);
    return SUCCESS;
}
} // namespace AudioStandard
} // namesapce OHOS
