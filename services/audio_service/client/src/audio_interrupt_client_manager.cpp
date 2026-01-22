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

#include "audio_interrupt_client_manager.h"
#include "audio_log.h"
#include "audio_policy_manager.h"
#include "audio_focus_info_change_callback_impl.h"
#include "audio_utils.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

class AudioManagerInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioManagerInterruptCallbackImpl()
    {
        AUDIO_INFO_LOG("AudioManagerInterruptCallbackImpl constructor");
    }
    virtual ~AudioManagerInterruptCallbackImpl()
    {
        AUDIO_DEBUG_LOG("AudioManagerInterruptCallbackImpl: instance destroy");
    }

    void OnInterrupt(const InterruptEventInternal &interruptEvent) override
    {
        cb_ = callback_.lock();
        if (cb_ != nullptr) {
            cb_->cbMutex_.lock();
            InterruptAction interruptAction = {};
            interruptAction.actionType = (interruptEvent.eventType == INTERRUPT_TYPE_BEGIN)
                ? TYPE_INTERRUPT : TYPE_ACTIVATED;
            interruptAction.interruptType = interruptEvent.eventType;
            interruptAction.interruptHint = interruptEvent.hintType;
            interruptAction.activated = (interruptEvent.eventType == INTERRUPT_TYPE_BEGIN) ? false : true;
            cb_->OnInterrupt(interruptAction);
            AUDIO_DEBUG_LOG("Notify event to app complete");
            cb_->cbMutex_.unlock();
        } else {
            AUDIO_ERR_LOG("callback is null");
        }

        return;
    }
    void SaveCallback(const std::weak_ptr<AudioManagerCallback> &callback)
    {
        auto wp = callback.lock();
        if (wp != nullptr) {
            callback_ = callback;
        } else {
            AUDIO_ERR_LOG("callback is nullptr");
        }
    }

private:
    std::weak_ptr<AudioManagerCallback> callback_;
    std::shared_ptr<AudioManagerCallback> cb_;
};

AudioInterruptClientManager &AudioInterruptClientManager::GetInstance()
{
    static AudioInterruptClientManager instance;
    return instance;
}

int32_t AudioInterruptClientManager::GetAudioFocusInfoList(
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().GetAudioFocusInfoList(focusInfoList);
}

int32_t AudioInterruptClientManager::RegisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    int32_t clientId = getpid();
    AUDIO_DEBUG_LOG("RegisterFocusInfoChangeCallback clientId:%{public}d", clientId);
    if (audioFocusInfoCallback_ == nullptr) {
        audioFocusInfoCallback_ = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(audioFocusInfoCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioInterruptCallback");
        int32_t ret = AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId,
            audioFocusInfoCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed set callback");
    }

    std::shared_ptr<AudioFocusInfoChangeCallbackImpl> cbFocusInfo =
        std::static_pointer_cast<AudioFocusInfoChangeCallbackImpl>(audioFocusInfoCallback_);
    CHECK_AND_RETURN_RET_LOG(cbFocusInfo != nullptr, ERROR, "cbFocusInfo is nullptr");
    cbFocusInfo->SaveCallback(callback);

    return SUCCESS;
}

int32_t AudioInterruptClientManager::UnregisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    int32_t clientId = getpid();
    int32_t ret = 0;

    if (callback == nullptr) {
        ret = AudioPolicyManager::GetInstance().UnregisterFocusInfoChangeCallback(clientId);
        audioFocusInfoCallback_.reset();
        audioFocusInfoCallback_ = nullptr;
        if (!ret) {
            AUDIO_DEBUG_LOG("AudioInterruptClientManager::UnregisterVolumeKeyEventCallback success");
        }
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(audioFocusInfoCallback_ != nullptr, ERROR,
        "Failed to allocate memory for audioInterruptCallback");
    std::shared_ptr<AudioFocusInfoChangeCallbackImpl> cbFocusInfo =
        std::static_pointer_cast<AudioFocusInfoChangeCallbackImpl>(audioFocusInfoCallback_);
    cbFocusInfo->RemoveCallback(callback);

    return ret;
}

int32_t AudioInterruptClientManager::ActivateAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    AUDIO_DEBUG_LOG("stub implementation");
    return AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
}

int32_t AudioInterruptClientManager::SetAppConcurrencyMode(const int32_t appUid, const int32_t mode)
{
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED,
        "SetAppConcurrencyMode: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "SetAppConcurrencyMode: No system permission");
    return AudioPolicyManager::GetInstance().SetAppConcurrencyMode(appUid, mode);
}

int32_t AudioInterruptClientManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const
{
    AUDIO_DEBUG_LOG("stub implementation");
    return AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
}

int32_t AudioInterruptClientManager::SetAudioInterruptCallback(const uint32_t sessionID,
    const std::shared_ptr<AudioInterruptCallback> &callback, uint32_t clientUid, const int32_t zoneID)
{
    return AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID, callback, clientUid, zoneID);
}

int32_t AudioInterruptClientManager::UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId)
{
    return AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(zoneId, sessionId);
}

int32_t AudioInterruptClientManager::SetAudioManagerInterruptCallback(
    const std::shared_ptr<AudioManagerCallback> &callback)
{
    int32_t clientId = getpid();
    AUDIO_INFO_LOG("client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    if (audioInterruptCallback_ != nullptr) {
        callback->cbMutex_.lock();
        AUDIO_DEBUG_LOG("reset existing callback object");
        AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
        audioInterruptCallback_.reset();
        audioInterruptCallback_ = nullptr;
        callback->cbMutex_.unlock();
    }

    audioInterruptCallback_ = std::make_shared<AudioManagerInterruptCallbackImpl>();
    CHECK_AND_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
        "Failed to allocate memory for audioInterruptCallback");

    int32_t ret =
        AudioPolicyManager::GetInstance().SetAudioManagerInterruptCallback(clientId, audioInterruptCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed set callback");

    std::shared_ptr<AudioManagerInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioManagerInterruptCallbackImpl>(audioInterruptCallback_);
    CHECK_AND_RETURN_RET_LOG(cbInterrupt != nullptr, ERROR, "cbInterrupt is nullptr");
    cbInterrupt->SaveCallback(callback);

    return SUCCESS;
}

int32_t AudioInterruptClientManager::UnsetAudioManagerInterruptCallback()
{
    int32_t clientId = getpid();
    AUDIO_INFO_LOG("client id: %{public}d", clientId);

    int32_t ret = AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
    if (audioInterruptCallback_ != nullptr) {
        audioInterruptCallback_.reset();
        audioInterruptCallback_ = nullptr;
    }

    return ret;
}

int32_t AudioInterruptClientManager::RequestAudioFocus(const AudioInterrupt &audioInterrupt)
{
    int32_t clientId = getpid();
    AUDIO_INFO_LOG("RequestAudioFocus client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.contentType >= CONTENT_TYPE_UNKNOWN &&
        audioInterrupt.contentType <= CONTENT_TYPE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid content type");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.streamUsage >= STREAM_USAGE_UNKNOWN &&
        audioInterrupt.streamUsage <= STREAM_USAGE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid stream usage");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.audioFocusType.streamType >= STREAM_VOICE_CALL &&
        audioInterrupt.audioFocusType.streamType <= STREAM_TYPE_MAX, ERR_INVALID_PARAM, "Invalid stream type");
    return AudioPolicyManager::GetInstance().RequestAudioFocus(clientId, audioInterrupt);
}

int32_t AudioInterruptClientManager::AbandonAudioFocus(const AudioInterrupt &audioInterrupt)
{
    int32_t clientId = getpid();
    AUDIO_INFO_LOG("AbandonAudioFocus client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.contentType >= CONTENT_TYPE_UNKNOWN &&
        audioInterrupt.contentType <= CONTENT_TYPE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid content type");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.streamUsage >= STREAM_USAGE_UNKNOWN &&
        audioInterrupt.streamUsage <= STREAM_USAGE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid stream usage");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.audioFocusType.streamType >= STREAM_VOICE_CALL &&
        audioInterrupt.audioFocusType.streamType <= STREAM_TYPE_MAX, ERR_INVALID_PARAM, "Invalid stream type");
    return AudioPolicyManager::GetInstance().AbandonAudioFocus(clientId, audioInterrupt);
}

bool AudioInterruptClientManager::RequestIndependentInterrupt(FocusType focusType)
{
    AUDIO_INFO_LOG("RequestIndependentInterrupt : foncusType");
    AudioInterrupt audioInterrupt;
    int32_t clientId = getpid();
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_SPEECH;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_RECORDING;
    audioInterrupt.streamId = static_cast<uint32_t>(clientId);
    int32_t result = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);

    AUDIO_DEBUG_LOG("Rresult -> %{public}d", result);
    return (result == SUCCESS) ? true:false;
}

bool AudioInterruptClientManager::AbandonIndependentInterrupt(FocusType focusType)
{
    AUDIO_INFO_LOG("AbandonIndependentInterrupt : foncusType");
    AudioInterrupt audioInterrupt;
    int32_t clientId = getpid();
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_SPEECH;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_RECORDING;
    audioInterrupt.streamId = static_cast<uint32_t>(clientId);
    int32_t result = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
    AUDIO_DEBUG_LOG("result -> %{public}d", result);
    return (result == SUCCESS) ? true:false;
}

int32_t AudioInterruptClientManager::InjectInterruption(const std::string networkId, InterruptEvent &event)
{
    return AudioPolicyManager::GetInstance().InjectInterruption(networkId, event);
}
} // namespace AudioStandard
} // namespace OHOS
