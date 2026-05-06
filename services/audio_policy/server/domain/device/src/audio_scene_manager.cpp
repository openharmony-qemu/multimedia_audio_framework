
/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSceneManager"
#endif

#include "audio_scene_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_router_select_strategy.h"

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif
#include "audio_active_device.h"
#include "sle_audio_device_manager.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {

constexpr int32_t ANCO_SERVICE_BROKER_UID = 5557;

void AudioSceneManager::SetAudioScenePre(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    AudioScene oldScene;
    AudioScene newScene;
    {
        std::lock_guard<std::mutex> lock(sceneMutex_);
        HILOG_COMM_INFO("[SetAudioScenePre]Set audio scene start %{public}d, lastScene %{public}d",
            audioScene, audioScene_);
        oldScene = audioScene_;
        lastAudioScene_ = audioScene_;
        audioScene_ = audioScene;
        newScene = audioScene;
    }
    if (oldScene != AUDIO_SCENE_DEFAULT && newScene == AUDIO_SCENE_DEFAULT) {
        if (oldScene != AUDIO_SCENE_RINGING || !VolumeUtils::IsPCVolumeEnable()) {
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                std::make_shared<AudioDeviceDescriptor>(), CLEAR_UID, "SetAudioScenePre");
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE,
                std::make_shared<AudioDeviceDescriptor>());
            std::vector<shared_ptr<AudioDeviceDescriptor>> unexcludedDevices =
                AudioRouterSelectStrategy::GetInstance().GetExcludedDevices(CALL_OUTPUT_DEVICES);
            AudioRouterSelectStrategy::GetInstance().UnexcludeDevices(unexcludedDevices, CALL_OUTPUT_DEVICES);
        }
#ifdef BLUETOOTH_ENABLE
        Bluetooth::AudioHfpManager::UpdateAudioScene(newScene);
        Bluetooth::AudioHfpManager::DisconnectSco();
        AudioRouterSelectStrategy::GetInstance().SetScoExcluded(false);
#endif
    }
    if (newScene == AUDIO_SCENE_DEFAULT) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState();
    }
}

bool AudioSceneManager::IsStreamActive(AudioStreamType streamType) const
{
    CHECK_AND_RETURN_RET(streamType != STREAM_VOICE_CALL ||
        GetAudioScene(true) != AUDIO_SCENE_PHONE_CALL, true);

    return streamCollector_.IsStreamActive(streamType);
}

bool AudioSceneManager::IsStreamActiveByStreamUsage(StreamUsage streamUsage) const
{
    CHECK_AND_RETURN_RET(streamUsage != STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
        GetAudioScene(true) != AUDIO_SCENE_PHONE_CALL, true);
    return streamCollector_.IsStreamActiveByStreamUsage(streamUsage);
}

bool AudioSceneManager::CheckVoiceCallActive(int32_t sessionId) const
{
    return streamCollector_.CheckVoiceCallActive(sessionId);
}

int32_t AudioSceneManager::SetAudioSceneAfter(AudioScene audioScene, BluetoothOffloadState state)
{
    return AudioServerProxy::GetInstance().SetAudioSceneProxy(audioScene, state);
}

AudioScene AudioSceneManager::GetAudioScene(bool hasSystemPermission) const
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    AUDIO_DEBUG_LOG("GetAudioScene return value: %{public}d", audioScene_);
    if (!hasSystemPermission) {
        switch (audioScene_) {
            case AUDIO_SCENE_CALL_START:
            case AUDIO_SCENE_CALL_END:
                return AUDIO_SCENE_DEFAULT;
            default:
                break;
        }
    }
    return audioScene_;
}

AudioScene AudioSceneManager::GetLastAudioScene() const
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return lastAudioScene_;
}

bool AudioSceneManager::IsSameAudioScene()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return lastAudioScene_ == audioScene_;
}

bool AudioSceneManager::IsVoiceCallRelatedScene()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return audioScene_ == AUDIO_SCENE_RINGING ||
        audioScene_ == AUDIO_SCENE_PHONE_CALL ||
        audioScene_ == AUDIO_SCENE_PHONE_CHAT ||
        audioScene_ == AUDIO_SCENE_VOICE_RINGING;
}

bool AudioSceneManager::IsInPhoneCallScene()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return audioScene_ == AUDIO_SCENE_PHONE_CALL;
}

bool AudioSceneManager::IsHangUpScene()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return (lastAudioScene_ == AUDIO_SCENE_PHONE_CALL || lastAudioScene_ == AUDIO_SCENE_PHONE_CHAT) &&
        audioScene_ == AUDIO_SCENE_DEFAULT;
}

bool AudioSceneManager::IsCallEnded()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return (lastAudioScene_ == AUDIO_SCENE_PHONE_CALL && audioScene_ != AUDIO_SCENE_PHONE_CALL) ||
        (lastAudioScene_ == AUDIO_SCENE_PHONE_CHAT && audioScene_ != AUDIO_SCENE_PHONE_CHAT);
}

bool AudioSceneManager::IsPhoneCallOrChatSceneTriggeredByUid(int32_t uid) const
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return (audioScene_ == AUDIO_SCENE_PHONE_CALL || audioScene_ == AUDIO_SCENE_PHONE_CHAT) &&
        (phoneCallOrChatSceneTriggerUid_ == uid);
}

void AudioSceneManager::SetPhoneCallOrChatSceneTriggerUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    if (audioScene_ != AUDIO_SCENE_PHONE_CALL && audioScene_ != AUDIO_SCENE_PHONE_CHAT) {
        return;
    }
    phoneCallOrChatSceneTriggerUid_ = uid;
}

int32_t AudioSceneManager::GetAudioSceneOwnerUid()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return callOwnerUid_;
}

void AudioSceneManager::SetAudioSceneOwnerUid(const int32_t uid)
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    AUDIO_INFO_LOG("callOwnerUid_: %{public}d, uid: %{public}d", callOwnerUid_, uid);
    callOwnerUid_ = uid;
    if (uid == AUDIO_ID) {
        callOwnerUid_ = ANCO_SERVICE_BROKER_UID;
    }
}

bool AudioSceneManager::IsInAudioCallScene()
{
    std::lock_guard<std::mutex> lock(sceneMutex_);
    return callOwnerUid_ != 0;
}
}
}
