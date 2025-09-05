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
#define LOG_TAG "RemoteFastAudioCaptureSource"
#endif

#include "source/remote_fast_audio_capture_source.h"
#include <climits>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace AudioStandard {
RemoteFastAudioCaptureSource::RemoteFastAudioCaptureSource(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_INFO_LOG("construction");
}

RemoteFastAudioCaptureSource::~RemoteFastAudioCaptureSource()
{
    AUDIO_INFO_LOG("destruction");
    if (sourceInited_.load()) {
        DeInit();
    }
    AUDIO_INFO_LOG("end");
}

int32_t RemoteFastAudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("in");
    attr_ = attr;

    if (!captureInited_.load()) {
        int32_t ret = CreateCapture();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create capture fail");
    }

    captureInited_.store(true);
    sourceInited_.store(true);

#ifdef DEBUG_DIRECT_USE_HDI
    AUDIO_INFO_LOG("attr: [%{public}s]", PrintAttr().c_str());
    dumpFile_ = fopen(dumpFileName_, "a+");
    AUDIO_INFO_LOG("dumpFileName: %{public}s", dumpFileName_);
    if (dumpFile_ == nullptr) {
        AUDIO_WARNING_LOG("open dump file fail");
    }
#endif
    AUDIO_INFO_LOG("end");
    return SUCCESS;
}

void RemoteFastAudioCaptureSource::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    sourceInited_.store(false);
    captureInited_.store(false);
    started_.store(false);
    paused_.store(false);
    muteState_.store(false);

#ifdef DEBUG_DIRECT_USE_HDI
    if (dumpFile_) {
        fclose(dumpFile_);
        dumpFile_ = nullptr;
    }
    if (ashmemSource_ != nullptr) {
        ashmemSource_->UnmapAshmem();
        ashmemSource_->CloseAshmem();
        ashmemSource_ = nullptr;
        AUDIO_INFO_LOG("deinit ashmem source succ");
    }
#endif

    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyCapture(deviceNetworkId_, hdiCaptureId_);
    deviceManager->UnRegistCaptureSourceCallback(deviceNetworkId_, hdiCaptureId_);
    audioCapture_.ForceSetRefPtr(nullptr);
    AUDIO_DEBUG_LOG("end");
}

bool RemoteFastAudioCaptureSource::IsInited(void)
{
    return sourceInited_.load();
}

int32_t RemoteFastAudioCaptureSource::Start(void)
{
    AUDIO_INFO_LOG("in");
    if (!captureInited_.load()) {
        int32_t ret = CreateCapture();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create capture fail");
        captureInited_.store(true);
    }
    if (started_.load()) {
        AUDIO_INFO_LOG("already start");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    ret = CheckPositionTime();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "check position time fail, ret: %{public}d", ret);
    started_.store(true);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::Stop(void)
{
    AUDIO_INFO_LOG("in");
    if (!started_.load()) {
        AUDIO_INFO_LOG("already stop");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail, ret: %{public}d", ret);
    started_.store(false);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::Resume(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("already resume");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "resume fail, ret: %{public}d", ret);
    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::Pause(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (paused_.load()) {
        AUDIO_INFO_LOG("already pause");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "pause fail, ret: %{public}d", ret);
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::Flush(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "flush fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::Reset(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "reset fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

std::string RemoteFastAudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

void RemoteFastAudioCaptureSource::SetAudioParameter(
    const AudioParamKey key, const std::string &condition, const std::string &value)
{
    AUDIO_WARNING_LOG("not support");
    return;
}

int32_t RemoteFastAudioCaptureSource::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    AUDIO_INFO_LOG("left: %{public}f, right: %{public}f", left, right);
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

    int32_t ret = audioCapture_->SetVolume(volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

int32_t RemoteFastAudioCaptureSource::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    float val = 0.0;
    audioCapture_->GetVolume(val);
    left = val;
    right = val;
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::SetMute(bool isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->SetMute(isMute);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "set mute fail");
    muteState_ = isMute;
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    bool hdiMuteState = false;
    int32_t ret = audioCapture_->GetMute(hdiMuteState);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("get mute fail");
    }
    AUDIO_DEBUG_LOG("hdiMuteState: %{public}d, muteState: %{public}d", hdiMuteState, muteState_.load());
    isMute = muteState_;
    return SUCCESS;
}

uint64_t RemoteFastAudioCaptureSource::GetTransactionId(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float RemoteFastAudioCaptureSource::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

int32_t RemoteFastAudioCaptureSource::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");
    AUDIO_INFO_LOG("scene: %{public}d", audioScene);

    struct AudioSceneDescriptor sceneDesc;
    InitSceneDesc(sceneDesc, audioScene);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->SelectScene(sceneDesc);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void RemoteFastAudioCaptureSource::RegistCallback(uint32_t type, IAudioSourceCallback *callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

int32_t RemoteFastAudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return ERR_NOT_SUPPORTED;
}

void RemoteFastAudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: RemoteFastSource\tstarted: " + std::string(started_.load() ? "true" : "false") +
        "\tdeviceNetworkId: " + deviceNetworkId_ + "\n";
}

void RemoteFastAudioCaptureSource::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        DeInit();
    }

    callback_.OnCaptureSourceParamChange(adapterName, key, condition, value);
}

int32_t RemoteFastAudioCaptureSource::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
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

int32_t RemoteFastAudioCaptureSource::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioCapture_->GetMmapPosition(frames, stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get mmap position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invaild time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

uint32_t RemoteFastAudioCaptureSource::PcmFormatToBit(enum AudioSampleFormat format)
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
            return PCM_16_BIT;
    }
}

AudioFormat RemoteFastAudioCaptureSource::ConvertToHdiFormat(AudioSampleFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16LE:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24LE:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32LE:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }

    return hdiFormat;
}

AudioCategory RemoteFastAudioCaptureSource::GetAudioCategory(AudioScene audioScene)
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

void RemoteFastAudioCaptureSource::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.interleaved = CAPTURE_INTERLEAVED;
    param.format = ConvertToHdiFormat(attr_.format);
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    param.frameSize = param.format * param.channelCount;
    param.isBigEndian = attr_.isBigEndian;
    param.isSignedData = true;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    }
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = attr_.bufferSize;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_REMOTE_FAST));

    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AudioCategory::AUDIO_MMAP_VOIP :
        AudioCategory::AUDIO_MMAP_NOIRQ;
}

void RemoteFastAudioCaptureSource::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = AudioPortPin::PIN_IN_MIC;
    deviceDesc.desc = "";
}

void RemoteFastAudioCaptureSource::InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene)
{
    sceneDesc.scene.id = GetAudioCategory(audioScene);
    sceneDesc.desc.pins = AudioPortPin::PIN_IN_MIC;
}

int32_t RemoteFastAudioCaptureSource::CreateCapture(void)
{
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create capture, format: %{public}u", param.format);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);

    void *capture = deviceManager->CreateCapture(deviceNetworkId_, &param, &deviceDesc, hdiCaptureId_);
    audioCapture_.ForceSetRefPtr(static_cast<IAudioCapture *>(capture));
    CHECK_AND_RETURN_RET(audioCapture_ != nullptr, ERR_NOT_STARTED);
    deviceManager->RegistCaptureSourceCallback(deviceNetworkId_, hdiCaptureId_, this);
    if (param.type == AudioCategory::AUDIO_MMAP_NOIRQ || param.type == AudioCategory::AUDIO_MMAP_VOIP) {
        PrepareMmapBuffer(param);
    }
    AUDIO_INFO_LOG("end");
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::PrepareMmapBuffer(const AudioSampleAttributes &param)
{
    int32_t totalBufferInMs = 40; // 40: 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency
    int32_t reqBufferFrameSize = totalBufferInMs * static_cast<int32_t>(attr_.sampleRate / SECOND_TO_MILLISECOND);
    struct AudioMmapBufferDescriptor desc;

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->ReqMmapBuffer(reqBufferFrameSize, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "request mmap buffer fail, ret:%{public}d", ret);
    AUDIO_DEBUG_LOG("memoryFd: [%{public}d], totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d], "
        "isShareable: [%{public}d], offset: [%{public}d]", desc.memoryFd, desc.totalBufferFrames,
        desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030, 3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "invalid value, totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d]", desc.totalBufferFrames,
        desc.transferFrameSize);

    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames);
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize);

#ifdef DEBUG_DIRECT_USE_HDI
    bufferSize_ = bufferTotalFrameSize_ * param.channelCount * param.format;
    ashmemSource_ = new Ashmem(bufferFd_, bufferSize_);
    AUDIO_DEBUG_LOG("create ashmem source succ, ashmemLen: %{public}zu", bufferSize_);
    bool tmp = ashmemSource_->MapReadAndWriteAshmem();
    CHECK_AND_RETURN_RET_LOG(tmp, ERR_OPERATION_FAILED, "map ashmem source fail");
#endif
    return SUCCESS;
}

int32_t RemoteFastAudioCaptureSource::CheckPositionTime(void)
{
    int32_t tryCount = MAX_GET_POSITION_TRY_COUNT;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(MAX_GET_POSITION_WAIT_TIME);
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > MAX_GET_POSITION_HANDLE_TIME) {
            AUDIO_WARNING_LOG("tryCount: %{public}d, ret: %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("check succ");
            return SUCCESS;
        }
    }
    return ERR_OPERATION_FAILED;
}

void RemoteFastAudioCaptureSource::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

} // namespace AudioStandard
} // namespace OHOS
