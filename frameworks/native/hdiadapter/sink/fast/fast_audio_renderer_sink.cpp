/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "FastAudioRendererSinkInner"
#endif

#include "fast_audio_renderer_sink.h"

#include <cinttypes>
#include <climits>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <list>
#include <mutex>
#include <string>
#include <unistd.h>

#include <sys/mman.h>
#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#include "running_lock.h"
#include "audio_running_lock_manager.h"
#endif
#include "securec.h"
#include "v4_0/iaudio_manager.h"

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_performance_monitor.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t HALF_FACTOR = 2;
const uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
const float DEFAULT_VOLUME_LEVEL = 1.0f;
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 3840;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
const int64_t GENERAL_MAX_HANDLE_COST_IN_NANOSEC = 10000000; // 10ms = 10ns * 1000 * 1000
const int64_t VOIP_MAX_HANDLE_COST_IN_NANOSEC = 20000000; // 20ms = 20ns * 1000 * 1000
const int64_t SECOND_TO_NANOSECOND = 1000000000;
const int INVALID_FD = -1;
const unsigned int XCOLLIE_TIME_OUT_SECONDS = 10;
const std::string MMAP_PRIMARY_HAL_NAME = "mmap primary";
const std::string MMAP_VOIP_HAL_NAME = "mmap voip";
}

class FastAudioRendererSinkInner : public FastAudioRendererSink {
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
    int32_t SetVoiceVolume(float volume) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetTransactionId(uint64_t *transactionId) override;
    int32_t GetAudioScene() override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;
    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    void ResetOutputRouteForDisconnect(DeviceType device) override;

    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterAudioSinkCallback(IAudioSinkCallback* callback) override;

    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t SetSinkMuteForSwitchDevice(bool mute) final;

    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    float GetMaxAmplitude() override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;
    int32_t GetRenderId(uint32_t &renderId) const override;

    FastAudioRendererSinkInner();
    ~FastAudioRendererSinkInner();

private:
#ifdef FEATURE_POWER_MANAGER
    void KeepRunningLock();
    void KeepRunningUnlock();
#endif
    int32_t PrepareMmapBuffer();
    void ReleaseMmapBuffer();

    int32_t CheckPositionTime();
    void PreparePosition();

    void InitAttrs(struct AudioSampleAttributes &attrs);
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    int32_t CreateRender(const struct AudioPort &renderPort);
    int32_t InitAudioManager();
    void UpdateSinkState(bool started);

private:
    IAudioSinkAttr attr_ = {};
    bool rendererInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    float leftVolume_ = 0.0f;
    float rightVolume_ = 0.0f;
    int32_t routeHandle_ = -1;
    std::string adapterNameCase_ = "";
    struct IAudioManager *audioManager_ = nullptr;
    struct IAudioAdapter *audioAdapter_ = nullptr;
    struct IAudioRender *audioRender_ = nullptr;
    struct AudioAdapterDescriptor adapterDesc_ = {};
    struct AudioPort audioPort_ = {};
    uint32_t renderId_ = 0;
    uint32_t sinkId_ = 0;
    std::string halName_ = "";

    size_t bufferSize_ = 0;
    uint32_t bufferTotalFrameSize_ = 0;

    int bufferFd_ = INVALID_FD;
    uint32_t frameSizeInByte_ = 1;
    uint32_t eachReadFrameSize_ = 0;
    std::mutex mutex_;
    IAudioSinkCallback *callback_ = nullptr;
    // for device switch
    std::mutex switchDeviceMutex_;
    int32_t muteCount_ = 0;
    std::atomic<bool> switchDeviceMute_ = false;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
#endif

#ifdef DEBUG_DIRECT_USE_HDI
    char *bufferAddresss_ = nullptr;
    bool isFirstWrite_ = true;
    uint64_t alreadyReadFrames_ = 0;
    uint32_t curReadPos_ = 0;
    uint32_t curWritePos_ = 0;
    uint32_t writeAheadPeriod_ = 1;

    int privFd_ = INVALID_FD; // invalid fd
#endif
};  // FastAudioRendererSinkInner

FastAudioRendererSinkInner::FastAudioRendererSinkInner()
    : rendererInited_(false), started_(false), paused_(false), leftVolume_(DEFAULT_VOLUME_LEVEL),
      rightVolume_(DEFAULT_VOLUME_LEVEL), audioManager_(nullptr), audioAdapter_(nullptr),
      audioRender_(nullptr)
{
    attr_ = {};
}

FastAudioRendererSinkInner::~FastAudioRendererSinkInner()
{
    AUDIO_INFO_LOG("In");
    FastAudioRendererSinkInner::DeInit();
}

IMmapAudioRendererSink *FastAudioRendererSink::GetInstance()
{
    static FastAudioRendererSinkInner audioRenderer;

    return &audioRenderer;
}

IMmapAudioRendererSink *FastAudioRendererSink::GetVoipInstance()
{
    static FastAudioRendererSinkInner audioVoipRenderer;

    return &audioVoipRenderer;
}

std::shared_ptr<IMmapAudioRendererSink> FastAudioRendererSink::CreateFastRendererSink()
{
    std::shared_ptr<IMmapAudioRendererSink> audioRenderer = std::make_shared<FastAudioRendererSinkInner>();

    return audioRenderer;
}

bool FastAudioRendererSinkInner::IsInited()
{
    return rendererInited_;
}

void FastAudioRendererSinkInner::DeInit()
{
    AUDIO_INFO_LOG("In");
#ifdef FEATURE_POWER_MANAGER
    KeepRunningUnlock();

#endif

    started_ = false;
    rendererInited_ = false;
    if ((audioRender_ != nullptr) && (audioAdapter_ != nullptr)) {
        AUDIO_INFO_LOG("Destroy render");
        audioAdapter_->DestroyRender(audioAdapter_, renderId_);
    }
    audioRender_ = nullptr;

    if ((audioManager_ != nullptr) && (audioAdapter_ != nullptr)) {
        AUDIO_INFO_LOG("Unload adapter");
        if (routeHandle_ != -1) {
            audioAdapter_->ReleaseAudioRoute(audioAdapter_, routeHandle_);
        }
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;

    ReleaseMmapBuffer();
}

void FastAudioRendererSinkInner::InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.interleaved = true;
    attrs.streamId = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ?
        static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_VOIP_FAST)) :
        static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_FAST));
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

static int32_t SwitchAdapterRender(struct AudioAdapterDescriptor *descs, string adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &renderPort, int32_t size)
{
    if (descs == nullptr) {
        return ERROR;
    }
    for (int32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
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
    AUDIO_ERR_LOG("SwitchAdapterRender Fail");

    return ERR_INVALID_INDEX;
}

int32_t FastAudioRendererSinkInner::InitAudioManager()
{
    AUDIO_INFO_LOG("Initialize audio proxy manager");

    audioManager_ = IAudioManagerGet(false);
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    return 0;
}

// UpdateSinkState must be called with FastAudioRendererSinkInner::mutex_ held
void FastAudioRendererSinkInner::UpdateSinkState(bool started)
{
    if (callback_) {
        callback_->OnAudioSinkStateChange(sinkId_, started);
    } else {
        AUDIO_WARNING_LOG("AudioSinkCallback is nullptr");
    }
}

uint32_t PcmFormatToBits(HdiAdapterFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return PCM_8_BIT;
        case SAMPLE_S16LE:
            return PCM_16_BIT;
        case SAMPLE_S24LE:
            return PCM_24_BIT;
        case SAMPLE_S32LE:
            return PCM_32_BIT;
        case SAMPLE_F32LE:
            return PCM_32_BIT;
        default:
            return PCM_24_BIT;
    }
}

int32_t FastAudioRendererSinkInner::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
    uint32_t &byteSizePerFrame)
{
    CHECK_AND_RETURN_RET_LOG(bufferFd_ != INVALID_FD, ERR_INVALID_HANDLE, "buffer fd has been released!");
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::SetSinkMuteForSwitchDevice(bool mute)
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
        if (halName_ == MMAP_VOIP_HAL_NAME && audioRender_ != nullptr) {
            audioRender_->SetVolume(audioRender_, 0.0f);
        }
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("%{public}s not all unmuted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
        if (halName_ == MMAP_VOIP_HAL_NAME) {
            SetVolume(leftVolume_, rightVolume_);
        }
    }

    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Audio render is null!");

    struct AudioTimeStamp timestamp = {};
    int32_t ret = audioRender_->GetMmapPosition(audioRender_, &frames, &timestamp);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Hdi GetMmapPosition filed, ret:%{public}d!", ret);
#ifdef DEBUG_DIRECT_USE_HDI
    alreadyReadFrames_ = frames; // frames already read.
    curReadPos_ = frameSizeInByte_ * (frames - bufferTotalFrameSize_ * (frames / bufferTotalFrameSize_));
    CHECK_AND_RETURN_RET_LOG((curReadPos_ >= 0 && curReadPos_ < bufferSize_), ERR_INVALID_PARAM, "curReadPos invalid");
    AUDIO_DEBUG_LOG("GetMmapHandlePosition frames[:%{public}" PRIu64 "] tvsec:%{public}" PRId64 " tvNSec:"
        "%{public}" PRId64 " alreadyReadFrames:%{public}" PRId64 " curReadPos[%{public}d]",
        frames, timestamp.tvSec, timestamp.tvNSec, alreadyReadFrames_, curReadPos_);
#endif

    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it.
    CHECK_AND_RETURN_RET_LOG(timestamp.tvSec >= 0 && timestamp.tvSec <= maxSec && timestamp.tvNSec >= 0 &&
        timestamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "Hdi GetMmapPosition get invaild second:%{public}" PRId64 " or nanosecond:%{public}" PRId64 " !",
        timestamp.tvSec, timestamp.tvNSec);
    timeSec = timestamp.tvSec;
    timeNanoSec = timestamp.tvNSec;

    return SUCCESS;
}

void FastAudioRendererSinkInner::ReleaseMmapBuffer()
{
#ifdef DEBUG_DIRECT_USE_HDI
    if (bufferAddresss_ != nullptr) {
        munmap(bufferAddresss_, bufferSize_);
        bufferAddresss_ = nullptr;
        bufferSize_ = 0;
        AUDIO_INFO_LOG("ReleaseMmapBuffer end.");
    } else {
        AUDIO_WARNING_LOG("ReleaseMmapBuffer buffer already null.");
    }
    if (privFd_ != INVALID_FD) {
        CloseFd(privFd_);
        privFd_ = INVALID_FD;
    }
#endif
    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }
}

int32_t FastAudioRendererSinkInner::PrepareMmapBuffer()
{
    uint32_t totalBufferInMs = 40; // 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency.
    frameSizeInByte_ = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    uint32_t reqBufferFrameSize = totalBufferInMs * (attr_.sampleRate / 1000);

    struct AudioMmapBufferDescriptor desc = {0};
    int32_t ret = audioRender_->ReqMmapBuffer(audioRender_, reqBufferFrameSize, &desc);
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
    eachReadFrameSize_ = desc.transferFrameSize; // 240

    CHECK_AND_RETURN_RET_LOG(frameSizeInByte_ <= ULLONG_MAX / bufferTotalFrameSize_, ERR_OPERATION_FAILED,
        "BufferSize will overflow!");
    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte_;
#ifdef DEBUG_DIRECT_USE_HDI
    privFd_ = dup(bufferFd_);
    bufferAddresss_ = (char *)mmap(nullptr, bufferSize_, PROT_READ | PROT_WRITE, MAP_SHARED, privFd_, 0);
    CHECK_AND_RETURN_RET_LOG(bufferAddresss_ != nullptr && bufferAddresss_ != MAP_FAILED, ERR_OPERATION_FAILED,
        "mmap buffer failed!");
#endif
    return SUCCESS;
}

AudioFormat FastAudioRendererSinkInner::ConvertToHdiFormat(HdiAdapterFormat format)
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
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }

    return hdiFormat;
}

int32_t FastAudioRendererSinkInner::CreateRender(const struct AudioPort &renderPort)
{
    int32_t ret;
    struct AudioSampleAttributes param;
    InitAttrs(param);
    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AUDIO_MMAP_VOIP : AUDIO_MMAP_NOIRQ;
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    if (param.channelCount == MONO) {
        param.channelLayout = CH_LAYOUT_MONO;
    } else if (param.channelCount == STEREO) {
        param.channelLayout = CH_LAYOUT_STEREO;
    }
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBits(attr_.format) * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize); // not passed in hdi
    AUDIO_INFO_LOG("Type: %{public}d, sampleRate: %{public}u, channel: %{public}d, format: %{public}d, "
        "device:%{public}d", param.type, param.sampleRate, param.channelCount, param.format, attr_.deviceType);
    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = renderPort.portId;
    switch (static_cast<DeviceType>(attr_.deviceType)) {
        case DEVICE_TYPE_EARPIECE:
            deviceDesc.pins = PIN_OUT_EARPIECE;
            break;
        case DEVICE_TYPE_SPEAKER:
            deviceDesc.pins = PIN_OUT_SPEAKER;
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            deviceDesc.pins = PIN_OUT_HEADSET;
            break;
        case DEVICE_TYPE_USB_HEADSET:
            deviceDesc.pins = PIN_OUT_USB_EXT;
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            deviceDesc.pins = PIN_OUT_BLUETOOTH_SCO;
            break;
        default:
            deviceDesc.pins = PIN_OUT_SPEAKER;
            break;
    }
    char desc[] = "";
    deviceDesc.desc = desc;
    ret = audioAdapter_->CreateRender(audioAdapter_, &deviceDesc, &param, &audioRender_, &renderId_);
    if (ret != 0 || audioRender_ == nullptr) {
        AUDIO_ERR_LOG("AudioDeviceCreateRender failed, ret is :%{public}d", ret);
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
        return ERR_NOT_STARTED;
    }

    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("FastAudioRendererSinkInner::Init");
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;  // Set sound card information
    halName_ = attr_.audioStreamFlag == AUDIO_FLAG_MMAP ? MMAP_PRIMARY_HAL_NAME : MMAP_VOIP_HAL_NAME;
    enum AudioPortDirection port = PORT_OUT; // Set port information

    CHECK_AND_RETURN_RET_LOG(InitAudioManager() == 0, ERR_NOT_STARTED, "Init audio manager Fail");

    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    if (audioManager_ == nullptr) {
        AUDIO_ERR_LOG("The audioManager is nullptr!");
        return ERROR;
    }
    int32_t ret = audioManager_->GetAllAdapters(audioManager_,
        (struct AudioAdapterDescriptor *)&descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && ret == 0, ERR_NOT_STARTED,
        "Get adapters Fail");

    int32_t index = SwitchAdapterRender((struct AudioAdapterDescriptor *)&descs, adapterNameCase_, port, audioPort_,
        size);
    CHECK_AND_RETURN_RET_LOG(index >= 0, ERR_NOT_STARTED, "Switch Adapter Fail");

    adapterDesc_ = descs[index];
    int32_t result = audioManager_->LoadAdapter(audioManager_, &adapterDesc_, &audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(result == 0, ERR_NOT_STARTED, "Load Adapter Fail");
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device failed");

    // Initialization port information, can fill through mode and other parameters
    ret = audioAdapter_->InitAllPorts(audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "InitAllPorts failed");

    CHECK_AND_RETURN_RET_LOG(CreateRender(audioPort_) == SUCCESS && PrepareMmapBuffer() == SUCCESS,
        ERR_NOT_STARTED, "Create render failed, Audio Port: %{public}d", audioPort_.portId);

    rendererInited_ = true;
    GetRenderId(sinkId_);

    return SUCCESS;
}

void FastAudioRendererSinkInner::PreparePosition()
{
#ifdef DEBUG_DIRECT_USE_HDI
    isFirstWrite_ = false;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    GetMmapHandlePosition(frames, timeSec, timeNanoSec); // get first start position
    int32_t periodByteSize = eachReadFrameSize_ * frameSizeInByte_;
    CHECK_AND_RETURN_LOG(periodByteSize * writeAheadPeriod_ <= ULLONG_MAX - curReadPos_, "TempPos will overflow!");
    size_t tempPos = curReadPos_ + periodByteSize * writeAheadPeriod_; // 1 period ahead
    curWritePos_ = (tempPos < bufferSize_ ? tempPos : tempPos - bufferSize_);
    AUDIO_INFO_LOG("First render frame start with curReadPos_[%{public}d] curWritePos_[%{public}d]", curReadPos_,
        curWritePos_);
#endif
}

int32_t FastAudioRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
#ifdef DEBUG_DIRECT_USE_HDI
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Audio Render Handle is nullptr!");

    if (len > (bufferSize_ - eachReadFrameSize_ * frameSizeInByte_ * writeAheadPeriod_)) {
        writeLen = 0;
        AUDIO_ERR_LOG("RenderFrame failed,too large len[%{public}" PRIu64 "]!", len);
        return ERR_WRITE_FAILED;
    }

    if (isFirstWrite_) {
        PreparePosition();
    }

    CHECK_AND_RETURN_RET_LOG((curWritePos_ >= 0 && curWritePos_ < bufferSize_), ERR_INVALID_PARAM,
        "curWritePos_ invalid");
    char *writePtr = bufferAddresss_ + curWritePos_;
    uint64_t dataBefore = *(uint64_t *)writePtr;
    uint64_t dataAfter = 0;
    uint64_t tempPos = curWritePos_ + len;
    if (tempPos <= bufferSize_) {
        if (memcpy_s(writePtr, (bufferSize_ - curWritePos_), static_cast<void *>(&data), len)) {
            AUDIO_ERR_LOG("copy failed");
            return ERR_WRITE_FAILED;
        }
        dataAfter = *(uint64_t *)writePtr;
        curWritePos_ = (tempPos == bufferSize_ ? 0 : tempPos);
    } else {
        AUDIO_DEBUG_LOG("(tempPos%{public}" PRIu64 ")curWritePos_ + len > bufferSize_", tempPos);
        size_t writeableSize = bufferSize_ - curWritePos_;
        if (memcpy_s(writePtr, writeableSize, static_cast<void *>(&data), writeableSize) ||
            memcpy_s(bufferAddresss_, bufferSize_, static_cast<void *>((char *)&data + writeableSize),
            (len - writeableSize))) {
            AUDIO_ERR_LOG("copy failed");
            return ERR_WRITE_FAILED;
        }
        curWritePos_ = len - writeableSize;
    }
    writeLen = len;

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("Render len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms curWritePos[%{public}d] dataBefore"
        "<%{public}" PRIu64 "> dataAfter<%{public}" PRIu64 ">", len, stamp, curWritePos_, dataBefore, dataAfter);
    return SUCCESS;
#else
    AUDIO_WARNING_LOG("RenderFrame is not supported.");
    return ERR_NOT_SUPPORTED;
#endif
}

float FastAudioRendererSinkInner::GetMaxAmplitude()
{
    AUDIO_WARNING_LOG("getMaxAmplitude in fast_audio_renderder_sink not support");
    return 0;
}

int32_t FastAudioRendererSinkInner::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::CheckPositionTime()
{
    int32_t tryCount = 50;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int64_t maxHandleCost = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? VOIP_MAX_HANDLE_COST_IN_NANOSEC :
        GENERAL_MAX_HANDLE_COST_IN_NANOSEC;
    int64_t waitTime = 2000000; // 2ms
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(waitTime); // us
        int64_t timeBeforeGetPos = ClockTime::GetCurNano();
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curSec = timeBeforeGetPos / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = timeBeforeGetPos - curSec * AUDIO_NS_PER_SECOND;
        AUDIO_WARNING_LOG("DspSec: %{public}" PRId64 ", dspNanoSec: %{public}" PRId64 ", Time before get pos: "
            "%{public}" PRId64 ", time cost: %{public}" PRId64 "", timeSec, timeNanoSec, timeBeforeGetPos,
            ClockTime::GetCurNano() - timeBeforeGetPos);
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > maxHandleCost) {
            continue;
        } else {
            AUDIO_INFO_LOG("CheckPositionTime end, position and time is ok.");
            return SUCCESS;
        }
    }
#ifdef FEATURE_POWER_MANAGER
    KeepRunningUnlock();
#endif
    AUDIO_ERR_LOG("Stop hdi fast renderer when GetMmapPosition failed");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "audioRenderer_ is nullptr when trying to stop");
    int32_t ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Stop failed! ret: %{public}d.", ret);
    return ERROR;
}

int32_t FastAudioRendererSinkInner::Start(void)
{
    std::lock_guard<std::mutex> lock(mutex_);
    Trace trace("FastAudioRendererSinkInner::Start");
    AudioXCollie sourceXCollie("FastAudioRendererSinkInner::Start", XCOLLIE_TIME_OUT_SECONDS);
    AUDIO_INFO_LOG("FastAudioRendererSinkInner::Start, sinkId %{public}u", sinkId_);
    int64_t stamp = ClockTime::GetCurNano();
    int32_t ret;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "FastAudioRendererSink::Start audioRender_ null!");

    if (!started_) {
        ret = audioRender_->Start(audioRender_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED,
            "FastAudioRendererSink::Start failed!");
        UpdateSinkState(true);
        int32_t err = CheckPositionTime();
        CHECK_AND_RETURN_RET_LOG(err == SUCCESS, ERR_NOT_STARTED,
            "FastAudioRendererSink::CheckPositionTime failed!");
    }
#ifdef FEATURE_POWER_MANAGER
    KeepRunningLock();
#endif
    started_ = true;
    AUDIO_DEBUG_LOG("Start cost[%{public}" PRId64 "]ms", (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_FAST, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}
#ifdef FEATURE_POWER_MANAGER
void FastAudioRendererSinkInner::KeepRunningLock()
{
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (runningLockManager_ == nullptr) {
        WatchTimeout guard("PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock:KeepRunningLock");
        keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioFastBackgroundPlay",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        guard.CheckCurrTimeout();
        if (keepRunningLock) {
            runningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }

    if (runningLockManager_ != nullptr) {
        int32_t timeOut = -1; // -1 for lasting.
        AUDIO_INFO_LOG("keepRunningLock lock result: %{public}d",
            runningLockManager_->Lock(timeOut)); // -1 for lasting.
    } else {
        AUDIO_ERR_LOG("keepRunningLock is null, playback can not work well!");
    }
}
#endif

#ifdef FEATURE_POWER_MANAGER
void FastAudioRendererSinkInner::KeepRunningUnlock()
{
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unLock");
        runningLockManager_->UnLock();
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, playback can not work well!");
    }
}
#endif


int32_t FastAudioRendererSinkInner::SetVolume(float left, float right)
{
    int32_t ret;
    float volume;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "FastAudioRendererSink::SetVolume failed audioRender_ null");
    if (halName_ == MMAP_VOIP_HAL_NAME && switchDeviceMute_ && (abs(left) > FLOAT_EPS || abs(right) > FLOAT_EPS)) {
        AUDIO_ERR_LOG("Mmap voip scene. No need set to volume when switch device and volume is 0");
        leftVolume_ = left;
        rightVolume_ = right;
        return ERR_INVALID_HANDLE;
    }

    leftVolume_ = left;
    rightVolume_ = right;
    if ((abs(leftVolume_) < FLOAT_EPS) && (abs(rightVolume_) > FLOAT_EPS)) {
        volume = rightVolume_;
    } else if ((abs(leftVolume_) > FLOAT_EPS) && (abs(rightVolume_) < FLOAT_EPS)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    AUDIO_INFO_LOG("Set hdi volume to %{public}f", volume);
    ret = audioRender_->SetVolume(audioRender_, volume);
    if (ret) {
        AUDIO_ERR_LOG("FastAudioRendererSink::Set volume failed!");
    }

    return ret;
}

int32_t FastAudioRendererSinkInner::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::SetVoiceVolume(float volume)
{
    AUDIO_ERR_LOG("FastAudioRendererSink SetVoiceVolume not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::GetAudioScene()
{
    AUDIO_ERR_LOG("FastAudioRendererSink GetAudioScene not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    AUDIO_ERR_LOG("FastAudioRendererSink SetAudioScene not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    AUDIO_ERR_LOG("SetOutputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

void FastAudioRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_ERR_LOG("FastAudioRendererSink SetAudioParameter not supported.");
    return;
}

std::string FastAudioRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_INFO_LOG("GetAudioParameter, key: %{public}d, condition: %{public}s",
        key, condition.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    char value[DumpFileUtil::PARAM_VALUE_LENTH];
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, "",
        "GetAudioParameter failed, audioAdapter_ is null");
    int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value,
        DumpFileUtil::PARAM_VALUE_LENTH);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "",
        "FRSink GetAudioParameter failed, error code:%{public}d", ret);
    return value;
}

void FastAudioRendererSinkInner::RegisterAudioSinkCallback(IAudioSinkCallback* callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback_) {
        AUDIO_INFO_LOG("AudioSinkCallback registered");
    } else {
        callback_ = callback;
        AUDIO_INFO_LOG("Register AudioSinkCallback");
    }
}

void FastAudioRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    AUDIO_ERR_LOG("FastAudioRendererSink SetAudioMonoState not supported.");
    return;
}

void FastAudioRendererSinkInner::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_ERR_LOG("FastAudioRendererSink SetAudioBalanceValue not supported.");
    return;
}

int32_t FastAudioRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_ERR_LOG("FastAudioRendererSink GetPresentationPosition not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    AUDIO_ERR_LOG("FastAudioRendererSink %{public}s", __func__);
    *transactionId = 6; // 6 is the mmap device.
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRendererSinkInner::GetLatency(uint32_t *latency)
{
    Trace trace("FastAudioRendererSinkInner::GetLatency");
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

int32_t FastAudioRendererSinkInner::Stop(void)
{
    std::lock_guard<std::mutex> lock(mutex_);
    Trace trace("FastAudioRendererSinkInner::Stop");
    AudioXCollie sourceXCollie("FastAudioRendererSinkInner::Stop", XCOLLIE_TIME_OUT_SECONDS);
    AUDIO_INFO_LOG("Stop, sinkId %{public}u", sinkId_);

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Stop failed audioRender_ null");
#ifdef FEATURE_POWER_MANAGER
    KeepRunningUnlock();
#endif

    if (started_) {
        int32_t ret = audioRender_->Stop(audioRender_);
        UpdateSinkState(false);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
            "Stop failed! ret: %{public}d.", ret);
    }
    started_ = false;

    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::Pause(void)
{
    int32_t ret;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Pause failed audioRender_ null");

    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Pause invalid state!");

    if (!paused_) {
        ret = audioRender_->Pause(audioRender_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
            "Pause failed!");
    }
    paused_ = true;

    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::Resume(void)
{
    Trace trace("FastAudioRendererSinkInner::Resume");
    int32_t ret;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "Resume failed audioRender_ null");

    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED,
        "Resume invalid state!");

    if (paused_) {
        ret = audioRender_->Resume(audioRender_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
            "Resume failed!");
    }
    paused_ = false;
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_FAST, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::RestoreRenderSink(void)
{
    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::Reset(void)
{
    Trace trace("FastAudioRendererSinkInner::Reset");
    int32_t ret;

    if (started_ && audioRender_ != nullptr) {
        ret = audioRender_->Flush(audioRender_);

        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
            "Reset failed!");
    }

    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::Flush(void)
{
    Trace trace("FastAudioRendererSinkInner::Flush");
    int32_t ret;

    if (started_ && audioRender_ != nullptr) {
        ret = audioRender_->Flush(audioRender_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
            "Flush failed!");
    }

    return SUCCESS;
}

void FastAudioRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
}

int32_t FastAudioRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size)
{
    return SUCCESS;
}

int32_t FastAudioRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
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

int32_t FastAudioRendererSinkInner::GetRenderId(uint32_t &renderId) const
{
    renderId = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_FAST);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
