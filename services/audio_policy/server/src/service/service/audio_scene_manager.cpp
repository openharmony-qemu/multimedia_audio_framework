
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
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {

static const int64_t MEDIA_TO_RING_MUTE_DURATION_TIME_US = 200000; // 200ms
static const int64_t HEADSET_SWITCH_DELAY_US = 100000; //100ms

void AudioSceneManager::SetAudioScenePre(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    lastAudioScene_ = audioScene_;
    audioScene_ = audioScene;
    Bluetooth::AudioHfpManager::SetAudioSceneFromPolicy(audioScene_);
    if (lastAudioScene_ != AUDIO_SCENE_DEFAULT && audioScene_ == AUDIO_SCENE_DEFAULT) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), CLEAR_UID, "SetAudioScenePre");
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_CAPTURE,
            std::make_shared<AudioDeviceDescriptor>());
#ifdef BLUETOOTH_ENABLE
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

int32_t AudioSceneManager::SetAudioSceneAfter(AudioScene audioScene, BluetoothOffloadState state)
{
    std::vector<DeviceType> activeOutputDevices;
    bool haveArmUsbDevice = false;
    DealAudioSceneOutputDevices(audioScene, activeOutputDevices, haveArmUsbDevice);
    // mute primary when play media and ring
    if (activeOutputDevices.size() > 1 && streamCollector_.IsMediaPlaying()) {
        audioIOHandleMap_.MuteSinkPort(PRIMARY_SPEAKER, MEDIA_TO_RING_MUTE_DURATION_TIME_US, true);
        // Wait for the audio data in the cache to be drained before moving the stream
        // Increase the delay time for the headset device
        DeviceType mainDeviceType = activeOutputDevices.front();
        if (mainDeviceType == DEVICE_TYPE_USB_HEADSET || mainDeviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            usleep(HEADSET_SWITCH_DELAY_US); // sleep fix data cache pop.
        }
    }
    int32_t result = SUCCESS;
    if (AudioPolicyUtils::GetInstance().GetScoExcluded()) {
        return result;
    }
    if (haveArmUsbDevice) {
        result = AudioServerProxy::GetInstance().SetAudioSceneProxy(audioScene, activeOutputDevices,
            DEVICE_TYPE_USB_ARM_HEADSET, state);
    } else {
        result = AudioServerProxy::GetInstance().SetAudioSceneProxy(audioScene, activeOutputDevices,
            audioActiveDevice_.GetCurrentInputDeviceType(), state);
    }
    return result;
}

void AudioSceneManager::DealAudioSceneOutputDevices(const AudioScene &audioScene,
    std::vector<DeviceType> &activeOutputDevices, bool &haveArmUsbDevice)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs {};
    switch (audioScene) {
        case AUDIO_SCENE_RINGING:
            descs = audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_RINGTONE, -1);
            if (!descs.empty()) {
                audioActiveDevice_.SetCurrentInputDeviceType(descs.front()->getType());
            }
            break;
        case AUDIO_SCENE_VOICE_RINGING:
            descs = audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_RINGTONE, -1);
            if (!descs.empty()) {
                audioActiveDevice_.SetCurrentInputDeviceType(descs.front()->getType());
            }
            break;
        default:
            AUDIO_INFO_LOG("No ringing scene:%{public}d", audioScene);
            break;
    }

    if (!descs.empty()) {
        for (size_t i = 0; i < descs.size(); i++) {
            if (descs[i]->getType() == DEVICE_TYPE_USB_ARM_HEADSET) {
                AUDIO_INFO_LOG("usb headset is arm device.");
                activeOutputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
                haveArmUsbDevice = true;
            } else {
                activeOutputDevices.push_back(descs[i]->getType());
            }
        }
    } else {
        DeviceType activeDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        if (activeDeviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            activeOutputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
            haveArmUsbDevice = true;
        } else {
            activeOutputDevices.push_back(audioActiveDevice_.GetCurrentOutputDeviceType());
        }
    }
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
}
}