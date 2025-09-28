/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "MultichannelAudioRenderSink"
#endif

#include "sink/multichannel_audio_render_sink.h"
#include <climits>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

namespace {
const std::string DEFAULT_NAME = "multichannel";
}
namespace OHOS {
namespace AudioStandard {
MultichannelAudioRenderSink::MultichannelAudioRenderSink(const std::string &halName)
    : halName_(halName == HDI_ID_INFO_DEFAULT ? DEFAULT_NAME : halName)
{
    AUDIO_INFO_LOG("construction");
}

MultichannelAudioRenderSink::~MultichannelAudioRenderSink()
{
    AUDIO_INFO_LOG("destruction");
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(ADAPTER_TYPE_MULTICHANNEL);
}

int32_t MultichannelAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;
    openSpeaker_ = attr_.openMicSpeaker;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);

    int32_t ret = InitRender();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);

    sinkInited_ = true;
    return SUCCESS;
}

void MultichannelAudioRenderSink::DeInit(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    sinkInited_ = false;
    started_ = false;

    AUDIO_INFO_LOG("destroy render, hdiRenderId: %{public}u", hdiRenderId_);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    renderInited_ = false;
    deviceManager->DestroyRender(adapterNameCase_, hdiRenderId_);
    audioRender_ = nullptr;

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool MultichannelAudioRenderSink::IsInited(void)
{
    return sinkInited_;
}

int32_t MultichannelAudioRenderSink::Start(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("MultichannelAudioRenderSink::Start");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME_BASE) + halName_);
        guard.CheckCurrTimeout();
    }
    if (runningLock_ != nullptr) {
        runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    } else {
        AUDIO_ERR_LOG("running lock is null, playback can not work well");
    }
#endif
    dumpFileName_ = "multichannel_sink_" + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    if (started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail");
    UpdateSinkState(true);
    started_ = true;

    uint64_t frameSize = 0;
    uint64_t frameCount = 0;
    ret = audioRender_->GetFrameSize(audioRender_, &frameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "get frame size fail");
    ret = audioRender_->GetFrameCount(audioRender_, &frameCount);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "get frame count fail");
    ret = audioRender_->SetVolume(audioRender_, 1);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "set volume fail");
    AUDIO_INFO_LOG("start success, frameSize: %{public}" PRIu64 ", frameCount: %{public}" PRIu64, frameSize,
        frameCount);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_MULTICHANNEL, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("MultichannelAudioRenderSink::Stop");
    AUDIO_INFO_LOG("in");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    if (!started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_ = false;
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::Resume(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (!paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->Resume(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    paused_ = false;
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_MULTICHANNEL, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("MultichannelAudioRenderSink::Pause");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->Pause(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::Flush(void)
{
    Trace trace("MultichannelAudioRenderSink::Flush");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::Reset(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }
    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }
    CheckUpdateState(&data, len);
    if (switchDeviceMute_) {
        Trace trace("MultichannelAudioRenderSink::RenderFrame::switch");
        if (memset_s(reinterpret_cast<void *>(&data), static_cast<size_t>(len), 0, static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s fail");
        }
    }
    if (emptyFrameCount_ > 0) {
        Trace trace("MultichannelAudioRenderSink::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void *>(&data), static_cast<size_t>(len), 0, static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s fail");
        }
        --emptyFrameCount_;
        if (emptyFrameCount_ == 0) {
            updateActiveDeviceCV_.notify_all();
        }
    }

    BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
    Trace trace("MultichannelAudioRenderSink::RenderFrame");
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), len);
    }
    int32_t ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t *>(&data), static_cast<uint32_t>(len),
        &writeLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_MULTICHANNEL, ClockTime::GetCurNano());
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_) {
        AUDIO_DEBUG_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }

    return SUCCESS;
}

int64_t MultichannelAudioRenderSink::GetVolumeDataCount()
{
    return volumeDataCount_;
}

int32_t MultichannelAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void MultichannelAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
}

std::string MultichannelAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    if (condition == "get_usb_info") {
        // init adapter to get parameter before load sink module (need fix)
        adapterNameCase_ = "usb";
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN_RET(deviceManager != nullptr, "");
        return deviceManager->GetAudioParameter(adapterNameCase_, key, condition);
    }
    return "";
}

int32_t MultichannelAudioRenderSink::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    leftVolume_ = left;
    rightVolume_ = right;
    float volume;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioRender_->SetVolume(audioRender_, volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

int32_t MultichannelAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::GetLatency(uint32_t &latency)
{
    Trace trace("MultichannelAudioRenderSink::GetLatency");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    uint32_t hdiLatency;
    int32_t ret = audioRender_->GetLatency(audioRender_, &hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail");
    latency = hdiLatency;
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float MultichannelAudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void MultichannelAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void MultichannelAudioRenderSink::SetAudioBalanceValue(float audioBalance)
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

int32_t MultichannelAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");
    if (!openSpeaker_) {
        return SUCCESS;
    }

    if (audioScene != currentAudioScene_) {
        struct AudioSceneDescriptor sceneDesc;
        InitSceneDesc(sceneDesc, audioScene);

        CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
        int32_t ret = audioRender_->SelectScene(audioRender_, &sceneDesc);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
        currentAudioScene_ = audioScene;
    }
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::GetAudioScene(void)
{
    return currentAudioScene_;
}

int32_t MultichannelAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() == 1, ERR_INVALID_PARAM, "invalid device");
    if (currentActiveDevice_ == outputDevices[0]) {
        AUDIO_INFO_LOG("output device not change, device: %{public}d", outputDevices[0]);
        return SUCCESS;
    }
    currentActiveDevice_ = outputDevices[0];

    emptyFrameCount_ = 5; // 5: frame count before update route
    std::unique_lock<std::mutex> lock(switchDeviceMutex_);
    updateActiveDeviceCV_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME_FOR_EMPTY_FRAME), [this] {
        if (emptyFrameCount_ == 0) {
            AUDIO_INFO_LOG("wait for empty frame end");
            return true;
        }
        AUDIO_DEBUG_LOG("emptyFrameCount: %{public}d", emptyFrameCount_.load());
        return false;
    });
    int32_t ret = DoSetOutputRoute(outputDevices);
    emptyFrameCount_ = 5; // 5: frame count after update route
    return ret;
}

void MultichannelAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
    AUDIO_INFO_LOG("regist succ");
}

void MultichannelAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    if (currentActiveDevice_ == device) {
        currentActiveDevice_ = DEVICE_TYPE_NONE;
    }
}

int32_t MultichannelAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t MultichannelAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t MultichannelAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void MultichannelAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: MchSink\tstarted: " + std::string(started_ ? "true" : "false") + "\thalName: " + halName_ +
        "\tcurrentActiveDevice: " + std::to_string(currentActiveDevice_) + "\n";
}

void MultichannelAudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

uint32_t MultichannelAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
{
    AudioFormat hdiFormat = ConvertToHdiFormat(format);
    switch (hdiFormat) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            return PCM_8_BIT;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            return PCM_24_BIT;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            return PCM_32_BIT;
        default:
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat MultichannelAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

AudioSampleFormat MultichannelAudioRenderSink::ParseAudioFormat(const std::string &format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return SAMPLE_S16LE;
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT" || format == "AUDIO_FORMAT_PCM_24_BIT_PACKED") {
        return SAMPLE_S24LE;
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return SAMPLE_S32LE;
    } else {
        return SAMPLE_S16LE;
    }
}

AudioCategory MultichannelAudioRenderSink::GetAudioCategory(AudioScene audioScene)
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
    AUDIO_DEBUG_LOG("audioCategory: %{public}d", audioCategory);

    return audioCategory;
}

void MultichannelAudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.channelCount = CHANNEL_6;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_MULTICHANNEL));
    param.type = AUDIO_MULTI_CHANNEL;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.channelLayout = attr_.channelLayout;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
}

void MultichannelAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.desc = const_cast<char *>("");
    deviceDesc.pins = PIN_OUT_SPEAKER;
    if (halName_ == HDI_ID_INFO_USB) {
        deviceDesc.pins = PIN_OUT_USB_HEADSET;
    }
}

void MultichannelAudioRenderSink::InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene)
{
    sceneDesc.scene.id = GetAudioCategory(audioScene);

    AudioPortPin port = PIN_OUT_SPEAKER;
    if (halName_ == HDI_ID_INFO_USB) {
        port = PIN_OUT_USB_HEADSET;
    }
    AUDIO_DEBUG_LOG("port: %{public}d", port);
    sceneDesc.desc.pins = port;
    sceneDesc.desc.desc = const_cast<char *>("");
}

int32_t MultichannelAudioRenderSink::CreateRender(void)
{
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, halName: %{public}s, rate: %{public}u, channel: %{public}u, format: %{public}u"
        "channelLayout: %{public}" PRIu64,
        halName_.c_str(), param.sampleRate, param.channelCount, param.format, param.channelLayout);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(adapterNameCase_, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct IAudioRender *>(render);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);

    AUDIO_INFO_LOG("create render success, hdiRenderId_: %{public}u", hdiRenderId_);
    return SUCCESS;
}

int32_t MultichannelAudioRenderSink::DoSetOutputRoute(std::vector<DeviceType> &outputDevices)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t ret = deviceManager->SetOutputRoute(adapterNameCase_, outputDevices,
        GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_MULTICHANNEL));
    return ret;
}

int32_t MultichannelAudioRenderSink::InitRender(void)
{
    AUDIO_INFO_LOG("in");
    if (renderInited_) {
        AUDIO_INFO_LOG("render already inited");
        return SUCCESS;
    }

    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
    renderInited_ = true;
    return SUCCESS;
}

void MultichannelAudioRenderSink::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8:
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        case SAMPLE_S16LE:
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        case SAMPLE_S24LE:
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<uint8_t *>(data), len);
            break;
        case SAMPLE_S32LE:
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void MultichannelAudioRenderSink::AdjustAudioBalance(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM8Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S16LE:
            AdjustAudioBalanceForPCM16Bit(reinterpret_cast<int16_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S24LE:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<uint8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S32LE:
            AdjustAudioBalanceForPCM32Bit(reinterpret_cast<int32_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void MultichannelAudioRenderSink::CheckUpdateState(char *data, uint64_t len)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), data, len);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
            }
        }
    }
}

// must be called with sinkMutex_ held
void MultichannelAudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_MULTICHANNEL),
        started);
}

int32_t MultichannelAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set multichannel mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("multichannel already muted");
            return SUCCESS;
        }
        switchDeviceMute_ = true;
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("multichannel not all unmuted");
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
    }

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
