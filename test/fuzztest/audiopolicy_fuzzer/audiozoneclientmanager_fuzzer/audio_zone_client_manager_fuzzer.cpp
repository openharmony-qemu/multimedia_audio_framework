/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_log.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_zone_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
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

void AudioZoneClientManagerGetInstanceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    audioZoneClientManager.GetInstance();
}

void AudioZoneClientManagerRegisterAudioZoneClientFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    sptr<IStandardAudioZoneClient> client = nullptr;
    audioZoneClientManager.RegisterAudioZoneClient(clientPid, client);
}

void AudioZoneClientManagerUnRegisterAudioZoneClientFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    audioZoneClientManager.UnRegisterAudioZoneClient(clientPid);
}

void AudioZoneClientManagerDispatchEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    std::shared_ptr<AudioZoneEvent> event;
    audioZoneClientManager.DispatchEvent(event);
}

void AudioZoneClientManagerSendZoneAddEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    std::shared_ptr<AudioZoneDescriptor> descriptor;
    audioZoneClientManager.SendZoneAddEvent(clientPid, descriptor);
}

void AudioZoneClientManagerSendZoneRemoveEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    audioZoneClientManager.SendZoneRemoveEvent(clientPid, zoneId);
}

void AudioZoneClientManagerSendZoneChangeEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<AudioZoneChangeReason> audioZoneChangeReason {
        AudioZoneChangeReason::UNKNOWN,
        AudioZoneChangeReason::BIND_NEW_DEVICE,
        AudioZoneChangeReason::BIND_NEW_APP,
        AudioZoneChangeReason::UNBIND_APP
    };
    AudioZoneChangeReason reason =
        static_cast<AudioZoneChangeReason>(GetData<int32_t>() % audioZoneChangeReason.size());
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    std::shared_ptr<AudioZoneDescriptor> descriptor;
    audioZoneClientManager.SendZoneChangeEvent(clientPid, descriptor, reason);
}

void AudioZoneClientManagerSendZoneInterruptEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<AudioZoneInterruptReason> audioZoneInterruptReason {
        AudioZoneInterruptReason::UNKNOWN,
        AudioZoneInterruptReason::LOCAL_INTERRUPT,
        AudioZoneInterruptReason::REMOTE_INJECT,
        AudioZoneInterruptReason::RELEASE_AUDIO_ZONE,
        AudioZoneInterruptReason::BIND_APP_TO_ZONE,
        AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE,
    };
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = "test";
    AudioZoneInterruptReason reason =
        static_cast<AudioZoneInterruptReason>(GetData<int32_t>() % audioZoneInterruptReason.size());
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    audioZoneClientManager.SendZoneInterruptEvent(clientPid, zoneId, deviceTag, interrupts, reason);
}

void AudioZoneClientManagerSetSystemVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t volumeFlag = GetData<int32_t>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    audioZoneClientManager.SetSystemVolumeLevel(clientPid, zoneId, volumeType, volumeLevel, volumeFlag);
}

void AudioZoneClientManagerGetSystemVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    audioZoneClientManager.GetSystemVolumeLevel(clientPid, zoneId, volumeType);
}

void AudioZoneClientManagerSetSystemVolumeDegreeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    AudioVolumeType volumeType  = GetData<AudioVolumeType>();
    int32_t volumeDegree = GetData<int32_t>();
    int32_t volumeFlag  = GetData<int32_t>();
    audioZoneClientManager.SetSystemVolumeDegree(clientPid, zoneId, volumeType, volumeDegree, volumeFlag);
}

void AudioZoneClientManagerGetSystemVolumeDegreeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler;
    AudioZoneClientManager audioZoneClientManager(handler);
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    AudioVolumeType volumeType  = GetData<AudioVolumeType>();
    audioZoneClientManager.GetSystemVolumeDegree(clientPid, zoneId, volumeType);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioZoneClientManagerGetInstanceFuzzTest,
    AudioZoneClientManagerRegisterAudioZoneClientFuzzTest,
    AudioZoneClientManagerUnRegisterAudioZoneClientFuzzTest,
    AudioZoneClientManagerDispatchEventFuzzTest,
    AudioZoneClientManagerSendZoneAddEventFuzzTest,
    AudioZoneClientManagerSendZoneRemoveEventFuzzTest,
    AudioZoneClientManagerSendZoneChangeEventFuzzTest,
    AudioZoneClientManagerSendZoneInterruptEventFuzzTest,
    AudioZoneClientManagerSetSystemVolumeLevelFuzzTest,
    AudioZoneClientManagerGetSystemVolumeLevelFuzzTest,
    AudioZoneClientManagerSetSystemVolumeDegreeFuzzTest,
    AudioZoneClientManagerGetSystemVolumeDegreeFuzzTest,
    });
    func(fdp);
}
void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}