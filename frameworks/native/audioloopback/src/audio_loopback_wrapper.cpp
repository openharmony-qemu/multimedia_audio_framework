/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_loopback_wrapper.h"
#include "audio_policy_log.h"
#include "audio_debug_manager.h"

#undef LOG_TAG
#define LOG_TAG "AudioLoopbackWrapper"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const int32_t WAIT_SERVER_RESULT_TIMEOUT_IN_MS = 1500; // 1500MS
    static const int32_t WAIT_SERVER_STATUS_TIMEOUT_IN_MS = 200; //200MS
};
AudioLoopbackWrapper::AudioLoopbackWrapper(AudioLoopbackMode mode, LoopbackType type, const AppInfo &appInfo)
    : mode_(mode), type_(type), appInfo_(appInfo)
{
    AUDIO_INFO_LOG("AudioLoopbackWrapper created, mode %{public}d, type %{public}d", mode, type);
    
    if (type == LOOPBACK_TYPE_NORMAL || type == LOOPBACK_TYPE_GLOBAL_HANDLER) {
        bool isGlobal = type == LOOPBACK_TYPE_GLOBAL_HANDLER;
        private_ = std::make_shared<AudioLoopbackPrivate>(mode, appInfo, isGlobal);
    }
}

AudioLoopbackWrapper::~AudioLoopbackWrapper()
{
    AUDIO_INFO_LOG("AudioLoopbackWrapper destroyed");

    if (loopbackProxy_ != nullptr) {
        AudioPolicyManager::GetInstance().DestroyLoopback(type_);
        loopbackProxy_ = nullptr;
    }

    if (debugKey_ != 0) {
        (void)AudioDebugManager::GetInstance().UnregisterAudioLoopback(debugKey_);
    }

    callbackStub_ = nullptr;
    private_ = nullptr;
}

int32_t AudioLoopbackWrapper::Init()
{
    return CreateLoopbackIpc();
}

int32_t AudioLoopbackWrapper::CreateLoopbackIpc()
{
    callbackStub_ = new LoopbackCallback(shared_from_this());
    sptr<IRemoteObject> loopbackRemote;
    int32_t result = AudioPolicyManager::GetInstance().CreateLoopback(mode_, type_, callbackStub_, loopbackRemote);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("CreateLoopback failed, result %{public}d", result);
        return result;
    }
    loopbackProxy_ = iface_cast<ILoopback>(loopbackRemote);
    CHECK_AND_RETURN_RET_LOG(loopbackProxy_ != nullptr, ERR_OPERATION_FAILED, "create failed.");
    AUDIO_INFO_LOG("CreateLoopback success");
    return SUCCESS;
}

bool AudioLoopbackWrapper::Enable(bool enable)
{
    switch (type_) {
        case LOOPBACK_TYPE_NORMAL:
            return EnableNormal(enable) == SUCCESS;
        case LOOPBACK_TYPE_GLOBAL_HANDLER:
            return EnableHandler(enable) == SUCCESS;
        case LOOPBACK_TYPE_GLOBAL_CONTROL:
            return EnableControl(enable) == SUCCESS;
        default:
            AUDIO_ERR_LOG("unknown type %{public}d", type_);
            return false;
    }
}

int32_t AudioLoopbackWrapper::EnableNormal(bool enable)
{
    if (loopbackProxy_ != nullptr) {
        int result = SUCCESS;
        loopbackProxy_->Enable(enable, result);
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("ILoopback.Enable failed %{public}d", result);
            return result;
        }
    }

    if (private_ != nullptr) {
        return private_->Enable(enable) ? SUCCESS : ERR_UNKNOWN;
    }
    return SUCCESS;
}

int32_t AudioLoopbackWrapper::EnableHandler(bool enable)
{
    CHECK_AND_RETURN_RET_LOG(loopbackProxy_ != nullptr, ERR_INIT_FAILED, "no proxy");
    if (private_ != nullptr) {
        return private_->Enable(enable) ? SUCCESS : ERR_UNKNOWN;
    }
    return SUCCESS;
}

int32_t AudioLoopbackWrapper::EnableControl(bool enable)
{
    if (loopbackProxy_ == nullptr) {
        AUDIO_ERR_LOG("ILoopback proxy is nullptr");
        return ERROR_LOOPBACK_HANDLER_NOT_EXIST;
    }

    int result = SUCCESS;
    loopbackProxy_->Enable(enable, result);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "failed with %{public}d", result);

    // wait server notify(timeout 500ms)
    std::unique_lock<std::mutex> waitLock(waitMutex_);
    auto ret = waitCv_.wait_for(waitLock, std::chrono::milliseconds(WAIT_SERVER_RESULT_TIMEOUT_IN_MS));
    CHECK_AND_RETURN_RET_LOG(ret != std::cv_status::timeout, ERR_OPERATION_FAILED, "Client wait server timed out");

    waitLock.unlock();
    result = notifyStatus_ == LOOPBACK_AVAILABLE_RUNNING ? SUCCESS : ERR_OPERATION_FAILED;
    return result;
}

AudioLoopbackStatus AudioLoopbackWrapper::GetStatus()
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        if (loopbackProxy_ == nullptr) {
            AUDIO_ERR_LOG("ILoopback proxy is nullptr");
            return LOOPBACK_UNAVAILABLE_SCENE;
        }
        int status = 0;
        int result = SUCCESS;
        loopbackProxy_->GetStatus(status, result);
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("ILoopback.GetStatus failed %{public}d", result);
            return LOOPBACK_UNAVAILABLE_SCENE;
        }
        std::unique_lock<std::mutex> waitLock(waitMutex_);
        auto ret = waitCv_.wait_for(waitLock, std::chrono::milliseconds(WAIT_SERVER_STATUS_TIMEOUT_IN_MS));
        CHECK_AND_RETURN_RET_LOG(ret != std::cv_status::timeout, LOOPBACK_UNAVAILABLE_SCENE,
            "Client wait server timed out");
        waitLock.unlock();
        return notifyStatus_;
    }

    if (private_ != nullptr) {
        return private_->GetStatus();
    }
    return LOOPBACK_UNAVAILABLE_SCENE;
}

int32_t AudioLoopbackWrapper::SetVolume(float volume)
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        AUDIO_ERR_LOG("SetVolume not supported for CONTROL type");
        return ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED;
    }

    if (private_ != nullptr) {
        return private_->SetVolume(volume);
    }
    return ERR_NULL_POINTER;
}

float AudioLoopbackWrapper::GetVolume()
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        if (loopbackProxy_ == nullptr) {
            AUDIO_ERR_LOG("ILoopback proxy is nullptr");
            return 0.0f;
        }
        float volume = 0.0f;
        int result = SUCCESS;
        loopbackProxy_->GetVolume(volume, result);
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("ILoopback.GetVolume failed %{public}d", result);
            return 0.0f;
        }
        return volume;
    }

    if (private_ != nullptr) {
        return private_->GetVolume();
    }
    return 0.0f;
}

int32_t AudioLoopbackWrapper::SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    userCallback_ = callback;

    if (private_ != nullptr) {
        return private_->SetAudioLoopbackCallback(shared_from_this());
    }
    return SUCCESS;
}

int32_t AudioLoopbackWrapper::RemoveAudioLoopbackCallback()
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    userCallback_ = nullptr;

    if (private_ != nullptr) {
        return private_->RemoveAudioLoopbackCallback();
    }
    return SUCCESS;
}

bool AudioLoopbackWrapper::SetReverbPreset(AudioLoopbackReverbPreset preset)
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        AUDIO_ERR_LOG("SetReverbPreset not supported for CONTROL type");
        return false;
    }

    if (private_ != nullptr) {
        return private_->SetReverbPreset(preset);
    }
    return false;
}

AudioLoopbackReverbPreset AudioLoopbackWrapper::GetReverbPreset()
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        AUDIO_ERR_LOG("GetReverbPreset not supported for CONTROL type");
        return REVERB_PRESET_ORIGINAL;
    }

    if (private_ != nullptr) {
        return private_->GetReverbPreset();
    }
    return REVERB_PRESET_ORIGINAL;
}

bool AudioLoopbackWrapper::SetEqualizerPreset(AudioLoopbackEqualizerPreset preset)
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        AUDIO_ERR_LOG("SetEqualizerPreset not supported for CONTROL type");
        return false;
    }

    if (private_ != nullptr) {
        return private_->SetEqualizerPreset(preset);
    }
    return false;
}

AudioLoopbackEqualizerPreset AudioLoopbackWrapper::GetEqualizerPreset()
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        AUDIO_ERR_LOG("GetEqualizerPreset not supported for CONTROL type");
        return EQUALIZER_PRESET_FLAT;
    }

    if (private_ != nullptr) {
        return private_->GetEqualizerPreset();
    }
    return EQUALIZER_PRESET_FLAT;
}

std::vector<AudioDevicePair> AudioLoopbackWrapper::GetSupportedDevicePairs()
{
    return AudioPolicyManager::GetInstance().GetLoopbackSupportedDevicePairs();
}

AudioDevicePair AudioLoopbackWrapper::GetPreferredDevicePair()
{
    return AudioPolicyManager::GetInstance().GetLoopbackPreferredDevicePair();
}

void AudioLoopbackWrapper::OnStatusChange(const AudioLoopbackStatus state, const StateChangeCmdType cmdType)
{
    if (type_ == LOOPBACK_TYPE_GLOBAL_HANDLER) {
        CHECK_AND_RETURN_LOG(loopbackProxy_ != nullptr, "no proxy");
        int32_t ret = 0;
        loopbackProxy_->NotifyStatus(state, ret);
    }
    std::unique_lock<std::mutex> lock(callbackMutex_);
    CHECK_AND_RETURN_LOG(userCallback_ != nullptr, "no client callback");
    std::shared_ptr<AudioLoopbackCallback> temp = userCallback_;
    lock.unlock();
    temp->OnStatusChange(static_cast<AudioLoopbackStatus>(state), CMD_FROM_SYSTEM);
}

// called from AudioLoopbackManager
void AudioLoopbackWrapper::OnCommandResult(int command, bool success, int errorCode)
{
    AUDIO_DEBUG_LOG("server called: command %{public}d, success %{public}d, error %{public}d",
        command, success, errorCode);
    CHECK_AND_RETURN_LOG(type_ == LOOPBACK_TYPE_GLOBAL_HANDLER, "not handler!");
    if (static_cast<LoopbackCommand>(command) == LOOPBACK_CMD_GETSTATUS) {
        CHECK_AND_RETURN_LOG(private_ != nullptr && loopbackProxy_ != nullptr, "no proxy");
        AudioLoopbackStatus status = private_->GetStatus();
        loopbackProxy_->NotifyStatus(status, errorCode);
        return;
    }
    bool isEnable = static_cast<LoopbackCommand>(command) == LOOPBACK_CMD_ENABLE ? true : false;
    bool result = false;
    AudioLoopbackStatus status = LOOPBACK_AVAILABLE_IDLE;
    if (private_ != nullptr) {
        result = private_->Enable(isEnable);
        status = private_->GetStatus();
    }
    if (isEnable && result) {
        return;
    }
    CHECK_AND_RETURN_LOG(loopbackProxy_ != nullptr, "no proxy");
    int32_t ret = 0;
    loopbackProxy_->NotifyStatus(status, ret); // notify to all controls when failed
}

// called from AudioLoopbackManager
void AudioLoopbackWrapper::OnStatusNotify(int status, int reason)
{
    AUDIO_INFO_LOG("status %{public}d, reason %{public}d", status, reason);

    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (userCallback_ != nullptr && type_ == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        notifyStatus_ = static_cast<AudioLoopbackStatus>(status);
        waitCv_.notify_all();
        userCallback_->OnStatusChange(static_cast<AudioLoopbackStatus>(status), CMD_FROM_SYSTEM);
    }
}

} // namespace AudioStandard
} // namespace OHOS