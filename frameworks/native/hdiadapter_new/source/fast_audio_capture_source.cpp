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
#define LOG_TAG "FastAudioCaptureSource"
#endif

#include "source/fast_audio_capture_source.h"
#include <climits>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
FastAudioCaptureSource::~FastAudioCaptureSource()
{
    AUDIO_INFO_LOG("destruction");
}

int32_t FastAudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("In, flag: %{public}d", attr.audioStreamFlag);
    attr_ = attr;

    int32_t ret = CreateCapture();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create capture fail");
    ret = DoSetInputRoute(static_cast<DeviceType>(attr_.deviceType));
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("update route fail, ret: %{public}d", ret);
    }
    ret = PrepareMmapBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "prepare mmap buffer fail");
    sourceInited_ = true;
    return SUCCESS;
}

void FastAudioCaptureSource::DeInit(void)
{
    AUDIO_INFO_LOG("in, flag: %{public}d, isCheckPositionSuccess: %{public}d", attr_.audioStreamFlag,
        isCheckPositionSuccess_.load());
    std::lock_guard<std::mutex> lock(statusMutex_);
    if (started_ || !isCheckPositionSuccess_) {
        StopInner();
        started_ = false;
    }
    sourceInited_ = false;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyCapture(attr_.adapterName, hdiCaptureId_);
    audioCapture_ = nullptr;

    callback_.OnCaptureState(false);
}

bool FastAudioCaptureSource::IsInited(void)
{
    return sourceInited_;
}

int32_t FastAudioCaptureSource::Start(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("in");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME));
        guard.CheckCurrTimeout();
    }
    if (runningLock_ != nullptr) {
        runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    } else {
        AUDIO_ERR_LOG("running lock is null, playback can not work well");
    }
#endif
    if (started_) {
        isCheckPositionSuccess_ = true;
        return SUCCESS;
    }
    callback_.OnCaptureState(true);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Start(audioCapture_);
    if (ret != SUCCESS) {
        goto ERR_RET;
    }
    ret = CheckPositionTime();
    if (ret != SUCCESS) {
        isCheckPositionSuccess_ = false;
        goto ERR_RET;
    }
    isCheckPositionSuccess_ = true;
    started_ = true;
    return SUCCESS;

ERR_RET:
    callback_.OnCaptureState(false);
    return ERR_NOT_STARTED;
}

int32_t FastAudioCaptureSource::Stop(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    return StopInner();
}


int32_t FastAudioCaptureSource::StopInner()
{
    AUDIO_INFO_LOG("in, isCheckPositionSuccess: %{public}d", isCheckPositionSuccess_.load());

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
    if (isCheckPositionSuccess_) {
        CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
        int32_t ret = audioCapture_->Stop(audioCapture_);
        callback_.OnCaptureState(false);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail, ret: %{public}d", ret);
    }
    started_ = false;
    return SUCCESS;
}

int32_t FastAudioCaptureSource::Resume(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    if (!paused_) {
        return SUCCESS;
    }
    int32_t ret = audioCapture_->Resume(audioCapture_);
    callback_.OnCaptureState(true);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    paused_ = false;
    return SUCCESS;
}

int32_t FastAudioCaptureSource::Pause(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioCapture_->Pause(audioCapture_);
    callback_.OnCaptureState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t FastAudioCaptureSource::Flush(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioCapture_->Flush(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t FastAudioCaptureSource::Reset(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioCapture_->Flush(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

int32_t FastAudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

std::string FastAudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

void FastAudioCaptureSource::SetAudioParameter(
    const AudioParamKey key, const std::string &condition, const std::string &value)
{
    AUDIO_WARNING_LOG("not support");
    return;
}

int32_t FastAudioCaptureSource::SetVolume(float left, float right)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioCaptureSource::GetVolume(float &left, float &right)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioCaptureSource::SetMute(bool isMute)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioCaptureSource::GetMute(bool &isMute)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

uint64_t FastAudioCaptureSource::GetTransactionId(void)
{
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t FastAudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float FastAudioCaptureSource::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

int32_t FastAudioCaptureSource::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");
    AUDIO_INFO_LOG("scene: %{public}d", audioScene);

    if (audioScene != currentAudioScene_) {
        struct AudioSceneDescriptor sceneDesc;
        InitSceneDesc(sceneDesc, audioScene);

        CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
        int32_t ret = audioCapture_->SelectScene(audioCapture_, &sceneDesc);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
        currentAudioScene_ = audioScene;
    }
    return SUCCESS;
}

int32_t FastAudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    return DoSetInputRoute(inputDevice);
}

void FastAudioCaptureSource::RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

int32_t FastAudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t FastAudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void FastAudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: FastSource\tstarted: " + std::string(started_ ? "true" : "false") + "\n";
}

int32_t FastAudioCaptureSource::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
    uint32_t &byteSizePerFrame, uint32_t &syncInfoSize)
{
    CHECK_AND_RETURN_RET_LOG(bufferFd_ != INVALID_FD, ERR_INVALID_HANDLE, "buffer fd has been released");
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    syncInfoSize = syncInfoSize_;
    return SUCCESS;
}

int32_t FastAudioCaptureSource::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioCapture_->GetMmapPosition(audioCapture_, &frames, &stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get mmap position fail, ret: %{public}d", ret);

    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

uint32_t FastAudioCaptureSource::PcmFormatToBit(AudioSampleFormat format)
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
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat FastAudioCaptureSource::ConvertToHdiFormat(AudioSampleFormat format)
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

enum AudioInputType FastAudioCaptureSource::ConvertToHDIAudioInputType(int32_t sourceType)
{
    enum AudioInputType hdiAudioInputType;
    switch (sourceType) {
        case SOURCE_TYPE_INVALID:
            hdiAudioInputType = AUDIO_INPUT_DEFAULT_TYPE;
            break;
        case SOURCE_TYPE_MIC:
        case SOURCE_TYPE_PLAYBACK_CAPTURE:
        case SOURCE_TYPE_ULTRASONIC:
        case SOURCE_TYPE_UNPROCESSED:
        case SOURCE_TYPE_LIVE:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
        case SOURCE_TYPE_WAKEUP:
            hdiAudioInputType = AUDIO_INPUT_SPEECH_WAKEUP_TYPE;
            break;
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
        default:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
    }
    return hdiAudioInputType;
}

AudioCategory FastAudioCaptureSource::GetAudioCategory(AudioScene audioScene)
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

void FastAudioCaptureSource::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_FAST));
    param.period = 0;
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.startThreshold = 0;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sourceType = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));
    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AUDIO_MMAP_VOIP : AUDIO_MMAP_NOIRQ; // enable mmap!
    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    if (param.channelCount == MONO) {
        param.channelLayout = CH_LAYOUT_MONO;
    } else if (param.channelCount == STEREO) {
        param.channelLayout = CH_LAYOUT_STEREO;
    }
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    param.startThreshold = 0;
}

void FastAudioCaptureSource::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    switch (static_cast<DeviceType>(attr_.deviceType)) {
        case DEVICE_TYPE_MIC:
            deviceDesc.pins = PIN_IN_MIC;
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            deviceDesc.pins = PIN_IN_HS_MIC;
            break;
        case DEVICE_TYPE_USB_HEADSET:
            deviceDesc.pins = PIN_IN_USB_EXT;
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            deviceDesc.pins = PIN_IN_BLUETOOTH_SCO_HEADSET;
            break;
        default:
            AUDIO_WARNING_LOG("unsupport, use default, deviceType: %{public}d", attr_.deviceType);
            deviceDesc.pins = PIN_IN_MIC;
            break;
    }
    deviceDesc.desc = const_cast<char *>("");
}

void FastAudioCaptureSource::InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene)
{
    sceneDesc.scene.id = GetAudioCategory(audioScene);
    sceneDesc.desc.pins = PIN_IN_BLUETOOTH_SCO_HEADSET;
    sceneDesc.desc.desc = const_cast<char *>("pin_in_bluetooth_sco_headset");
}

int32_t FastAudioCaptureSource::CreateCapture(void)
{
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create capture, type: %{public}d, rate: %{public}u, channel: %{public}u, format: %{public}u, "
        "device: %{public}u", param.type, param.sampleRate, param.channelCount, param.format, attr_.deviceType);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *capture = deviceManager->CreateCapture(attr_.adapterName, &param, &deviceDesc, hdiCaptureId_);
    audioCapture_ = static_cast<struct IAudioCapture *>(capture);
    CHECK_AND_RETURN_RET(audioCapture_ != nullptr, ERR_NOT_STARTED);

    return SUCCESS;
}

int32_t FastAudioCaptureSource::DoSetInputRoute(DeviceType inputDevice)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_FAST));
    int32_t inputType = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));
    AUDIO_INFO_LOG("adapterName: %{public}s, inputDevice: %{public}d, streamId: %{public}d, input :%{public}d",
        attr_.adapterName.c_str(), inputDevice, streamId, inputType);
    int32_t ret = deviceManager->SetInputRoute(attr_.adapterName, inputDevice, streamId, inputType);
    return ret;
}

int32_t FastAudioCaptureSource::PrepareMmapBuffer(void)
{
    uint32_t totalBufferInMs = 40; // 40: 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency
    uint32_t reqBufferFrameSize = totalBufferInMs * (attr_.sampleRate / SECOND_TO_MILLISECOND);
    struct AudioMmapBufferDescriptor desc;
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    int32_t ret = audioCapture_->ReqMmapBuffer(audioCapture_, reqBufferFrameSize, &desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "request mmap buffer fail, ret: %{public}d", ret);
    AUDIO_INFO_LOG("memoryFd: [%{public}d], totalBufferFrames: [%{public}d], "
        "transferFrameSize: [%{public}d], isShareable: [%{public}d], offset: [%{public}d]", desc.memoryFd,
        desc.totalBufferFrames, desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030, 3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "invalid value, totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d]", desc.totalBufferFrames,
        desc.transferFrameSize);

    uint32_t frameSizeInByte = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames); // 1440 ~ 3840
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize); // 240
    CHECK_AND_RETURN_RET_LOG(frameSizeInByte <= ULLONG_MAX / bufferTotalFrameSize_, ERR_OPERATION_FAILED,
        "buffer size will overflow");

    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte;
    return SUCCESS;
}

int32_t FastAudioCaptureSource::CheckPositionTime(void)
{
    int32_t tryCount = MAX_GET_POSITION_TRY_COUNT;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int64_t maxHandleTime = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? VOIP_MAX_GET_POSITION_HANDLE_TIME :
        GENERAL_MAX_GET_POSITION_HANDLE_TIME;
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(MAX_GET_POSITION_WAIT_TIME);
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        AUDIO_WARNING_LOG("sec: %{public}" PRId64 ", nanoSec: %{public}" PRId64 ", time cost: %{public}" PRId64,
            timeSec, timeNanoSec, ClockTime::GetCurNano() - curTime);
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > maxHandleTime) {
            AUDIO_WARNING_LOG("tryCount: %{public}d, ret: %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("check succ");
            return SUCCESS;
        }
    }
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    AUDIO_ERR_LOG("fail, stop capture");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Stop(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "stop fail, ret: %{public}d", ret);
    return ERR_OPERATION_FAILED;
}

void FastAudioCaptureSource::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

} // namespace AudioStandard
} // namespace OHOS
