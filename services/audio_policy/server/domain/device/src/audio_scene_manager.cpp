
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

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif
#include "audio_active_device.h"
#include "sle_audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {

void AudioSceneManager::SetAudioScenePre(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    lastAudioScene_ = audioScene_;
    audioScene_ = audioScene;
    if (lastAudioScene_ != AUDIO_SCENE_DEFAULT && audioScene_ == AUDIO_SCENE_DEFAULT) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), CLEAR_UID, "SetAudioScenePre");
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE,
            std::make_shared<AudioDeviceDescriptor>());
#ifdef BLUETOOTH_ENABLE
        Bluetooth::AudioHfpManager::UpdateAudioScene(audioScene_);
        Bluetooth::AudioHfpManager::DisconnectSco();
        AudioPolicyUtils::GetInstance().SetScoExcluded(false);
#endif
    }
    if (audioScene_ == AUDIO_SCENE_DEFAULT) {
        AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState();
    }
}

bool AudioSceneManager::IsStreamActive(AudioStreamType streamType) const
{
    CHECK_AND_RETURN_RET(streamType != STREAM_VOICE_CALL ||
        GetAudioScene(true) != AUDIO_SCENE_PHONE_CALL, true);

    return streamCollector_.IsStreamActive(streamType);
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
    return lastAudioScene_;
}

bool AudioSceneManager::IsSameAudioScene()
{
    return lastAudioScene_ == audioScene_;
}

bool AudioSceneManager::IsVoiceCallRelatedScene()
{
    return audioScene_ == AUDIO_SCENE_RINGING ||
        audioScene_ == AUDIO_SCENE_PHONE_CALL ||
        audioScene_ == AUDIO_SCENE_PHONE_CHAT ||
        audioScene_ == AUDIO_SCENE_VOICE_RINGING;
}

bool AudioSceneManager::IsInPhoneCallScene()
{
    return audioScene_ == AUDIO_SCENE_PHONE_CALL;
}
}
}
