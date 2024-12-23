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
#define LOG_TAG "RemoteFastAudioCapturerSourceInner"
#endif

#include "remote_fast_audio_capturer_source.h"

#include <cinttypes>
#include <dlfcn.h>
#include <sstream>
#include "securec.h"

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include "i_audio_device_adapter.h"
#include "i_audio_device_manager.h"

using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortDirection;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioManager;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCategory;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRouteNode;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRoute;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortPin;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortType;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortRole;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioMmapBufferDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioTimeStamp;

namespace OHOS {
namespace AudioStandard {
class RemoteFastAudioCapturerSourceInner : public RemoteFastAudioCapturerSource, public IAudioDeviceAdapterCallback {
public:
    explicit RemoteFastAudioCapturerSourceInner(const std::string &deviceNetworkId);
    ~RemoteFastAudioCapturerSourceInner();

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
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;
    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice) override;
    int32_t SetInputRoute(DeviceType inputDevice) override;
    uint64_t GetTransactionId() override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;
    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    int32_t CheckPositionTime();

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;
    
    float GetMaxAmplitude() override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

private:
    int32_t CreateCapture(const struct AudioPort &capturePort);
    void InitAttrs(struct AudioSampleAttributes &attrs);
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    int32_t InitAshmem(const struct AudioSampleAttributes &attrs);
    AudioCategory GetAudioCategory(AudioScene audioScene);
    int32_t SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source);
    uint32_t PcmFormatToBits(HdiAdapterFormat format);
    void ClearCapture();

private:
    static constexpr int32_t INVALID_FD = -1;
    static constexpr int32_t INVALID_INDEX = -1;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t CAPTURE_INTERLEAVED = 1;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t DEEP_BUFFER_CAPTURER_PERIOD_SIZE = 3840;
    static constexpr uint32_t INT_32_MAX = 0x7fffffff;
    static constexpr uint32_t REMOTE_FAST_INPUT_STREAM_ID = 38; // 14 + 3 * 8
    static constexpr int32_t EVENT_DES_SIZE = 60;
    static constexpr int64_t SECOND_TO_NANOSECOND = 1000000000;
    static constexpr int64_t CAPTURE_FIRST_FRIME_WAIT_NANO = 20000000; // 20ms
    static constexpr int64_t CAPTURE_RESYNC_SLEEP_NANO = 2000000; // 2ms
    static constexpr  uint32_t PCM_8_BIT = 8;
    static constexpr  uint32_t PCM_16_BIT = 16;
    static constexpr  uint32_t PCM_24_BIT = 24;
    static constexpr  uint32_t PCM_32_BIT = 32;

    std::atomic<bool> micMuteState_ = false;
    std::atomic<bool> capturerInited_ = false;
    std::atomic<bool> isCapturerCreated_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    float leftVolume_ = 0;
    float rightVolume_ = 0;
    int32_t bufferFd_ = -1;
    uint32_t bufferTotalFrameSize_ = 0;
    uint32_t eachReadFrameSize_ = 0;
    std::shared_ptr<IAudioDeviceManager> audioManager_ = nullptr;
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter_ = nullptr;
    IAudioSourceCallback *paramCb_ = nullptr;
    sptr<IAudioCapture> audioCapture_ = nullptr;
    struct AudioPort audioPort_ = {};
    IAudioSourceAttr attr_ = {};
    std::string deviceNetworkId_ = "";
    uint32_t captureId_ = 0;

#ifdef DEBUG_DIRECT_USE_HDI
    sptr<Ashmem> ashmemSource_ = nullptr;
    int32_t ashmemLen_ = 0;
    int32_t lenPerRead_ = 0;
    const char *audioFilePath = "/data/local/tmp/remote_fast_audio_capture.pcm";
    FILE *pfd_ = nullptr;
#endif // DEBUG_DIRECT_USE_HDI
};

std::mutex g_remoteFastSourcesMutex;
std::map<std::string, RemoteFastAudioCapturerSourceInner *> allRFSources;
IMmapAudioCapturerSource *RemoteFastAudioCapturerSource::GetInstance(const std::string &deviceNetworkId)
{
    std::lock_guard<std::mutex> lock(g_remoteFastSourcesMutex);
    AUDIO_INFO_LOG("GetInstance.");
    bool isEmpty = deviceNetworkId.empty();
    CHECK_AND_RETURN_RET_LOG(!isEmpty, nullptr, "Remote capture device networkId is null.");

    if (allRFSources.count(deviceNetworkId)) {
        return allRFSources[deviceNetworkId];
    }
    RemoteFastAudioCapturerSourceInner *rfCapturer =
        new(std::nothrow) RemoteFastAudioCapturerSourceInner(deviceNetworkId);
    AUDIO_DEBUG_LOG("New daudio remote fast capture device networkId: [%{public}s].", deviceNetworkId.c_str());
    allRFSources[deviceNetworkId] = rfCapturer;
    return rfCapturer;
}

RemoteFastAudioCapturerSourceInner::RemoteFastAudioCapturerSourceInner(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSource Constract.");
}

RemoteFastAudioCapturerSourceInner::~RemoteFastAudioCapturerSourceInner()
{
    if (capturerInited_.load()) {
        RemoteFastAudioCapturerSourceInner::DeInit();
    }
    AUDIO_INFO_LOG("~RemoteFastAudioCapturerSource end.");
}

bool RemoteFastAudioCapturerSourceInner::IsInited()
{
    return capturerInited_.load();
}

void RemoteFastAudioCapturerSourceInner::ClearCapture()
{
    AUDIO_INFO_LOG("Clear capture enter.");
    capturerInited_.store(false);
    isCapturerCreated_.store(false);
    started_.store(false);
    paused_.store(false);
    micMuteState_.store(false);

#ifdef DEBUG_DIRECT_USE_HDI
    if (pfd_) {
        fclose(pfd_);
        pfd_ = nullptr;
    }
    if (ashmemSource_ != nullptr) {
        ashmemSource_->UnmapAshmem();
        ashmemSource_->CloseAshmem();
        ashmemSource_ = nullptr;
        AUDIO_DEBUG_LOG("ClearCapture: Uninit source ashmem OK.");
    }
#endif // DEBUG_DIRECT_USE_HDI

    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyCapture(audioCapture_, captureId_);
        audioAdapter_->Release();
    }
    audioCapture_ = nullptr;
    audioAdapter_ = nullptr;

    if (audioManager_ != nullptr) {
        audioManager_->UnloadAdapter(deviceNetworkId_);
    }
    audioManager_ = nullptr;

    AudioDeviceManagerFactory::GetInstance().DestoryDeviceManager(REMOTE_DEV_MGR);
    AUDIO_DEBUG_LOG("Clear capture end.");
}

void RemoteFastAudioCapturerSourceInner::DeInit()
{
    std::lock_guard<std::mutex> lock(g_remoteFastSourcesMutex);
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::DeInit");
    ClearCapture();

    // remove map recorder.
    CHECK_AND_RETURN_LOG(allRFSources.count(this->deviceNetworkId_) > 0,
        "not find %{public}s", this->deviceNetworkId_.c_str());
    RemoteFastAudioCapturerSource *temp = allRFSources[this->deviceNetworkId_];
    allRFSources.erase(this->deviceNetworkId_);
    if (temp == nullptr) {
        AUDIO_ERR_LOG("temp is nullptr");
    } else {
        delete temp;
    }
}

int32_t RemoteFastAudioCapturerSourceInner::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSource::Init");
    attr_ = attr;
    audioManager_ = AudioDeviceManagerFactory::GetInstance().CreatDeviceManager(REMOTE_DEV_MGR);
    CHECK_AND_RETURN_RET_LOG(audioManager_ != nullptr, ERR_NOT_STARTED, "Init audio manager fail.");

    struct AudioAdapterDescriptor *desc = audioManager_->GetTargetAdapterDesc(deviceNetworkId_, true);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NOT_STARTED, "Get target adapters descriptor fail.");
    for (uint32_t port = 0; port < desc->ports.size(); port++) {
        if (desc->ports[port].portId == AudioPortPin::PIN_IN_MIC) {
            audioPort_ = desc->ports[port];
            break;
        }
        CHECK_AND_RETURN_RET_LOG(port != (desc->ports.size() - 1), ERR_INVALID_INDEX,
            "Not found the audio mic port.");
    }

    audioAdapter_ = audioManager_->LoadAdapters(deviceNetworkId_, true);
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device adapter failed.");

    int32_t ret = audioAdapter_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Audio adapter init fail, ret %{public}d.", ret);

    if (!isCapturerCreated_.load()) {
        CHECK_AND_RETURN_RET_LOG(CreateCapture(audioPort_) == SUCCESS, ERR_NOT_STARTED,
            "Create capture failed, Audio Port: %{public}d.", audioPort_.portId);
    }
    capturerInited_.store(true);

#ifdef DEBUG_DIRECT_USE_HDI
    AUDIO_INFO_LOG("Dump audio source attr: [%{public}s]", PrintRemoteAttr(attr_).c_str());
    pfd_ = fopen(audioFilePath, "a+"); // here will not create a file if not exit.
    AUDIO_INFO_LOG("Init dump file [%{public}s]", audioFilePath);
    if (pfd_ == nullptr) {
        AUDIO_WARNING_LOG("Opening remote pcm file [%{public}s] fail.", audioFilePath);
    }
#endif // DEBUG_DIRECT_USE_HDI

    AUDIO_INFO_LOG("Init end.");
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::CreateCapture(const struct AudioPort &capturePort)
{
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE, "CreateCapture: audio adapter is null.");
    struct AudioSampleAttributes param;
    InitAttrs(param);
    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AudioCategory::AUDIO_MMAP_VOIP :
        AudioCategory::AUDIO_MMAP_NOIRQ;

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = AudioPortPin::PIN_IN_MIC;
    deviceDesc.desc = "";
    int32_t ret = audioAdapter_->CreateCapture(deviceDesc, param, audioCapture_, this, captureId_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && audioCapture_ != nullptr, ret,
        "Create capture fail, ret %{public}d.", ret);
    if (param.type == AudioCategory::AUDIO_MMAP_NOIRQ || param.type == AudioCategory::AUDIO_MMAP_VOIP) {
        InitAshmem(param); // The remote fast source first start
    }

    isCapturerCreated_.store(true);
    AUDIO_INFO_LOG("Create capture end, capture format: %{public}d.", param.format);
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::InitAshmem(const struct AudioSampleAttributes &attrs)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "InitAshmem: Audio capture is null.");

    int32_t totalBufferInMs = 40; // 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency.
    int32_t reqSize = totalBufferInMs * static_cast<int32_t>(attr_.sampleRate / 1000);

    struct AudioMmapBufferDescriptor desc;
    int32_t ret = audioCapture_->ReqMmapBuffer(reqSize, desc);
    CHECK_AND_RETURN_RET_LOG((ret == 0), ERR_OPERATION_FAILED,
        "InitAshmem require mmap buffer failed, ret %{public}d.", ret);
    AUDIO_DEBUG_LOG("InitAshmem audio capture mmap buffer info, memoryFd[%{public}d] "
        "totalBufferFrames [%{public}d] transferFrameSize[%{public}d] isShareable[%{public}d] offset[%{public}d]",
        desc.memoryFd, desc.totalBufferFrames, desc.transferFrameSize,
        desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd;
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "ReqMmapBuffer invalid values: totalBufferFrames[%{public}d] transferFrameSize[%{public}d]",
        desc.totalBufferFrames, desc.transferFrameSize);
    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames);
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize);

#ifdef DEBUG_DIRECT_USE_HDI
    ashmemLen_ = desc.totalBufferFrames * attrs.channelCount * attrs.format;
    ashmemSource_ = new Ashmem(bufferFd_, ashmemLen_);
    AUDIO_DEBUG_LOG("InitAshmem creat ashmem source OK, ashmemLen %{public}d.", ashmemLen_);
    bool tmp = ashmemSource_->MapReadAndWriteAshmem();
    CHECK_AND_RETURN_RET_LOG(tmp, ERR_OPERATION_FAILED, "InitAshmem map ashmem source failed.");
    lenPerRead_ = desc.transferFrameSize * attrs.channelCount * attrs.format;
    AUDIO_DEBUG_LOG("InitAshmem map ashmem source OK, lenPerWrite %{public}d.", lenPerRead_);
#endif
    return SUCCESS;
}

void RemoteFastAudioCapturerSourceInner::InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.interleaved = CAPTURE_INTERLEAVED;
    attrs.format = ConvertToHdiFormat(attr_.format);
    attrs.sampleRate = attr_.sampleRate;
    attrs.channelCount = attr_.channel;
    attrs.period = DEEP_BUFFER_CAPTURER_PERIOD_SIZE;
    attrs.frameSize = attrs.format * attrs.channelCount;
    attrs.isBigEndian = attr_.isBigEndian;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_CAPTURER_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = attr_.bufferSize;
    attrs.streamId = REMOTE_FAST_INPUT_STREAM_ID;
}

AudioFormat RemoteFastAudioCapturerSourceInner::ConvertToHdiFormat(HdiAdapterFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

inline std::string PrintRemoteAttr(const IAudioSourceAttr &attr)
{
    std::stringstream value;
    value << "adapterName[" << attr.adapterName << "] openMicSpeaker[" << attr.openMicSpeaker << "] ";
    value << "format[" << static_cast<int32_t>(attr.format) << "]";
    value << "sampleRate[" << attr.sampleRate << "] channel[" << attr.channel << "] ";
    value << "volume[" << attr.volume << "] filePath[" << attr.filePath << "] ";
    value << "deviceNetworkId[" << attr.deviceNetworkId << "] device_type[" << attr.deviceType << "]";
    return value.str();
}

int32_t RemoteFastAudioCapturerSourceInner::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe,
    uint32_t &spanSizeInframe, uint32_t &byteSizePerFrame)
{
    CHECK_AND_RETURN_RET_LOG(bufferFd_ != INVALID_FD, ERR_INVALID_HANDLE,
        "buffer fd has been released!");
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec,
    int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "GetMmapHandlePosition: Audio capture is null.");

    struct AudioTimeStamp timestamp;
    int32_t ret = audioCapture_->GetMmapPosition(frames, timestamp);
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

int32_t RemoteFastAudioCapturerSourceInner::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    AUDIO_DEBUG_LOG("Capture frame is not supported.");
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::CheckPositionTime()
{
    int32_t tryCount = 10;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int64_t maxHandleCost = 10000000; // ns
    int64_t waitTime = 10000000; // 10ms
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(waitTime); // us
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > maxHandleCost) {
            AUDIO_WARNING_LOG("CheckPositionTime[%{public}d]:ret %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("CheckPositionTime end, position and time is ok.");
            return SUCCESS;
        }
    }
    return ERROR;
}

int32_t RemoteFastAudioCapturerSourceInner::Start(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::Start");
    if (!isCapturerCreated_.load()) {
        int32_t err = CreateCapture(audioPort_);
        CHECK_AND_RETURN_RET_LOG(err == SUCCESS, ERR_NOT_STARTED,
            "Create capture failed, Audio Port: %{public}d.", audioPort_.portId);
    }

    if (started_.load()) {
        AUDIO_INFO_LOG("Remote fast capturer is already start.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Start: Audio capture is null.");
    int32_t ret = audioCapture_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "Remote fast capturer start fail, ret %{public}d.", ret);

    ret = CheckPositionTime();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED,
        "Remote fast capturer check position time fail, ret %{public}d.", ret);
    started_.store(true);

    AUDIO_DEBUG_LOG("Start OK.");
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::Stop(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::Stop");
    if (!started_.load()) {
        AUDIO_INFO_LOG("Remote capture is already stopped.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Stop: Audio capture is null.");
    int32_t ret = audioCapture_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Stop fail, ret %{public}d.", ret);
    started_.store(false);
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::Pause(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::Pause");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Pause invalid state!");

    if (paused_.load()) {
        AUDIO_INFO_LOG("Remote render is already paused.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Pause: Audio capture is null.");
    int32_t ret = audioCapture_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Pause fail, ret %{public}d.", ret);
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::Resume(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::Resume");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Resume invalid state!");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("Remote render is already resumed.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Resume: Audio capture is null.");
    int32_t ret = audioCapture_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Resume fail, ret %{public}d.", ret);
    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::Reset(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::Reset");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Reset invalid state!");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Reset: Audio capture is null.");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Reset fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::Flush(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioCapturerSourceInner::Flush");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Flush invalid state!");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Flush: Audio capture is null.");

    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Flush fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::SetVolume(float left, float right)
{
    AUDIO_INFO_LOG("Set volume enter, left %{public}f, right %{public}f.", left, right);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "SetVolume: Audio capture is null.");

    float volume;
    leftVolume_ = left;
    rightVolume_ = right;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioCapture_->SetVolume(volume);
    if (ret) {
        AUDIO_ERR_LOG("Remote fast capturer set volume fail, ret %{public}d.", ret);
    }
    return ret;
}

int32_t RemoteFastAudioCapturerSourceInner::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "GetVolume: Audio capture is null.");
    float val = 0;
    audioCapture_->GetVolume(val);
    left = val;
    right = val;
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::SetMute(bool isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "SetMute: Audio capture is null.");
    int32_t ret = audioCapture_->SetMute(isMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("Remote fast capturer set mute fail, ret %{public}d.", ret);
    }

    micMuteState_ = isMute;
    return SUCCESS;
}

int32_t RemoteFastAudioCapturerSourceInner::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "GetMute: Audio capture is null.");
    bool isHdiMute = false;
    int32_t ret = audioCapture_->GetMute(isHdiMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("Remote fast capturer get mute fail, ret %{public}d.", ret);
    }

    isMute = micMuteState_;
    return SUCCESS;
}

uint64_t RemoteFastAudioCapturerSourceInner::GetTransactionId()
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioCapturerSourceInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec,
    int64_t& timeNanoSec)
{
    return ERR_OPERATION_FAILED;
}

std::string RemoteFastAudioCapturerSourceInner::GetAudioParameter(const AudioParamKey key,
                                                                  const std::string &condition)
{
    AUDIO_WARNING_LOG("not supported yet");
    return "";
}

int32_t RemoteFastAudioCapturerSourceInner::SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source)
{
    int32_t ret = SUCCESS;
    switch (inputDevice) {
        case DEVICE_TYPE_MIC:
            source.ext.device.type = AudioPortPin::PIN_IN_MIC;
            source.ext.device.desc = "pin_in_mic";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            source.ext.device.type = AudioPortPin::PIN_IN_HS_MIC;
            source.ext.device.desc = "pin_in_hs_mic";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            source.ext.device.type = AudioPortPin::PIN_IN_USB_EXT;
            source.ext.device.desc = "pin_in_usb_ext";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }
    return ret;
}

int32_t RemoteFastAudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice)
{
    AudioRouteNode source = {};
    AudioRouteNode sink = {};
    int32_t ret = SetInputPortPin(inputDevice, source);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set input port pin fail, ret %{public}d", ret);

    source.portId = static_cast<int32_t>(audioPort_.portId);
    source.role = AudioPortRole::AUDIO_PORT_SOURCE_ROLE;
    source.type = AudioPortType::AUDIO_PORT_DEVICE_TYPE;
    source.ext.device.moduleId = 0;

    sink.portId = 0;
    sink.role = AudioPortRole::AUDIO_PORT_SINK_ROLE;
    sink.type = AudioPortType::AUDIO_PORT_MIX_TYPE;
    sink.ext.mix.moduleId = 0;
    sink.ext.mix.streamId = REMOTE_FAST_INPUT_STREAM_ID;

    AudioRoute route;
    route.sources.push_back(source);
    route.sinks.push_back(sink);

    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE, "SetInputRoute: Audio adapter is null.");
    ret = audioAdapter_->UpdateAudioRoute(route);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Update audio route fail, ret %{public}d", ret);
    return SUCCESS;
}

AudioCategory RemoteFastAudioCapturerSourceInner::GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AudioCategory::AUDIO_IN_MEDIA;
            break;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_VOICE_RINGING:
            audioCategory = AudioCategory::AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AudioCategory::AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AudioCategory::AUDIO_IN_COMMUNICATION;
            break;
        default:
            audioCategory = AudioCategory::AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("RemoteFastAudioCapturerSource: Audio category returned is: %{public}d", audioCategory);

    return audioCategory;
}

int32_t RemoteFastAudioCapturerSourceInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    AUDIO_INFO_LOG("SetAudioScene enter: scene: %{public}d, device %{public}d.", audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "SetAudioScene: Audio capture is null.");
    struct AudioSceneDescriptor scene;
    scene.scene.id = GetAudioCategory(audioScene);
    scene.desc.pins = AudioPortPin::PIN_IN_MIC;
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "invalid audioScene");

    AUDIO_DEBUG_LOG("AudioCapturerSource::SelectScene start");
    int32_t ret = audioCapture_->SelectScene(scene);
    AUDIO_DEBUG_LOG("AudioCapturerSource::SelectScene over");
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED,
        "AudioCapturerSource: Select scene FAILED: %{public}d", ret);
    AUDIO_DEBUG_LOG("AudioCapturerSource::Select audio scene SUCCESS: %{public}d", audioScene);
    return SUCCESS;
}

uint32_t RemoteFastAudioCapturerSourceInner::PcmFormatToBits(HdiAdapterFormat format)
{
    switch (format) {
        case HdiAdapterFormat::SAMPLE_U8:
            return PCM_8_BIT;
        case HdiAdapterFormat::SAMPLE_S16:
            return PCM_16_BIT;
        case HdiAdapterFormat::SAMPLE_S24:
            return PCM_24_BIT;
        case HdiAdapterFormat::SAMPLE_S32:
            return PCM_32_BIT;
        case HdiAdapterFormat::SAMPLE_F32:
            return PCM_32_BIT;
        default:
            return PCM_16_BIT;
    }
}

void RemoteFastAudioCapturerSourceInner::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("RegisterWakeupCloseCallback FAILED");
}

void RemoteFastAudioCapturerSourceInner::RegisterAudioCapturerSourceCallback(
    std::unique_ptr<ICapturerStateCallback> callback)
{
    AUDIO_WARNING_LOG("RegisterAudioCapturerSourceCallback FAILED");
}

void RemoteFastAudioCapturerSourceInner::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_INFO_LOG("register params callback");
    paramCb_ = callback;

#ifdef FEATURE_DISTRIBUTE_AUDIO
    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "RegisterParameterCallback: Audio adapter is null.");
    int32_t ret = audioAdapter_->RegExtraParamObserver();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RegisterParameterCallback failed, ret %{public}d.", ret);
#endif
}

void RemoteFastAudioCapturerSourceInner::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("Audio param change event, key:%{public}d, condition:%{public}s, value:%{public}s",
        key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        ClearCapture();
    }

    CHECK_AND_RETURN_LOG(paramCb_ != nullptr, "Sink audio param callback is null.");
    paramCb_->OnAudioSourceParamChange(adapterName, key, condition, value);
}

float RemoteFastAudioCapturerSourceInner::GetMaxAmplitude()
{
    AUDIO_WARNING_LOG("getMaxAmplitude in remote fast audio cap not support");
    return 0;
}

int32_t RemoteFastAudioCapturerSourceInner::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioCapturerSourceInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namesapce OHOS
