/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_server_handler.h"
#include "audio_device_info.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const uint64_t CAPSESSION_ID = 123456;

void AudioEffectServiceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::shared_ptr<IAudioInterruptEventDispatcher> dispatcher = nullptr;
    audioPolicyServerHandler->Init(dispatcher);

    int32_t clientPid = *reinterpret_cast<const int32_t*>(rawData);
    sptr<IAudioPolicyClient> cb = nullptr;
    audioPolicyServerHandler->AddAudioPolicyClientProxyMap(clientPid, cb);

    pid_t removeClientPid = *reinterpret_cast<const pid_t*>(rawData);
    audioPolicyServerHandler->RemoveAudioPolicyClientProxyMap(removeClientPid);

    int32_t clientId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback = nullptr;
    audioPolicyServerHandler->AddExternInterruptCbsMap(clientId, audioInterruptCallback);
    audioPolicyServerHandler->RemoveExternInterruptCbsMap(clientId);

    AudioDeviceUsage usage = D_ALL_DEVICES;
    sptr<IStandardAudioPolicyManagerListener> audioPolicyManagerListener = nullptr;
    audioPolicyServerHandler->AddAvailableDeviceChangeMap(clientId, usage, audioPolicyManagerListener);
    audioPolicyServerHandler->RemoveAvailableDeviceChangeMap(clientId, usage);

    sptr<IStandardAudioRoutingManagerListener> audioRoutingManagerListener = nullptr;
    audioPolicyServerHandler->AddDistributedRoutingRoleChangeCbsMap(clientId, audioRoutingManagerListener);
    audioPolicyServerHandler->RemoveDistributedRoutingRoleChangeCbsMap(clientId);
}

void AudioSendCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    audioPolicyServerHandler->SendDeviceChangedCallback(desc, true);
    audioPolicyServerHandler->SendDeviceChangedCallback(desc, false);

    DeviceBlockStatus status = *reinterpret_cast<const DeviceBlockStatus*>(rawData);
    audioPolicyServerHandler->SendMicrophoneBlockedCallback(desc, status);

    audioPolicyServerHandler->SendAvailableDeviceChange(desc, true);
    audioPolicyServerHandler->SendAvailableDeviceChange(desc, false);

    VolumeEvent volumeEvent;
    audioPolicyServerHandler->SendVolumeKeyEventCallback(volumeEvent);

    std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair;
    audioPolicyServerHandler->SendAudioSessionDeactiveCallback(sessionDeactivePair);

    int32_t callbackCategory = *reinterpret_cast<const int32_t*>(rawData);
    AudioInterrupt audioInterrupt;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    audioPolicyServerHandler->SendAudioFocusInfoChangeCallback(callbackCategory, audioInterrupt, focusInfoList);

    AudioRingerMode ringMode = RINGER_MODE_NORMAL;
    audioPolicyServerHandler->SendRingerModeUpdatedCallback(ringMode);

    MicStateChangeEvent micStateChangeEvent;
    int32_t clientId = *reinterpret_cast<const int32_t*>(rawData);
    audioPolicyServerHandler->SendMicStateUpdatedCallback(micStateChangeEvent);
    audioPolicyServerHandler->SendMicStateWithClientIdCallback(micStateChangeEvent, clientId);

    InterruptEventInternal interruptEvent;
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    audioPolicyServerHandler->SendInterruptEventInternalCallback(interruptEvent);
    audioPolicyServerHandler->SendInterruptEventWithSessionIdCallback(interruptEvent, sessionId);
    audioPolicyServerHandler->SendInterruptEventWithClientIdCallback(interruptEvent, clientId);
}

void AudioPolicyServSendFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();

    audioPolicyServerHandler->SendPreferredOutputDeviceUpdated();
    audioPolicyServerHandler->SendPreferredInputDeviceUpdated();

    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    CastType type = CAST_TYPE_ALL;
    audioPolicyServerHandler->SendDistributedRoutingRoleChange(descriptor, type);

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    audioPolicyServerHandler->SendRendererInfoEvent(audioRendererChangeInfos);

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioPolicyServerHandler->SendCapturerInfoEvent(audioCapturerChangeInfos);

    int32_t clientPid = *reinterpret_cast<const int32_t*>(rawData);
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    int32_t streamFlag = *reinterpret_cast<const int32_t*>(rawData);
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioPolicyServerHandler->SendRendererDeviceChangeEvent(clientPid, sessionId, outputDeviceInfo, reason);
    audioPolicyServerHandler->SendRecreateRendererStreamEvent(clientPid, sessionId, streamFlag, reason);
    audioPolicyServerHandler->SendRecreateCapturerStreamEvent(clientPid, sessionId, streamFlag, reason);
    audioPolicyServerHandler->SendConcurrencyEventWithSessionIDCallback(sessionId);

    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint64_t sendCapturerSessionId = CAPSESSION_ID;
    int32_t error = *reinterpret_cast<const int32_t*>(rawData);
    audioPolicyServerHandler->SendCapturerCreateEvent(capturerInfo, streamInfo, sendCapturerSessionId, true, error);
    audioPolicyServerHandler->SendCapturerCreateEvent(capturerInfo, streamInfo, sendCapturerSessionId, false, error);
    audioPolicyServerHandler->SendCapturerRemovedEvent(sendCapturerSessionId, true);
    audioPolicyServerHandler->SendCapturerRemovedEvent(sendCapturerSessionId, false);
    audioPolicyServerHandler->SendWakeupCloseEvent(true);
    audioPolicyServerHandler->SendWakeupCloseEvent(false);

    std::unordered_map<std::string, bool> changeInfo;
    audioPolicyServerHandler->SendHeadTrackingDeviceChangeEvent(changeInfo);
    audioPolicyServerHandler->SendSpatializatonEnabledChangeEvent(true);
    audioPolicyServerHandler->SendSpatializatonEnabledChangeEvent(false);

    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice;
    audioPolicyServerHandler->SendSpatializatonEnabledChangeForAnyDeviceEvent(selectedAudioDevice, true);
    audioPolicyServerHandler->SendSpatializatonEnabledChangeForAnyDeviceEvent(selectedAudioDevice, false);
    audioPolicyServerHandler->SendHeadTrackingEnabledChangeEvent(true);
    audioPolicyServerHandler->SendHeadTrackingEnabledChangeEvent(false);
    audioPolicyServerHandler->SendHeadTrackingEnabledChangeForAnyDeviceEvent(selectedAudioDevice, true);
    audioPolicyServerHandler->SendHeadTrackingEnabledChangeForAnyDeviceEvent(selectedAudioDevice, false);

    AudioPipeType pipeType = PIPE_TYPE_MULTICHANNEL;
    audioPolicyServerHandler->SendPipeStreamCleanEvent(pipeType);
}

void AudioPolicyServHandleFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();

    CallbackChange callbackchange = CALLBACK_FOCUS_INFO_CHANGE;
    audioPolicyServerHandler->SetClientCallbacksEnable(callbackchange, true);
    audioPolicyServerHandler->SetClientCallbacksEnable(callbackchange, false);
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioEffectServiceFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioSendCallbackFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServSendFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServHandleFuzzTest(rawData, size);
    return 0;
}