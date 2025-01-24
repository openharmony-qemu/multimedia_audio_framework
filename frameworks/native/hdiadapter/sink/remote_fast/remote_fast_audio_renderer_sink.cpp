/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "RemoteFastAudioRendererSinkInner"
#endif

#include "remote_fast_audio_renderer_sink.h"

#include <cinttypes>
#include <dlfcn.h>
#include <map>
#include <sstream>
#include "securec.h"

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include <v1_0/iaudio_manager.h>
#include <v1_0/iaudio_callback.h>
#include <v1_0/audio_types.h>
#include "ashmem.h"
#include "i_audio_renderer_sink.h"
#include "i_audio_device_adapter.h"
#include "i_audio_device_manager.h"

using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortDirection;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioManager;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender;
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
namespace {
const int32_t HALF_FACTOR = 2;
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 3840;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t PCM_24_BIT = 24;
const uint32_t PCM_32_BIT = 32;
const int64_t SECOND_TO_NANOSECOND = 1000000000;
const int32_t INVALID_FD = -1;
}
class RemoteFastAudioRendererSinkInner : public RemoteFastAudioRendererSink, public IAudioDeviceAdapterCallback {
public:
    explicit RemoteFastAudioRendererSinkInner(const std::string &deviceNetworkId);
    ~RemoteFastAudioRendererSinkInner();

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
    int32_t GetTransactionId(uint64_t *transactionId) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetAudioScene() override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;
    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    void RegisterAudioSinkCallback(IAudioSinkCallback* callback) override;

    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;
    int32_t CheckPositionTime();

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;
    float GetMaxAmplitude() override;

    void ResetOutputRouteForDisconnect(DeviceType device) override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;
    int32_t GetRenderId(uint32_t &renderId) const override;

    std::string GetNetworkId();
    IAudioSinkCallback* GetParamCallback();

private:
    int32_t CreateRender(const struct AudioPort &renderPort);
    void InitAttrs(struct AudioSampleAttributes &attrs);
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    int32_t PrepareMmapBuffer();
    uint32_t PcmFormatToBits(HdiAdapterFormat format);
    void ClearRender();

private:
    std::atomic<bool> rendererInited_ = false;
    std::atomic<bool> isRenderCreated_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    float leftVolume_ = 0;
    float rightVolume_ = 0;
    std::shared_ptr<IAudioDeviceManager> audioManager_ = nullptr;
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter_ = nullptr;
    IAudioSinkCallback *callback_ = nullptr;
    sptr<IAudioRender> audioRender_ = nullptr;
    struct AudioPort audioPort_ = {};
    IAudioSinkAttr attr_ = {};
    std::string deviceNetworkId_ = "";

    uint32_t bufferTotalFrameSize_ = 0;
    int32_t bufferFd_ = INVALID_FD;
    uint32_t frameSizeInByte_ = 1;
    uint32_t eachReadFrameSize_ = 0;
    uint32_t renderId_ = 0;

#ifdef DEBUG_DIRECT_USE_HDI
    sptr<Ashmem> ashmemSink_ = nullptr;
    size_t bufferSize_ = 0;
#endif
};

std::mutex g_remoteFRSinksMutex;
std::map<std::string, RemoteFastAudioRendererSinkInner *> allRFSinks;
IMmapAudioRendererSink *RemoteFastAudioRendererSink::GetInstance(const std::string &deviceNetworkId)
{
    std::lock_guard<std::mutex> lock(g_remoteFRSinksMutex);
    AUDIO_INFO_LOG("GetInstance.");
    CHECK_AND_RETURN_RET_LOG(!deviceNetworkId.empty(), nullptr, "Remote fast render device networkId is null.");

    if (allRFSinks.count(deviceNetworkId)) {
        return allRFSinks[deviceNetworkId];
    }
    RemoteFastAudioRendererSinkInner *audioRenderer =
        new(std::nothrow) RemoteFastAudioRendererSinkInner(deviceNetworkId);
    AUDIO_DEBUG_LOG("New daudio remote fast render device networkId: [%{public}s].", deviceNetworkId.c_str());
    allRFSinks[deviceNetworkId] = audioRenderer;
    return audioRenderer;
}

RemoteFastAudioRendererSinkInner::RemoteFastAudioRendererSinkInner(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_DEBUG_LOG("RemoteFastAudioRendererSinkInner Constract.");
}

RemoteFastAudioRendererSinkInner::~RemoteFastAudioRendererSinkInner()
{
    if (rendererInited_.load()) {
        RemoteFastAudioRendererSinkInner::DeInit();
    }
    AUDIO_DEBUG_LOG("RemoteFastAudioRendererSink end.");
}

bool RemoteFastAudioRendererSinkInner::IsInited()
{
    return rendererInited_.load();
}

void RemoteFastAudioRendererSinkInner::ClearRender()
{
    AUDIO_INFO_LOG("Clear remote fast audio render enter.");
    rendererInited_.store(false);
    isRenderCreated_.store(false);
    started_.store(false);
    paused_.store(false);

#ifdef DEBUG_DIRECT_USE_HDI
    if (ashmemSink_ != nullptr) {
        ashmemSink_->UnmapAshmem();
        ashmemSink_->CloseAshmem();
        ashmemSink_ = nullptr;
        AUDIO_INFO_LOG("ClearRender: UnInit sink ashmem OK.");
    }
#endif // DEBUG_DIRECT_USE_HDI
    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyRender(audioRender_, renderId_);
        audioAdapter_->Release();
    }
    audioRender_ = nullptr;
    audioAdapter_ = nullptr;

    if (audioManager_ != nullptr) {
        audioManager_->UnloadAdapter(deviceNetworkId_);
    }
    audioManager_ = nullptr;

    AudioDeviceManagerFactory::GetInstance().DestoryDeviceManager(REMOTE_DEV_MGR);
    AUDIO_INFO_LOG("Clear remote audio render end.");
}

void RemoteFastAudioRendererSinkInner::DeInit()
{
    std::lock_guard<std::mutex> lock(g_remoteFRSinksMutex);
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::DeInit");
    ClearRender();

    CHECK_AND_RETURN_LOG(allRFSinks.count(this->deviceNetworkId_) > 0,
        "not find %{public}s", this->deviceNetworkId_.c_str());
    RemoteFastAudioRendererSink *temp = allRFSinks[this->deviceNetworkId_];
    allRFSinks.erase(this->deviceNetworkId_);
    if (temp == nullptr) {
        AUDIO_ERR_LOG("temp is nullptr");
    } else {
        delete temp;
    }
}

int32_t RemoteFastAudioRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::Init");
    attr_ = attr;
    audioManager_ = AudioDeviceManagerFactory::GetInstance().CreatDeviceManager(REMOTE_DEV_MGR);
    CHECK_AND_RETURN_RET_LOG(audioManager_ != nullptr, ERR_NOT_STARTED, "Init audio manager fail.");

    struct AudioAdapterDescriptor *desc = audioManager_->GetTargetAdapterDesc(deviceNetworkId_, true);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NOT_STARTED, "Get target adapters descriptor fail.");
    for (uint32_t port = 0; port < desc->ports.size(); port++) {
        if (desc->ports[port].portId == AudioPortPin::PIN_OUT_SPEAKER) {
            audioPort_ = desc->ports[port];
            break;
        }
        CHECK_AND_RETURN_RET_LOG(port != (desc->ports.size() - 1), ERR_INVALID_INDEX,
            "Not found the audio spk port.");
    }

    audioAdapter_ = audioManager_->LoadAdapters(deviceNetworkId_, true);
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device adapter failed.");

    int32_t ret = audioAdapter_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Audio adapter init fail, ret %{public}d.", ret);

    ret = CreateRender(audioPort_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Create render fail, audio port %{public}d, ret %{public}d.",
        audioPort_.portId, ret);

    rendererInited_.store(true);
    AUDIO_DEBUG_LOG("RemoteFastAudioRendererSink: Init end.");
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::CreateRender(const struct AudioPort &renderPort)
{
    int64_t start = ClockTime::GetCurNano();

    struct AudioSampleAttributes param;
    InitAttrs(param);
    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AudioCategory::AUDIO_MMAP_VOIP :
        AudioCategory::AUDIO_MMAP_NOIRQ;
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    AUDIO_INFO_LOG("Create render format: %{public}d", param.format);

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = renderPort.portId;
    deviceDesc.pins = AudioPortPin::PIN_OUT_SPEAKER;
    deviceDesc.desc = "";

    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE, "CreateRender: Audio adapter is null.");
    int32_t ret = audioAdapter_->CreateRender(deviceDesc, param, audioRender_, this, renderId_);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && audioRender_ != nullptr, ret,
        "AudioDeviceCreateRender failed");
    if (param.type == AudioCategory::AUDIO_MMAP_NOIRQ || param.type == AudioCategory::AUDIO_MMAP_VOIP) {
        PrepareMmapBuffer();
    }
    isRenderCreated_.store(true);
    int64_t cost = (ClockTime::GetCurNano() - start) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("CreateRender cost[%{public}" PRId64 "]ms", cost);
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::PrepareMmapBuffer()
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "PrepareMmapBuffer: Audio render is null.");

    int32_t totalBufferInMs = 40; // 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency.
    frameSizeInByte_ = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    int32_t reqBufferFrameSize = totalBufferInMs * (static_cast<int32_t>(attr_.sampleRate) / 1000);

    struct AudioMmapBufferDescriptor desc;
    int32_t ret = audioRender_->ReqMmapBuffer(reqBufferFrameSize, desc);
    CHECK_AND_RETURN_RET_LOG((ret == SUCCESS), ERR_OPERATION_FAILED,
        "PrepareMmapBuffer require mmap buffer failed, ret:%{public}d.", ret);

    AUDIO_INFO_LOG("AudioMmapBufferDescriptor  memoryFd[%{public}d] totalBufferFrames"
        "[%{public}d] transferFrameSize[%{public}d] isShareable[%{public}d] offset[%{public}d]",
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

#ifdef DEBUG_DIRECT_USE_HDI
    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte_;
    ashmemSink_ = new Ashmem(bufferFd_, bufferSize_);
    AUDIO_INFO_LOG("PrepareMmapBuffer create ashmem sink OK, ashmemLen %{public}zu.", bufferSize_);
    bool tmp = ashmemSink_->MapReadAndWriteAshmem();
    CHECK_AND_RETURN_RET_LOG(tmp, ERR_OPERATION_FAILED, "PrepareMmapBuffer map ashmem sink failed.");
#endif // DEBUG_DIRECT_USE_HDI
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe,
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

int32_t RemoteFastAudioRendererSinkInner::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec,
    int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetMmapHandlePosition: Audio render is null.");

    struct AudioTimeStamp timestamp;
    int32_t ret = audioRender_->GetMmapPosition(frames, timestamp);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
        "Hdi GetMmapPosition filed, ret:%{public}d!", ret);

    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it.
    CHECK_AND_RETURN_RET_LOG(timestamp.tvSec >= 0 && timestamp.tvSec <= maxSec && timestamp.tvNSec >= 0 &&
        timestamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "Hdi GetMmapPosition get invaild second:%{public}" PRId64 " or nanosecond:%{public}" PRId64 " !",
        timestamp.tvSec, timestamp.tvNSec);
    timeSec = timestamp.tvSec;
    timeNanoSec = timestamp.tvNSec;

    return SUCCESS;
}

uint32_t RemoteFastAudioRendererSinkInner::PcmFormatToBits(HdiAdapterFormat format)
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

AudioFormat RemoteFastAudioRendererSinkInner::ConvertToHdiFormat(HdiAdapterFormat format)
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

void RemoteFastAudioRendererSinkInner::InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = 0;
    attrs.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_FAST));
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

inline std::string PrintRemoteAttr(const IAudioSinkAttr &attr)
{
    std::stringstream value;
    value << "adapterName[" << attr.adapterName << "] openMicSpeaker[" << attr.openMicSpeaker << "] ";
    value << "format[" << static_cast<int32_t>(attr.format) << "] ";
    value << "sampleRate[" << attr.sampleRate << "] channel[" << attr.channel << "] ";
    value << "volume[" << attr.volume << "] filePath[" << attr.filePath << "] ";
    value << "deviceNetworkId[" << attr.deviceNetworkId << "] device_type[" << attr.deviceType << "]";
    return value.str();
}

int32_t RemoteFastAudioRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    AUDIO_DEBUG_LOG("RenderFrame is not supported.");
    return SUCCESS;
}

float RemoteFastAudioRendererSinkInner::GetMaxAmplitude()
{
    AUDIO_WARNING_LOG("getMaxAmplitude in remote fast audio not support");
    return 0;
}

int32_t RemoteFastAudioRendererSinkInner::CheckPositionTime()
{
    int32_t tryCount = 10;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int64_t maxHandleCost = 10000000; // ns
    int64_t waitTime = 2000000; // 2ms
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

int32_t RemoteFastAudioRendererSinkInner::Start(void)
{
    AUDIO_INFO_LOG("Start.");
    if (!isRenderCreated_.load()) {
        CHECK_AND_RETURN_RET_LOG(CreateRender(audioPort_) == SUCCESS, ERR_NOT_STARTED,
            "Create render fail, audio port %{public}d", audioPort_.portId);
    }

    if (started_.load()) {
        AUDIO_INFO_LOG("Remote fast render is already started.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Start: Audio render is null.");
    int32_t ret = audioRender_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "Start fail, ret %{public}d.", ret);
    ret = CheckPositionTime();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "CheckPositionTime failed, ret %{public}d.", ret);
    started_.store(true);
    AUDIO_INFO_LOG("Start Ok.");
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::Stop(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::Stop");
    if (!started_.load()) {
        AUDIO_INFO_LOG("Remote fast render is already stopped.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Stop: Audio render is null.");
    int32_t ret = audioRender_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Stop fail, ret %{public}d.", ret);
    started_.store(false);
    AUDIO_DEBUG_LOG("Stop ok.");
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::Pause(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::Pause");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Pause invalid state!");

    if (paused_.load()) {
        AUDIO_INFO_LOG("Remote fast render is already paused.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Pause: Audio render is null.");
    int32_t ret = audioRender_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Pause fail, ret %{public}d.", ret);
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::Resume(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::Resume");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Resume invalid state!");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("Remote fast render is already resumed.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Resume: Audio render is null.");
    int32_t ret = audioRender_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Resume fail, ret %{public}d.", ret);
    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::Reset(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::Reset");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Reset invalid state!");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Reset: Audio render is null.");
    int32_t ret = audioRender_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Reset fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::Flush(void)
{
    AUDIO_INFO_LOG("RemoteFastAudioRendererSinkInner::Flush");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Flush invalid state!");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "Flush: Audio render is null.");
    int32_t ret = audioRender_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Flush fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::RestoreRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "SetVolume: Audio render is null.");

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

    int32_t ret = audioRender_->SetVolume(volume);
    if (ret) {
        AUDIO_ERR_LOG("Set volume failed!");
    }
    return ret;
}

int32_t RemoteFastAudioRendererSinkInner::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::GetLatency(uint32_t *latency)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "GetLatency failed audio render null");

    CHECK_AND_RETURN_RET_LOG(latency, ERR_INVALID_PARAM,
        "GetLatency failed latency null");

    uint32_t hdiLatency = 0;
    int32_t ret = audioRender_->GetLatency(hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
        "GetLatency failed.");

    *latency = hdiLatency;
    return SUCCESS;
}

void RemoteFastAudioRendererSinkInner::RegisterAudioSinkCallback(IAudioSinkCallback* callback)
{
    AUDIO_INFO_LOG("register params callback");
    callback_ = callback;

#ifdef FEATURE_DISTRIBUTE_AUDIO
    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "RegisterAudioSinkCallback: Audio adapter is null.");
    int32_t ret = audioAdapter_->RegExtraParamObserver();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RegisterAudioSinkCallback failed, ret %{public}d.", ret);
#endif
}

void RemoteFastAudioRendererSinkInner::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("Audio param change event, key:%{public}d, condition:%{public}s, value:%{public}s",
        key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        ClearRender();
    }

    CHECK_AND_RETURN_LOG(callback_ != nullptr, "Sink audio param callback is null.");
    callback_->OnAudioSinkParamChange(adapterName, key, condition, value);
}

int32_t RemoteFastAudioRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    (void)transactionId;
    AUDIO_ERR_LOG("GetTransactionId not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::SetVoiceVolume(float volume)
{
    (void)volume;
    AUDIO_ERR_LOG("SetVoiceVolume not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    (void)outputDevices;
    AUDIO_DEBUG_LOG("SetOutputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

void RemoteFastAudioRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    (void)audioMono;
    AUDIO_ERR_LOG("SetAudioMonoState not supported");
    return;
}

void RemoteFastAudioRendererSinkInner::SetAudioBalanceValue(float audioBalance)
{
    (void)audioBalance;
    AUDIO_ERR_LOG("SetAudioBalanceValue not supported");
    return;
}

int32_t RemoteFastAudioRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec,
    int64_t& timeNanoSec)
{
    AUDIO_ERR_LOG("GetPresentationPosition not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::GetAudioScene()
{
    AUDIO_INFO_LOG("SetAudioScene not supported");
    return SUCCESS;
}

int32_t RemoteFastAudioRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    AUDIO_INFO_LOG("SetAudioScene not supported");
    return SUCCESS;
}

void RemoteFastAudioRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("SetAudioParameter not support.");
}

std::string RemoteFastAudioRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_INFO_LOG("GetAudioParameter not support.");
    return "";
}

std::string RemoteFastAudioRendererSinkInner::GetNetworkId()
{
    return deviceNetworkId_;
}

void RemoteFastAudioRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
    AUDIO_WARNING_LOG("not supported.");
}

OHOS::AudioStandard::IAudioSinkCallback* RemoteFastAudioRendererSinkInner::GetParamCallback()
{
    return callback_;
}

int32_t RemoteFastAudioRendererSinkInner::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRendererSinkInner::GetRenderId(uint32_t &renderId) const
{
    renderId = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_FAST);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
