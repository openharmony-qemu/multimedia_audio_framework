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
#define LOG_TAG "RemoteAudioCaptureSource"
#endif

#include "source/remote_audio_capture_source.h"
#include <climits>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace AudioStandard {
RemoteAudioCaptureSource::RemoteAudioCaptureSource(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
}

RemoteAudioCaptureSource::~RemoteAudioCaptureSource()
{
    if (sourceInited_.load()) {
        DeInit();
    }
}

int32_t RemoteAudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("in");
    attr_ = attr;
    sourceInited_.store(true);
    SetMute(muteState_);
    AUDIO_DEBUG_LOG("end");
    return SUCCESS;
}

void RemoteAudioCaptureSource::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    sourceInited_.store(false);
    captureInited_.store(false);
    started_.store(false);
    paused_.store(false);

    DestroyCapture();
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    AUDIO_INFO_LOG("end");
}

bool RemoteAudioCaptureSource::IsInited(void)
{
    return sourceInited_.load();
}

int32_t RemoteAudioCaptureSource::Start(void)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(createCaptureMutex_);
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, DUMP_REMOTE_CAPTURE_SOURCE_FILENAME, &dumpFile_);
    if (!captureInited_.load()) {
        int32_t ret = CreateCapture();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create capture fail");
        captureInited_.store(true);
    }

    if (started_.load()) {
        AUDIO_INFO_LOG("already started");
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    started_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::Stop(void)
{
    AUDIO_INFO_LOG("in");
    if (!started_.load()) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "stop fail, ret: %{public}d", ret);
    started_.store(false);
    if (captureInited_.load()) {
        DestroyCapture();
        captureInited_.store(false);
    }
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::Resume(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("already resumed");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail, ret: %{public}d", ret);
    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::Pause(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (paused_.load()) {
        AUDIO_INFO_LOG("already paused");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail, ret: %{public}d", ret);
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::Flush(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::Reset(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    if (!started_.load()) {
        AUDIO_WARNING_LOG("not start, invalid state");
        return ERR_ILLEGAL_STATE;
    }

    std::vector<int8_t> bufferVec(requestBytes);
    int32_t ret = audioCapture_->CaptureFrame(bufferVec, replyBytes);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_READ_FAILED, "fail, ret: %{public}x", ret);
    ret = memcpy_s(frame, requestBytes, bufferVec.data(), requestBytes);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "copy fail, error code: %{public}d", ret);
    replyBytes = requestBytes;

    DumpFileUtil::WriteDumpFile(dumpFile_, frame, requestBytes);
    CheckUpdateState(frame, requestBytes);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

std::string RemoteAudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t RemoteAudioCaptureSource::SetVolume(float left, float right)
{
    float leftVolume = left;
    float rightVolume = right;
    float volume;
    if ((leftVolume == 0) && (rightVolume != 0)) {
        volume = rightVolume;
    } else if ((leftVolume != 0) && (rightVolume == 0)) {
        volume = leftVolume;
    } else {
        volume = (leftVolume + rightVolume) / HALF_FACTOR;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->SetVolume(volume);
    AUDIO_INFO_LOG("left: %{public}f, right: %{public}f, ret: %{public}d", left, right, ret);
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    float val = 0.0;
    audioCapture_->GetVolume(val);
    left = val;
    right = val;
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::SetMute(bool isMute)
{
    muteState_ = isMute;
    if (!sourceInited_.load()) {
        AUDIO_INFO_LOG("source not init, just save mute state");
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->SetMute(isMute);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "set mute fail");
    return SUCCESS;
}
int32_t RemoteAudioCaptureSource::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    bool hdiMuteState = false;
    int32_t ret = audioCapture_->GetMute(hdiMuteState);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("get mute fail");
    }
    AUDIO_DEBUG_LOG("hdiMuteState: %{public}d, muteState: %{public}d", hdiMuteState, muteState_);
    isMute = muteState_;
    return SUCCESS;
}

uint64_t RemoteAudioCaptureSource::GetTransactionId(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float RemoteAudioCaptureSource::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t RemoteAudioCaptureSource::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");
    AUDIO_INFO_LOG("scene: %{public}d, device: %{public}d", audioScene, activeDevice);

    struct AudioSceneDescriptor sceneDesc = {
        .scene.id = GetAudioCategory(audioScene),
        .desc.pins = AudioPortPin::PIN_IN_MIC,
    };
    AUDIO_INFO_LOG("start");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->SelectScene(sceneDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
    AUDIO_INFO_LOG("end");
    return SUCCESS;
}

int32_t RemoteAudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void RemoteAudioCaptureSource::RegistCallback(uint32_t type, IAudioSourceCallback *callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

int32_t RemoteAudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return ERR_NOT_SUPPORTED;
}

void RemoteAudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: RemoteSource\tstarted: " + std::string(started_.load() ? "true" : "false") +
        "\tdeviceNetworkId: " + deviceNetworkId_ + "\n";
}

void RemoteAudioCaptureSource::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        DeInit();
    }

    callback_.OnCaptureSourceParamChange(adapterName, key, condition, value);
}

AudioFormat RemoteAudioCaptureSource::ConvertToHdiFormat(AudioSampleFormat format)
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

AudioCategory RemoteAudioCaptureSource::GetAudioCategory(AudioScene audioScene)
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

void RemoteAudioCaptureSource::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_REMOTE));
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = AUDIO_BUFFER_SIZE;
    // user need to set
    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    param.sourceType = attr_.sourceType;
}

void RemoteAudioCaptureSource::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = AudioPortPin::PIN_IN_MIC;
    deviceDesc.desc = "";
}

int32_t RemoteAudioCaptureSource::CreateCapture(void)
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
    CHECK_AND_RETURN_RET(audioCapture_ != nullptr, ERR_NOT_STARTED, "create capture fail");
    deviceManager->RegistCaptureSourceCallback(deviceNetworkId_, hdiCaptureId_, this);
    return SUCCESS;
}

void RemoteAudioCaptureSource::DestroyCapture(void)
{
    AUDIO_INFO_LOG("destroy capture");
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyCapture(deviceNetworkId_, hdiCaptureId_);
    deviceManager->UnRegistCaptureSourceCallback(deviceNetworkId_, hdiCaptureId_);
    audioCapture_.ForceSetRefPtr(nullptr);
    hdiCaptureId_ = 0;
}

void RemoteAudioCaptureSource::CheckUpdateState(char *frame, size_t replyBytes)
{
    if (startUpdate_) {
        if (captureFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        captureFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (captureFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            captureFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

} // namespace AudioStandard
} // namespace OHOS
