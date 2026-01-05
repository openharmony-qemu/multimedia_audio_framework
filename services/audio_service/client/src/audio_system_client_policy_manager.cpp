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


#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_system_client_policy_manager.h"
#include "audio_policy_manager.h"
#include "app_bundle_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string BUNDLE_NAME_SCENE_BOARD = "com.ohos.sceneboard";

class AudioDistributedRoutingRoleCallbackImpl : public AudioDistributedRoutingRoleCallback {
public:
    explicit AudioDistributedRoutingRoleCallbackImpl()
    {
        AUDIO_INFO_LOG("AudioDistributedRoutingRoleCallbackImpl constructor");
    }
    virtual ~AudioDistributedRoutingRoleCallbackImpl()
    {
        AUDIO_INFO_LOG("AudioDistributedRoutingRoleCallbackImpl destroy");
    }

    /**
     * Called when audio device descriptor change.
     *
     * @param descriptor Indicates the descriptor needed by client.
     * For details, refer AudioDeviceDescriptor in audio_system_manager.h
     * @since 9
     */
    void OnDistributedRoutingRoleChange(std::shared_ptr<AudioDeviceDescriptor>descriptor,
        const CastType type) override
    {
        std::vector<std::shared_ptr<AudioDistributedRoutingRoleCallback>> temp_;
        std::unique_lock<std::mutex> cbListLock(cbListMutex_);
        for (auto callback = callbackList_.begin(); callback != callbackList_.end(); ++callback) {
            callback_ = (*callback);
            if (callback_ != nullptr) {
                AUDIO_DEBUG_LOG("OnDistributedRoutingRoleChange : Notify event to app complete");
                temp_.push_back(callback_);
            } else {
                AUDIO_ERR_LOG("OnDistributedRoutingRoleChange: callback is null");
            }
        }
        cbListLock.unlock();
        for (uint32_t i = 0; i < temp_.size(); i++) {
            temp_[i]->OnDistributedRoutingRoleChange(descriptor, type);
        }
        return;
    }

    void SaveCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
    {
        bool hasCallback = false;
        std::lock_guard<std::mutex> cbListLock(cbListMutex_);
        for (auto it = callbackList_.begin(); it != callbackList_.end(); ++it) {
            if ((*it) == callback) {
                hasCallback = true;
            }
        }
        if (!hasCallback) {
            callbackList_.push_back(callback);
        }
    }

    void RemoveCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
    {
        AUDIO_INFO_LOG("Entered %{public}s", __func__);
        std::lock_guard<std::mutex> cbListLock(cbListMutex_);
        callbackList_.remove_if([&callback](std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback_) {
            return callback_ == callback;
        });
    }

private:
    std::list<std::shared_ptr<AudioDistributedRoutingRoleCallback>> callbackList_;
    std::shared_ptr<AudioDistributedRoutingRoleCallback> callback_;
    std::mutex cbListMutex_;
};

int32_t AudioSystemClientPolicyManager::ConfigDistributedRoutingRole(
    std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    if (descriptor == nullptr) {
        AUDIO_ERR_LOG("ConfigDistributedRoutingRole: invalid parameter");
        return ERR_INVALID_PARAM;
    }
    AUDIO_INFO_LOG(" Entered ConfigDistributedRoutingRole casttype %{public}d", type);
    if (descriptor->deviceRole_ != DeviceRole::OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("ConfigDistributedRoutingRole: not an output device");
        return ERR_INVALID_PARAM;
    }

    int32_t ret = AudioPolicyManager::GetInstance().ConfigDistributedRoutingRole(descriptor, type);
    return ret;
}

int32_t AudioSystemClientPolicyManager::SetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetDistributedRoutingRoleCallback: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    if (audioDistributedRoutingRoleCallback_ == nullptr) {
        audioDistributedRoutingRoleCallback_ = std::make_shared<AudioDistributedRoutingRoleCallbackImpl>();
        if (audioDistributedRoutingRoleCallback_ == nullptr) {
            AUDIO_ERR_LOG("AudioSystemManger failed to allocate memory for distributedRoutingRole callback");
            return ERROR;
        }
        int32_t ret = AudioPolicyManager::GetInstance().
            SetDistributedRoutingRoleCallback(audioDistributedRoutingRoleCallback_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("AudioSystemManger failed to set distributedRoutingRole callback");
            return ERROR;
        }
    }

    std::shared_ptr<AudioDistributedRoutingRoleCallbackImpl> cbImpl =
        std::static_pointer_cast<AudioDistributedRoutingRoleCallbackImpl>(audioDistributedRoutingRoleCallback_);
    if (cbImpl == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManger cbImpl is nullptr");
        return ERROR;
    }
    cbImpl->SaveCallback(callback);
    return SUCCESS;
}

int32_t AudioSystemClientPolicyManager::UnsetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    int32_t ret = AudioPolicyManager::GetInstance().UnsetDistributedRoutingRoleCallback();
    if (audioDistributedRoutingRoleCallback_ != nullptr) {
        audioDistributedRoutingRoleCallback_.reset();
        audioDistributedRoutingRoleCallback_ = nullptr;
    }

    std::shared_ptr<AudioDistributedRoutingRoleCallbackImpl> cbImpl =
        std::static_pointer_cast<AudioDistributedRoutingRoleCallbackImpl>(audioDistributedRoutingRoleCallback_);
    if (cbImpl == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManger cbImpl is nullptr");
        return ERROR;
    }
    cbImpl->RemoveCallback(callback);
    return ret;
}

AudioSystemClientPolicyManager &AudioSystemClientPolicyManager::GetInstance()
{
    static AudioSystemClientPolicyManager instance;
    return instance;
}

AudioSystemClientPolicyManager::~AudioSystemClientPolicyManager()
{
    AUDIO_DEBUG_LOG("~AudioSystemClientPolicyManager");
    if (cbClientId_ != -1) {
        UnsetRingerModeCallback(cbClientId_);
        cbClientId_ = -1;
    }
}

int32_t AudioSystemClientPolicyManager::SetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().SetMicrophoneBlockedCallback(clientId, callback);
}

int32_t AudioSystemClientPolicyManager::UnsetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().UnsetMicrophoneBlockedCallback(clientId, callback);
}

int32_t AudioSystemClientPolicyManager::SetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().SetAudioSceneChangeCallback(clientId, callback);
}

int32_t AudioSystemClientPolicyManager::UnsetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().UnsetAudioSceneChangeCallback(callback);
}

int32_t AudioSystemClientPolicyManager::SetQueryClientTypeCallback(
    const std::shared_ptr<AudioQueryClientTypeCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryClientTypeCallback(callback);
}

int32_t AudioSystemClientPolicyManager::SetAudioClientInfoMgrCallback(
    const std::shared_ptr<AudioClientInfoMgrCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAudioClientInfoMgrCallback(callback);
}

int32_t AudioSystemClientPolicyManager::SetAudioVKBInfoMgrCallback(
    const std::shared_ptr<AudioVKBInfoMgrCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAudioVKBInfoMgrCallback(callback);
}

int32_t AudioSystemClientPolicyManager::CheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    AUDIO_INFO_LOG("In");
    return AudioPolicyManager::GetInstance().CheckVKBInfo(bundleName, isValid);
}

int32_t AudioSystemClientPolicyManager::SetQueryBundleNameListCallback(
    const std::shared_ptr<AudioQueryBundleNameListCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryBundleNameListCallback(callback);
}

int32_t AudioSystemClientPolicyManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    return AudioPolicyManager::GetInstance().SetSystemSoundUri(key, uri);
}

std::string AudioSystemClientPolicyManager::GetSystemSoundUri(const std::string &key)
{
    return AudioPolicyManager::GetInstance().GetSystemSoundUri(key);
}

std::string AudioSystemClientPolicyManager::GetSystemSoundPath(int32_t systemSoundType)
{
    std::string systemSoundPath = AudioPolicyManager::GetInstance().GetSystemSoundPath(systemSoundType);
    AUDIO_INFO_LOG("systemSoundType: %{public}d, systemSoundType: %{public}s",
        systemSoundType, systemSoundPath.c_str());
    return systemSoundPath;
}

int32_t AudioSystemClientPolicyManager::SetAppSilentOnDisplay(int32_t displayId)
{
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED,
        "SetAppSilentOnDisplay: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "SetAppSilentOnDisplay: No system permission");
    return AudioPolicyManager::GetInstance().SetAppSilentOnDisplay(displayId);
}

int32_t AudioSystemClientPolicyManager::ActivatePreemptMode() const
{
    AUDIO_DEBUG_LOG("ActivatePreemptMode");
    return AudioPolicyManager::GetInstance().ActivatePreemptMode();
}

int32_t AudioSystemClientPolicyManager::DeactivatePreemptMode() const
{
    AUDIO_DEBUG_LOG("DeactivatePreemptMode");
    return AudioPolicyManager::GetInstance().DeactivatePreemptMode();
}

int32_t AudioSystemClientPolicyManager::UpdateStreamState(int32_t clientUid,
    StreamSetState streamSetState, StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("clientUid:%{public}d streamSetState:%{public}d streamUsage:%{public}d",
        clientUid, streamSetState, streamUsage);
    return AudioPolicyManager::GetInstance().UpdateStreamState(clientUid, streamSetState, streamUsage);
}

int32_t AudioSystemClientPolicyManager::SetSleVoiceStatusFlag(bool isSleVoiceStatus)
{
    AUDIO_INFO_LOG("isSleVoiceStatus: %{public}d", isSleVoiceStatus);
    return AudioPolicyManager::GetInstance().SetSleVoiceStatusFlag(isSleVoiceStatus);
}

int32_t AudioSystemClientPolicyManager::SetAvailableDeviceChangeCallback(AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().SetAvailableDeviceChangeCallback(clientId, usage, callback);
}

int32_t AudioSystemClientPolicyManager::UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage)
{
    int32_t clientId = getpid();
    return AudioPolicyManager::GetInstance().UnsetAvailableDeviceChangeCallback(clientId, usage);
}

int32_t AudioSystemClientPolicyManager::SetCallDeviceActive(DeviceType deviceType, bool flag,
    std::string &address, int32_t clientUid) const
{
    int32_t uid = clientUid == -1 ? getuid() : clientUid;
    return AudioPolicyManager::GetInstance().SetCallDeviceActive(static_cast<InternalDeviceType>(deviceType),
        flag, address, uid);
}

int32_t AudioSystemClientPolicyManager::DisableSafeMediaVolume()
{
    return AudioPolicyManager::GetInstance().DisableSafeMediaVolume();
}

int32_t AudioSystemClientPolicyManager::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    return AudioPolicyManager::GetInstance().LoadSplitModule(splitArgs, networkId);
}

int32_t AudioSystemClientPolicyManager::SetVirtualCall(bool isVirtual)
{
    return AudioPolicyManager::GetInstance().SetVirtualCall(isVirtual);
}

bool AudioSystemClientPolicyManager::GetVirtualCall()
{
    return AudioPolicyManager::GetInstance().GetVirtualCall();
}

int32_t AudioSystemClientPolicyManager::SetQueryAllowedPlaybackCallback(
    const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryAllowedPlaybackCallback(callback);
}

int32_t AudioSystemClientPolicyManager::SetBackgroundMuteCallback(
    const std::shared_ptr<AudioBackgroundMuteCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetBackgroundMuteCallback(callback);
}

int32_t AudioSystemClientPolicyManager::NotifySessionStateChange(int32_t uid, int32_t pid, bool hasSession)
{
    AUDIO_INFO_LOG("Set uid:%{public}d, pid:%{public}d, Session state:%{public}d", uid, pid, hasSession);
    return AudioPolicyManager::GetInstance().NotifySessionStateChange(uid, pid, hasSession);
}

int32_t AudioSystemClientPolicyManager::NotifyFreezeStateChange(const std::set<int32_t> &pidList, bool isFreeze)
{
    return AudioPolicyManager::GetInstance().NotifyFreezeStateChange(pidList, isFreeze);
}

int32_t AudioSystemClientPolicyManager::ResetAllProxy()
{
    AUDIO_INFO_LOG("RSS IN");
    return AudioPolicyManager::GetInstance().ResetAllProxy();
}

int32_t AudioSystemClientPolicyManager::NotifyProcessBackgroundState(int32_t uid, int32_t pid)
{
    AUDIO_INFO_LOG("RSS IN");
    return AudioPolicyManager::GetInstance().NotifyProcessBackgroundState(uid, pid);
}

int32_t AudioSystemClientPolicyManager::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    AUDIO_INFO_LOG("volumeType:%{public}d, duration:%{public}d", volumeType, duration);
    return AudioPolicyManager::GetInstance().ForceVolumeKeyControlType(volumeType, duration);
}

void AudioSystemClientPolicyManager::CleanUpResource()
{
    AudioPolicyManager::GetInstance().CleanUpResource();
}

int32_t AudioSystemClientPolicyManager::SetRingerModeCallback(const int32_t clientId,
    const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    std::lock_guard<std::mutex> lockSet(ringerModeCallbackMutex_);
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    cbClientId_ = clientId;
    ringerModeCallback_ = callback;

    return SUCCESS;
}

int32_t AudioSystemClientPolicyManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    CHECK_AND_RETURN_RET(clientId == cbClientId_, ERR_INVALID_OPERATION);

    return SUCCESS;
}

int32_t AudioSystemClientPolicyManager::SetRingerMode(AudioRingerMode ringMode)
{
    // Deprecated. Please use the SetRingerMode interface of AudioGroupManager.
    AUDIO_WARNING_LOG("Use the deprecated SetRingerMode func. ringer mode [%{public}d]", ringMode);
    std::lock_guard<std::mutex> lockSet(ringerModeCallbackMutex_);
    ringModeBackup_ = ringMode;
    if (ringerModeCallback_ != nullptr) {
        ringerModeCallback_->OnRingerModeUpdated(ringModeBackup_);
    }

    return SUCCESS;
}

AudioRingerMode AudioSystemClientPolicyManager::GetRingerMode()
{
    return ringModeBackup_;
}

int32_t AudioSystemClientPolicyManager::SetAudioScene(const AudioScene &scene)
{
    AUDIO_DEBUG_LOG("audioScene_=%{public}d done", scene);
    return AudioPolicyManager::GetInstance().SetAudioScene(scene);
}

AudioScene AudioSystemClientPolicyManager::GetAudioScene() const
{
    auto audioScene = AudioPolicyManager::GetInstance().GetAudioScene();
    std::string bundleName = AppBundleManager::GetBundleNameFromUid(static_cast<int32_t>(getuid()));
    if (bundleName == BUNDLE_NAME_SCENE_BOARD) {
        audioScene = AudioPolicyManager::GetInstance().GetAudioSceneFromAllZones();
    }
    AUDIO_DEBUG_LOG("origin audioScene: %{public}d", audioScene);
    switch (audioScene) {
        case AUDIO_SCENE_CALL_START:
        case AUDIO_SCENE_CALL_END:
            return AUDIO_SCENE_DEFAULT;

        case AUDIO_SCENE_VOICE_RINGING:
            return AUDIO_SCENE_RINGING;

        default:
            return audioScene;
    }
}
} // namespace AudioStandard
} // namespace OHOS
