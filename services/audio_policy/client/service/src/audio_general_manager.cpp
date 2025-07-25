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
#define LOG_TAG "AudioGeneralManager"
#endif

#include "audio_general_manager.h"

#include <mutex>
#include <unistd.h>
#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_focus_info_change_callback_impl.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "istandard_audio_service.h"

namespace OHOS {
namespace AudioStandard {
constexpr unsigned int XCOLLIE_TIME_OUT_SECONDS = 10;
std::mutex g_asManagerProxyMutex;
sptr<IStandardAudioService> g_asManagerProxy = nullptr;

AudioGeneralManager *AudioGeneralManager::GetInstance()
{
    static AudioGeneralManager AudioGeneralManager;
    return &AudioGeneralManager;
}

AudioGeneralManager::AudioGeneralManager()
{
    AUDIO_DEBUG_LOG("AudioGeneralManager start");
}

AudioGeneralManager::~AudioGeneralManager()
{
    AUDIO_DEBUG_LOG("AudioGeneralManager end");
}

int32_t AudioGeneralManager::GetCallingPid()
{
    return getpid();
}

int32_t AudioGeneralManager::SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    return AudioPolicyManager::GetInstance().SetAudioDeviceRefinerCallback(callback);
}

int32_t AudioGeneralManager::SetAudioClientInfoMgrCallback(
    const std::shared_ptr<AudioClientInfoMgrCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    return AudioPolicyManager::GetInstance().SetAudioClientInfoMgrCallback(callback);
}

int32_t AudioGeneralManager::GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    desc = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);

    return SUCCESS;
}

int32_t AudioGeneralManager::UnsetAudioDeviceRefinerCallback()
{
    return AudioPolicyManager::GetInstance().UnsetAudioDeviceRefinerCallback();
}

int32_t AudioGeneralManager::SetDeviceVolumeBehavior(const std::string &networkId,
    DeviceType deviceType, VolumeBehavior volumeBehavior)
{
    AUDIO_INFO_LOG("networkId [%{public}s], deviceType [%{public}d], isReady [%{public}d], "\
        "isVolumeControlDisabled [%{public}d], databaseVolumeName [%{public}s]", networkId.c_str(), deviceType,
        volumeBehavior.isReady, volumeBehavior.isVolumeControlDisabled, volumeBehavior.databaseVolumeName.c_str());
    return AudioPolicyManager::GetInstance().SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

int32_t AudioGeneralManager::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    return AudioPolicyManager::GetInstance().TriggerFetchDevice(reason);
}

int32_t AudioGeneralManager::SetPreferredDevice(const PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    return AudioPolicyManager::GetInstance().SetPreferredDevice(preferredType, desc, uid);
}

int32_t AudioGeneralManager::SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    return AudioPolicyManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, callback);
}

int32_t AudioGeneralManager::RegisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    int32_t clientId = GetCallingPid();
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

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioGeneralManager::GetDevicesInner(DeviceFlag deviceFlag)
{
    return AudioPolicyManager::GetInstance().GetDevicesInner(deviceFlag);
}

int32_t AudioGeneralManager::SetDeviceChangeCallback(const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
}

int32_t AudioGeneralManager::SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback>& callback)
{
    AUDIO_INFO_LOG("Entered");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryClientTypeCallback(callback);
}

int32_t AudioGeneralManager::SetQueryDeviceVolumeBehaviorCallback(
    const std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallback> &callback)
{
    AUDIO_INFO_LOG("Entered");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryDeviceVolumeBehaviorCallback(callback);
}

const sptr<IStandardAudioService> AudioGeneralManager::GetAudioGeneralManagerProxy()
{
    AudioXCollie xcollieGetAudioSystemManagerProxy("GetAudioGeneralManagerProxy", XCOLLIE_TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::lock_guard<std::mutex> lock(g_asManagerProxyMutex);
    if (g_asManagerProxy == nullptr) {
        AudioXCollie xcollieGetSystemAbilityManager("GetSystemAbilityManager", XCOLLIE_TIME_OUT_SECONDS,
             nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get sa manager failed");
        xcollieGetSystemAbilityManager.CancelXCollieTimer();

        AudioXCollie xcollieGetSystemAbility("GetSystemAbility", XCOLLIE_TIME_OUT_SECONDS,
             nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio service remote object failed");
        g_asManagerProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_asManagerProxy != nullptr, nullptr, "get audio service proxy failed");
        xcollieGetSystemAbility.CancelXCollieTimer();
    }
    sptr<IStandardAudioService> gasp = g_asManagerProxy;
    return gasp;
}

int32_t AudioGeneralManager::SetExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    const sptr<IStandardAudioService> gasp = GetAudioGeneralManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    std::vector<StringPair> pairs;
    for (auto &pair : kvpairs) {
        pairs.push_back({pair.first, pair.second});
    }
    return gasp->SetExtraParameters(key, pairs);
}

int32_t AudioGeneralManager::GetVolume(AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_RING:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("volumeType[%{public}d] is not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    return AudioPolicyManager::GetInstance().GetSystemVolumeLevel(volumeType);
}

int32_t AudioGeneralManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("UnregisterVolumeKeyEventCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeKeyEventCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("UnsetVolumeKeyEventCallback success");
        volumeChangeClientPid_ = -1;
    }
    return ret;
}

DeviceType AudioGeneralManager::GetActiveOutputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveOutputDevice();
}

AudioScene AudioGeneralManager::GetAudioScene() const
{
    return AudioPolicyManager::GetInstance().GetAudioScene();
}

int32_t AudioGeneralManager::SetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().SetAudioSceneChangeCallback(clientId, callback);
}

int32_t AudioGeneralManager::GetMaxVolume(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_ALL) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    }

    if (volumeType == STREAM_ULTRASONIC) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    }

    return AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
}

int32_t AudioGeneralManager::UnregisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    int32_t clientId = GetCallingPid();
    int32_t ret = 0;

    if (callback == nullptr) {
        ret = AudioPolicyManager::GetInstance().UnregisterFocusInfoChangeCallback(clientId);
        audioFocusInfoCallback_.reset();
        audioFocusInfoCallback_ = nullptr;
        if (!ret) {
            AUDIO_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback success");
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

int32_t AudioGeneralManager::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().GetAudioFocusInfoList(focusInfoList);
}

int32_t AudioGeneralManager::SelectOutputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() == 1 && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_PARAM, "invalid parameter");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an output device.");
    size_t validSize = 64;
    if (audioDeviceDescriptors[0]->networkId_ != LOCAL_NETWORK_ID &&
        audioDeviceDescriptors[0]->networkId_.size() != validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid networkId.");
        return ERR_INVALID_PARAM;
    }
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "audioRendererFilter is nullptr.");
    audioRendererFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
    return ret;
}

int32_t AudioGeneralManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    AUDIO_DEBUG_LOG("AudioGeneralManager RegisterVolumeKeyEventCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterVolumeKeyEventCallbackcallback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, callback, api_v);
}

int32_t AudioGeneralManager::RegisterAudioCapturerEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("client id: %{public}d", clientPid);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientPid, callback);
}

int32_t AudioGeneralManager::GetCurrentRendererChangeInfos(
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioGeneralManager::RegisterAudioRendererEventListener(
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);

    return ret;
}

AudioFocusInfoChangeCallbackImpl::AudioFocusInfoChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioFocusInfoChangeCallbackImpl constructor");
}

AudioFocusInfoChangeCallbackImpl::~AudioFocusInfoChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioFocusInfoChangeCallbackImpl: destroy");
}

void AudioFocusInfoChangeCallbackImpl::SaveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    bool hasCallback = false;
    std::lock_guard<std::mutex> cbListLock(cbListMutex_);
    for (auto it = callbackList_.begin(); it != callbackList_.end(); ++it) {
        if ((*it).lock() == callback.lock()) {
            hasCallback = true;
        }
    }
    if (!hasCallback) {
        callbackList_.push_back(callback);
    }
}

void AudioFocusInfoChangeCallbackImpl::RemoveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    std::lock_guard<std::mutex> cbListLock(cbListMutex_);
    callbackList_.remove_if([&callback](std::weak_ptr<AudioFocusInfoChangeCallback> &callback_) {
        return callback_.lock() == callback.lock();
    });
}

int32_t AudioGeneralManager::GetPreferredInputDeviceForCapturerInfo(
    AudioCapturerInfo captureInfo, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    desc = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(captureInfo);
    return SUCCESS;
}
 
int32_t AudioGeneralManager::SetPreferredInputDeviceChangeCallback(
    AudioCapturerInfo &capturerInfo, const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
 
    return AudioPolicyManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
}
 
int32_t AudioGeneralManager::GetCurrentCapturerChangeInfos(
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

int32_t AudioGeneralManager::SetDeviceConnectionStatus(std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    bool isConnected)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "deviceDesc is nullptr");
    return AudioPolicyManager::GetInstance().SetDeviceConnectionStatus(deviceDesc, isConnected);
}

int32_t AudioGeneralManager::UpdateDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    const DeviceInfoUpdateCommand command)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "deviceDesc is nullptr");
    return AudioPolicyManager::GetInstance().UpdateDeviceInfo(deviceDesc, command);
}

int32_t AudioGeneralManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_INVALID_PARAM, "audioRendererFilter is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() == 1 && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_PARAM, "invalid parameter");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an output device.");
    size_t validSize = 64; // Size of remote network ID
    if (audioDeviceDescriptors[0]->networkId_ != LOCAL_NETWORK_ID &&
        audioDeviceDescriptors[0]->networkId_.size() != validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid networkId.");
        return ERR_INVALID_PARAM;
    }
    return AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
}

int32_t AudioGeneralManager::SetSleAudioOperationCallback(const std::shared_ptr<SleAudioOperationCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetSleAudioOperationCallback(callback);
}
} // namespace AudioStandard
} // namespace OHOS
