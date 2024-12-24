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
const uint64_t CAPSESSION_ID = 123456;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioEffectServiceFuzzTest()
{
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::shared_ptr<IAudioInterruptEventDispatcher> dispatcher = nullptr;
    audioPolicyServerHandler->Init(dispatcher);

    int32_t clientPid = GetData<int32_t>();
    sptr<IAudioPolicyClient> cb = nullptr;
    audioPolicyServerHandler->AddAudioPolicyClientProxyMap(clientPid, cb);

    pid_t removeClientPid = GetData<pid_t>();
    audioPolicyServerHandler->RemoveAudioPolicyClientProxyMap(removeClientPid);

    int32_t clientId = GetData<int32_t>();
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback = nullptr;
    audioPolicyServerHandler->AddExternInterruptCbsMap(clientId, audioInterruptCallback);
    audioPolicyServerHandler->RemoveExternInterruptCbsMap(clientId);

    AudioDeviceUsage usage = GetData<AudioDeviceUsage>();
    sptr<IStandardAudioPolicyManagerListener> audioPolicyManagerListener = nullptr;
    audioPolicyServerHandler->AddAvailableDeviceChangeMap(clientId, usage, audioPolicyManagerListener);
    audioPolicyServerHandler->RemoveAvailableDeviceChangeMap(clientId, usage);

    sptr<IStandardAudioRoutingManagerListener> audioRoutingManagerListener = nullptr;
    audioPolicyServerHandler->AddDistributedRoutingRoleChangeCbsMap(clientId, audioRoutingManagerListener);
    audioPolicyServerHandler->RemoveDistributedRoutingRoleChangeCbsMap(clientId);
}

void AudioSendCallbackFuzzTest()
{
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    audioPolicyServerHandler->SendDeviceChangedCallback(desc, true);
    audioPolicyServerHandler->SendDeviceChangedCallback(desc, false);

    DeviceBlockStatus status = GetData<DeviceBlockStatus>();
    audioPolicyServerHandler->SendMicrophoneBlockedCallback(desc, status);

    audioPolicyServerHandler->SendAvailableDeviceChange(desc, true);
    audioPolicyServerHandler->SendAvailableDeviceChange(desc, false);

    VolumeEvent volumeEvent;
    audioPolicyServerHandler->SendVolumeKeyEventCallback(volumeEvent);
 
    std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair;
    audioPolicyServerHandler->SendAudioSessionDeactiveCallback(sessionDeactivePair);

    int32_t callbackCategory = GetData<int32_t>();
    AudioInterrupt audioInterrupt;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    audioPolicyServerHandler->SendAudioFocusInfoChangeCallback(callbackCategory, audioInterrupt, focusInfoList);

    AudioRingerMode ringMode = GetData<AudioRingerMode>();
    audioPolicyServerHandler->SendRingerModeUpdatedCallback(ringMode);

    MicStateChangeEvent micStateChangeEvent;
    int32_t clientId = GetData<int32_t>();
    audioPolicyServerHandler->SendMicStateUpdatedCallback(micStateChangeEvent);
    audioPolicyServerHandler->SendMicStateWithClientIdCallback(micStateChangeEvent, clientId);

    InterruptEventInternal interruptEvent;
    uint32_t sessionId = GetData<uint32_t>();
    audioPolicyServerHandler->SendInterruptEventInternalCallback(interruptEvent);
    audioPolicyServerHandler->SendInterruptEventWithSessionIdCallback(interruptEvent, sessionId);
    audioPolicyServerHandler->SendInterruptEventWithClientIdCallback(interruptEvent, clientId);
}

void AudioPolicyServSendFuzzTest()
{
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();

    audioPolicyServerHandler->SendPreferredOutputDeviceUpdated();
    audioPolicyServerHandler->SendPreferredInputDeviceUpdated();

    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    CastType type = GetData<CastType>();
    audioPolicyServerHandler->SendDistributedRoutingRoleChange(descriptor, type);

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    audioPolicyServerHandler->SendRendererInfoEvent(audioRendererChangeInfos);
 
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioPolicyServerHandler->SendCapturerInfoEvent(audioCapturerChangeInfos);

    int32_t clientPid = GetData<int32_t>();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t streamFlag = GetData<int32_t>();
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioPolicyServerHandler->SendRendererDeviceChangeEvent(clientPid, sessionId, outputDeviceInfo, reason);
    audioPolicyServerHandler->SendRecreateRendererStreamEvent(clientPid, sessionId, streamFlag, reason);
    audioPolicyServerHandler->SendRecreateCapturerStreamEvent(clientPid, sessionId, streamFlag, reason);
    audioPolicyServerHandler->SendConcurrencyEventWithSessionIDCallback(sessionId);
 
    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint64_t sendCapturerSessionId = CAPSESSION_ID;
    int32_t error = GetData<int32_t>();
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

    AudioPipeType pipeType = GetData<AudioPipeType>();
    audioPolicyServerHandler->SendPipeStreamCleanEvent(pipeType);
}

void AudioPolicyServHandleFuzzTest()
{
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();

    CallbackChange callbackchange = GetData<CallbackChange>();
    audioPolicyServerHandler->SetClientCallbacksEnable(callbackchange, true);
    audioPolicyServerHandler->SetClientCallbacksEnable(callbackchange, false);
}

typedef void (*TestFuncs[4])();

TestFuncs g_testFuncs = {
    AudioEffectServiceFuzzTest,
    AudioSendCallbackFuzzTest,
    AudioPolicyServSendFuzzTest,
    AudioPolicyServHandleFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}