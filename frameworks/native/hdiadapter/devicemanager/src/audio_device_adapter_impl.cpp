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
#define LOG_TAG "AudioDeviceAdapterImpl"
#endif

#include "audio_device_adapter_impl.h"

#include <securec.h>

#include "audio_errors.h"
#include "audio_hdi_log.h"

namespace OHOS {
namespace AudioStandard {
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCallbackType;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey;

std::shared_ptr<AudioDeviceAdapterImpl> AudioDeviceAdapterImpl::paramCallback_ = nullptr;

class AudioParamCallbackImpl final : public IAudioCallback {
public:
    AudioParamCallbackImpl() {}
    ~AudioParamCallbackImpl() override {}

    int32_t RenderCallback(AudioCallbackType type, int8_t &reserved, int8_t &cookie) override;
    int32_t ParamCallback(AudioExtParamKey key, const std::string &condition, const std::string &value,
        int8_t &reserved, int8_t cookie) override;
};

int32_t AudioParamCallbackImpl::RenderCallback(AudioCallbackType type, int8_t &reserved, int8_t &cookie)
{
    (void) type;
    (void) reserved;
    (void) cookie;
    return SUCCESS;
}

int32_t AudioParamCallbackImpl::ParamCallback(AudioExtParamKey key, const std::string &condition,
    const std::string &value, int8_t &reserved, int8_t cookie)
{
    (void) cookie;
    AUDIO_INFO_LOG("key %{public}d, condition %{public}s, value %{public}s", key, condition.c_str(), value.c_str());
    std::shared_ptr<AudioDeviceAdapterImpl> cookies = AudioDeviceAdapterImpl::GetParamCallback();
    AudioDeviceAdapterImpl::ParamEventCallback(static_cast<::AudioExtParamKey>(key), condition.c_str(),
        value.c_str(), static_cast<void *>(&reserved), cookies);
    return SUCCESS;
}

int32_t AudioDeviceAdapterImpl::Init()
{
    AUDIO_INFO_LOG("Init start.");

    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "Init: audio adapter is null.");
    int32_t ret = audioAdapter_->InitAllPorts();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_DEVICE_INIT, "InitAllPorts fail, ret %{public}d.", ret);

    ret = RegExtraParamObserver();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init: Register extra param observer fail, ret %{public}d.", ret);
    AUDIO_DEBUG_LOG("Init end.");
    return SUCCESS;
}

size_t AudioDeviceAdapterImpl::GetRenderPortsNum()
{
    std::lock_guard<std::mutex> lock(renderPortsMtx_);
    return renderPorts_.size();
}

size_t AudioDeviceAdapterImpl::GetCapturePortsNum()
{
    std::lock_guard<std::mutex> lock(capturePortsMtx_);
    return capturePorts_.size();
}

int32_t AudioDeviceAdapterImpl::HandleRenderParamEvent(std::shared_ptr<AudioDeviceAdapterImpl> devAdapter,
    const AudioParamKey audioKey, const char *condition, const char *value)
{
    CHECK_AND_RETURN_RET_LOG(devAdapter != nullptr && condition != nullptr && value != nullptr,
        ERR_INVALID_HANDLE, "some params are null.");

    IAudioDeviceAdapterCallback *sink = nullptr;
    {
        std::lock_guard<std::mutex> lock(devAdapter->renderPortsMtx_);
        if (devAdapter->renderPorts_.size() != 1) {
            AUDIO_WARNING_LOG("Please check renderId or other infos.");
        }
        for (auto i = devAdapter->renderPorts_.begin(); i != devAdapter->renderPorts_.end();) {
            sink = i->second.devAdpCb;
            if (sink == nullptr) {
                AUDIO_ERR_LOG("Device adapter sink callback is null.");
                devAdapter->renderPorts_.erase(i++);
                continue;
            }
            break;
        }
    }

    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE,
        "Not find daudio sink port in adapter, condition %{public}s.", condition);
    sink->OnAudioParamChange(devAdapter->adapterName_, audioKey, std::string(condition), std::string(value));
    return SUCCESS;
}

int32_t AudioDeviceAdapterImpl::HandleCaptureParamEvent(std::shared_ptr<AudioDeviceAdapterImpl> devAdapter,
    const AudioParamKey audioKey, const char *condition, const char *value)
{
    CHECK_AND_RETURN_RET_LOG(devAdapter != nullptr && condition != nullptr && value != nullptr,
        ERR_INVALID_HANDLE, "some params are null.");

    IAudioDeviceAdapterCallback *source = nullptr;
    {
        std::lock_guard<std::mutex> lock(devAdapter->capturePortsMtx_);
        if (devAdapter->capturePorts_.size() != 1) {
            AUDIO_WARNING_LOG("Please check captureId or other infos.");
        }
        for (auto j = devAdapter->capturePorts_.begin(); j != devAdapter->capturePorts_.end();) {
            source = j->second.devAdpCb;
            if (source == nullptr) {
                AUDIO_ERR_LOG("Device adapter source callback is null.");
                devAdapter->capturePorts_.erase(j++);
                continue;
            }
            break;
        }
    }

    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE,
        "Not find daudio source port in adapter, condition %{public}s.", condition);
    source->OnAudioParamChange(devAdapter->adapterName_, audioKey, std::string(condition), std::string(value));
    return SUCCESS;
}

int32_t AudioDeviceAdapterImpl::HandleStateChangeEvent(std::shared_ptr<AudioDeviceAdapterImpl> devAdapter,
    const AudioParamKey audioKey, const char *condition, const char *value)
{
    CHECK_AND_RETURN_RET_LOG(devAdapter != nullptr && condition != nullptr && value == nullptr,
        ERR_INVALID_HANDLE, "some params are null.");

    char eventDes[EVENT_DES_SIZE];
    char contentDes[ADAPTER_STATE_CONTENT_DES_SIZE];
    CHECK_AND_RETURN_RET_LOG(
        sscanf_s(condition, "%[^;];%s", eventDes, EVENT_DES_SIZE, contentDes, ADAPTER_STATE_CONTENT_DES_SIZE)
        == PARAMS_STATE_NUM, ERR_INVALID_PARAM, "ParamEventCallback: Failed parse condition");
    CHECK_AND_RETURN_RET_LOG(strcmp(eventDes, "ERR_EVENT") == 0, ERR_NOT_SUPPORTED,
        "Event %{public}s is not supported.", eventDes);

    AUDIO_INFO_LOG("render state invalid, destroy audioRender");

    std::string devTypeKey = "DEVICE_TYPE=";
    std::string contentDesStr = std::string(contentDes);
    size_t devTypeKeyPos =  contentDesStr.find(devTypeKey);
    CHECK_AND_RETURN_RET_LOG(devTypeKeyPos != std::string::npos, ERR_INVALID_PARAM,
        "Not find daudio device type info, contentDes %{public}s.", contentDesStr.c_str());
    size_t devTypeValPos = devTypeKeyPos + devTypeKey.length();
    CHECK_AND_RETURN_RET_LOG(devTypeValPos < contentDesStr.length(), ERR_INVALID_PARAM,
        "Not find daudio device type value, contentDes %{public}s.", contentDesStr.c_str());

    int32_t ret = SUCCESS;
    if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_SPK) {
        AUDIO_INFO_LOG("ERR_EVENT of DAUDIO_DEV_TYPE_SPK.");
        ret = HandleRenderParamEvent(devAdapter, audioKey, condition, value);
    } else if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_MIC) {
        AUDIO_INFO_LOG("ERR_EVENT of DAUDIO_DEV_TYPE_MIC.");
        ret = HandleCaptureParamEvent(devAdapter, audioKey, condition, value);
    } else {
        AUDIO_ERR_LOG("Device type is not supported, contentDes %{public}s.",
            contentDesStr.c_str());
        return ERR_NOT_SUPPORTED;
    }

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Handle state error change event %{public}s fail.", contentDesStr.c_str());
    return SUCCESS;
}

int32_t AudioDeviceAdapterImpl::ParamEventCallback(AudioExtParamKey key, const char *condition, const char *value,
    void *reserved, std::shared_ptr<AudioDeviceAdapterImpl> cookie)
{
    AUDIO_INFO_LOG("key %{public}d, condition %{public}s, value %{public}s",
        key, condition, value);
    AudioParamKey audioKey = AudioParamKey(key);
    int32_t ret = SUCCESS;
    switch (audioKey) {
        case AudioParamKey::PARAM_KEY_STATE:
            ret = HandleStateChangeEvent(cookie, audioKey, condition, value);
            break;
        case AudioParamKey::VOLUME:
        case AudioParamKey::INTERRUPT:
            HandleRenderParamEvent(cookie, audioKey, condition, value);
            break;
        default:
            AUDIO_ERR_LOG("Audio param key %{public}d is not supported.", audioKey);
            return ERR_NOT_SUPPORTED;
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Handle audio param key %{public}d fail, ret %{public}d.", audioKey, ret);
    return SUCCESS;
}

void AudioDeviceAdapterImpl::SetParamCallback(std::shared_ptr<AudioDeviceAdapterImpl> callback)
{
    AudioDeviceAdapterImpl::paramCallback_ = callback;
}

std::shared_ptr<AudioDeviceAdapterImpl> AudioDeviceAdapterImpl::GetParamCallback()
{
    return AudioDeviceAdapterImpl::paramCallback_;
}

int32_t AudioDeviceAdapterImpl::RegExtraParamObserver()
{
    AUDIO_INFO_LOG("Register extra param observer.");
#ifdef FEATURE_DISTRIBUTE_AUDIO
    std::lock_guard<std::mutex> lock(regParamCbMtx_);
    if (isParamCbReg_.load()) {
        AUDIO_INFO_LOG("Audio adapter already registered extra param observer.");
        return SUCCESS;
    }

    callbackStub_ = new AudioParamCallbackImpl();
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "audio adapter is null.");

    int32_t ret = audioAdapter_->RegExtraParamObserver(callbackStub_, 0);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
        "Register extra param observer fail, ret %{public}d", ret);

    isParamCbReg_.store(true);
    AUDIO_INFO_LOG("Register extra param observer for daudio OK.");
#endif
    return SUCCESS;
}

int32_t AudioDeviceAdapterImpl::CreateRender(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
    sptr<IAudioRender> &audioRender, IAudioDeviceAdapterCallback *renderCb, uint32_t &renderId)
{
    AUDIO_INFO_LOG("Create render start.");
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "audio adapter is null.");
    int32_t ret = audioAdapter_->CreateRender(devDesc, attr, audioRender, renderId);
    CHECK_AND_RETURN_RET_LOG(ret == 0 && audioRender != nullptr, ERR_NOT_STARTED, "AudioDeviceCreateRender failed");

    std::lock_guard<std::mutex> lock(renderPortsMtx_);
    if (renderPorts_.find(audioRender) != renderPorts_.end()) {
        AUDIO_DEBUG_LOG("Audio render already exit in renderPorts, will replace new port info.");
    }

    DevicePortInfo renderPortInfo = {
        .devAdpCb = renderCb,
        .portId = 0,
    };
    renderPorts_[audioRender] = renderPortInfo;
    return SUCCESS;
}

void AudioDeviceAdapterImpl::DestroyRender(sptr<IAudioRender> audioRender, uint32_t &renderId)
{
    CHECK_AND_RETURN_LOG(audioRender != nullptr, "DestroyRender: Audio render is null.");
    {
        std::lock_guard<std::mutex> lock(renderPortsMtx_);
        AUDIO_INFO_LOG("Destroy render start.");
        if (renderPorts_.find(audioRender) == renderPorts_.end()) {
            AUDIO_DEBUG_LOG("Audio render is already destoried.");
            return;
        }
    }

    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "DestroyRender: Audio adapter is null.");
    audioAdapter_->DestroyRender(renderId);

    {
        std::lock_guard<std::mutex> lock(renderPortsMtx_);
        renderPorts_.erase(audioRender);
    }
}

int32_t AudioDeviceAdapterImpl::CreateCapture(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
    sptr<IAudioCapture> &audioCapture, IAudioDeviceAdapterCallback *captureCb, uint32_t &captureId)
{
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "audio adapter is null.");
    int32_t ret = audioAdapter_->CreateCapture(devDesc, attr, audioCapture, captureId);
    CHECK_AND_RETURN_RET_LOG(ret == 0 && audioCapture != nullptr,
        ERR_NOT_STARTED, "Create capture failed, error code %{public}d.", ret);

    std::lock_guard<std::mutex> lock(capturePortsMtx_);
    if (capturePorts_.find(audioCapture) != capturePorts_.end()) {
        AUDIO_INFO_LOG("Audio capture already exit in capturePorts, will replace new port info.");
    }

    DevicePortInfo capturePortInfo = {
        .devAdpCb = captureCb,
        .portId = 0,
    };
    capturePorts_[audioCapture] = capturePortInfo;
    return SUCCESS;
}

void AudioDeviceAdapterImpl::DestroyCapture(sptr<IAudioCapture> audioCapture, uint32_t &captureId)
{
    CHECK_AND_RETURN_LOG(audioCapture != nullptr, "Audio capture is null.");
    {
        std::lock_guard<std::mutex> lock(capturePortsMtx_);
        AUDIO_INFO_LOG("Destroy capture start.");
        if (capturePorts_.find(audioCapture) == capturePorts_.end()) {
            AUDIO_INFO_LOG("Audio capture is already destoried.");
            return;
        }
    }

    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "Audio adapter is null.");
    audioAdapter_->DestroyCapture(captureId);

    {
        std::lock_guard<std::mutex> lock(capturePortsMtx_);
        capturePorts_.erase(audioCapture);
    }
}

void AudioDeviceAdapterImpl::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("key %{public}d, condition: %{public}s, value: %{public}s.",
        key, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(audioAdapter_ != nullptr, "Audio adapter is null.");

    enum AudioExtParamKey hdiKey = AudioExtParamKey(key);
    int32_t ret = audioAdapter_->SetExtraParams(hdiKey, condition, value);
    CHECK_AND_RETURN_LOG(ret == 0, "Set audio parameter fail, ret %{public}d.", ret);
#else
    AUDIO_INFO_LOG("SetAudioParameter is not supported.");
#endif
}

std::string AudioDeviceAdapterImpl::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("GetParameter: key %{public}d, condition: %{public}s", key,
        condition.c_str());
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, "", "Audio adapter is null.");
    enum AudioExtParamKey hdiKey = AudioExtParamKey(key);
    std::string value;
    int32_t ret = audioAdapter_->GetExtraParams(hdiKey, condition.c_str(), value);
    CHECK_AND_RETURN_RET_LOG(ret == 0, "", "Get audio parameter fail, ret %{public}d", ret);
    return value;
#else
    AUDIO_INFO_LOG("GetAudioParameter is not supported.");
    return "";
#endif
}

int32_t AudioDeviceAdapterImpl::UpdateAudioRoute(const AudioRoute &route)
{
    AUDIO_INFO_LOG("UpdateAudioRoute enter.");
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE,
        "UpdateAudioRoute: Audio adapter is null.");
    int32_t ret = audioAdapter_->UpdateAudioRoute(route, routeHandle_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Update audio route fail, ret %{public}d", ret);
    return SUCCESS;
}

int32_t AudioDeviceAdapterImpl::Release()
{
    AUDIO_INFO_LOG("Release enter.");
    if (audioAdapter_ == nullptr) {
        AUDIO_INFO_LOG("Audio adapter is already released.");
        return SUCCESS;
    }

    size_t capturePortsNum = GetCapturePortsNum();
    size_t renderPortsNum = GetRenderPortsNum();
    CHECK_AND_RETURN_RET_LOG(capturePortsNum + renderPortsNum == 0, ERR_ILLEGAL_STATE,
        "Audio adapter has some ports busy, capturePortsNum %zu, renderPortsNum %zu.",
        capturePortsNum, renderPortsNum);
    if (routeHandle_ != INVALID_ROUT_HANDLE) {
        audioAdapter_->ReleaseAudioRoute(routeHandle_);
    }
    audioAdapter_ = nullptr;
    AudioDeviceAdapterImpl::paramCallback_ = nullptr;
    AUDIO_DEBUG_LOG("Release end.");
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS
