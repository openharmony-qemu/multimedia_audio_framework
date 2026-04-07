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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_manager.h"
#include "audio_policy_proxy.h"
#include "audio_errors.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_zone_client.h"
#include "audio_zone_info.h"
#include "audio_device_descriptor.h"
#include "audio_interrupt_info.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static int32_t NUM_2 = 2;
static int32_t NUM_5 = 5;
static int32_t NUM_10 = 10;
static int32_t NUM_20 = 20;
static int32_t NUM_50 = 50;
static size_t g_dataSize = 0;
static size_t g_pos;

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

void RegisterAudioZoneClientFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> object = nullptr;
    AudioPolicyManager::GetInstance().RegisterAudioZoneClient(object);
}

void CreateAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = fdp.ConsumeRandomLengthString(100);
    AudioZoneContext context;
    context.focusStrategy_ = static_cast<AudioZoneFocusStrategy>(GetData<uint32_t>() % NUM_2);
    context.backStrategy_ = static_cast<MediaBackStrategy>(GetData<uint32_t>() % NUM_2);
    AudioPolicyManager::GetInstance().CreateAudioZone(name, context);
}

void ReleaseAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    AudioPolicyManager::GetInstance().ReleaseAudioZone(zoneId);
}

void UpdateContextForAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    AudioZoneContext context;
    context.focusStrategy_ = static_cast<AudioZoneFocusStrategy>(GetData<int32_t>() % NUM_2);
    context.backStrategy_ = static_cast<MediaBackStrategy>(GetData<int32_t>() % NUM_2);
    AudioPolicyManager::GetInstance().UpdateContextForAudioZone(zoneId, context);
}

void GetAllAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetAllAudioZone();
}

void GetAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    AudioPolicyManager::GetInstance().GetAudioZone(zoneId);
}

void GetAudioZoneByNameFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = fdp.ConsumeRandomLengthString(100);
    AudioPolicyManager::GetInstance().GetAudioZoneByName(name);
}

void BindDeviceToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    uint32_t deviceCount = GetData<uint32_t>() % 5;
    for (uint32_t i = 0; i < deviceCount; i++) {
        auto device = std::make_shared<AudioDeviceDescriptor>();
        devices.push_back(device);
    }
    AudioPolicyManager::GetInstance().BindDeviceToAudioZone(zoneId, devices);
}

void UnBindDeviceToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    uint32_t deviceCount = GetData<uint32_t>() % 5;
    for (uint32_t i = 0; i < deviceCount; i++) {
        auto device = std::make_shared<AudioDeviceDescriptor>();
        devices.push_back(device);
    }
    AudioPolicyManager::GetInstance().UnBindDeviceToAudioZone(zoneId, devices);
}

void EnableAudioZoneReportFuzzTest(FuzzedDataProvider& fdp)
{
    bool enable = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().EnableAudioZoneReport(enable);
}

void EnableAudioZoneChangeReportFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    bool enable = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().EnableAudioZoneChangeReport(zoneId, enable);
}

void AddUidToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().AddUidToAudioZone(zoneId, uid);
}

void RemoveUidFromAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().RemoveUidFromAudioZone(zoneId, uid);
}

void AddUidUsagesToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    std::set<StreamUsage> usages;
    uint32_t usageCount = GetData<uint32_t>() % 10;
    for (uint32_t i = 0; i < usageCount; i++) {
        int32_t usageValue = GetData<int32_t>() % 50;
        if (usageValue > STREAM_USAGE_UNKNOWN && usageValue <= STREAM_USAGE_MAX) {
            usages.insert(static_cast<StreamUsage>(usageValue));
        }
    }
    AudioPolicyManager::GetInstance().AddUidUsagesToAudioZone(zoneId, uid, usages);
}

void RemoveUidUsagesFromAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    std::set<StreamUsage> usages;
    uint32_t usageCount = GetData<uint32_t>() % 10;
    for (uint32_t i = 0; i < usageCount; i++) {
        int32_t usageValue = GetData<int32_t>() % 50;
        if (usageValue > STREAM_USAGE_UNKNOWN && usageValue <= STREAM_USAGE_MAX) {
            usages.insert(static_cast<StreamUsage>(usageValue));
        }
    }
    AudioPolicyManager::GetInstance().RemoveUidUsagesFromAudioZone(zoneId, uid, usages);
}

void AddStreamToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    AudioZoneStream stream;
    stream.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_50);
    stream.sourceType = static_cast<SourceType>(GetData<int32_t>() % NUM_10);
    stream.isPlay = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().AddStreamToAudioZone(zoneId, stream);
}

void AddStreamsToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::vector<AudioZoneStream> streams;
    uint32_t streamCount = GetData<uint32_t>() % 5;
    for (uint32_t i = 0; i < streamCount; i++) {
        AudioZoneStream stream;
        stream.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_50);
        stream.sourceType = static_cast<SourceType>(GetData<int32_t>() % NUM_10);
        stream.isPlay = GetData<uint32_t>() % NUM_2;
        streams.push_back(stream);
    }
    AudioPolicyManager::GetInstance().AddStreamsToAudioZone(zoneId, streams);
}

void SetZoneDeviceVisibleFuzzTest(FuzzedDataProvider& fdp)
{
    bool visible = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().SetZoneDeviceVisible(visible);
}

void RemoveStreamFromAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    AudioZoneStream stream;
    stream.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_50);
    stream.sourceType = static_cast<SourceType>(GetData<int32_t>() % NUM_10);
    stream.isPlay = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().RemoveStreamFromAudioZone(zoneId, stream);
}

void RemoveStreamsFromAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::vector<AudioZoneStream> streams;
    uint32_t streamCount = GetData<uint32_t>() % 5;
    for (uint32_t i = 0; i < streamCount; i++) {
        AudioZoneStream stream;
        stream.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_50);
        stream.sourceType = static_cast<SourceType>(GetData<int32_t>() % NUM_10);
        stream.isPlay = GetData<uint32_t>() % NUM_2;
        streams.push_back(stream);
    }
    AudioPolicyManager::GetInstance().RemoveStreamsFromAudioZone(zoneId, streams);
}

void EnableSystemVolumeProxyFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    bool enable = GetData<uint32_t>() % NUM_2;
    DeviceType deviceType = GetData<DeviceType>();
    AudioPolicyManager::GetInstance().EnableSystemVolumeProxy(zoneId, deviceType, enable);
}

void GetAudioInterruptForZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    AudioPolicyManager::GetInstance().GetAudioInterruptForZone(zoneId);
}

void GetAudioInterruptForZoneWithDeviceTagFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = fdp.ConsumeRandomLengthString(50);
    AudioPolicyManager::GetInstance().GetAudioInterruptForZone(zoneId, deviceTag);
}

void EnableAudioZoneInterruptReportFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = fdp.ConsumeRandomLengthString(50);
    bool enable = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().EnableAudioZoneInterruptReport(zoneId, deviceTag, enable);
}

void InjectInterruptToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    uint32_t interruptCount = GetData<uint32_t>() % 5;
    for (uint32_t i = 0; i < interruptCount; i++) {
        AudioInterrupt interrupt;
        interrupt.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_50);
        interrupt.contentType = static_cast<ContentType>(GetData<int32_t>() % NUM_10);
        interrupt.audioFocusType.streamType = static_cast<AudioStreamType>(GetData<int32_t>() % NUM_20);
        AudioFocuState state = static_cast<AudioFocuState>(GetData<int32_t>() % NUM_10);
        interrupts.push_back({interrupt, state});
    }
    AudioPolicyManager::GetInstance().InjectInterruptToAudioZone(zoneId, interrupts);
}

void InjectInterruptToAudioZoneWithDeviceTagFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = fdp.ConsumeRandomLengthString(50);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    uint32_t interruptCount = GetData<uint32_t>() % 5;
    for (uint32_t i = 0; i < interruptCount; i++) {
        AudioInterrupt interrupt;
        interrupt.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_50);
        interrupt.contentType = static_cast<ContentType>(GetData<int32_t>() % NUM_10);
        interrupt.audioFocusType.streamType = static_cast<AudioStreamType>(GetData<int32_t>() % NUM_20);
        AudioFocuState state = static_cast<AudioFocuState>(GetData<int32_t>() % NUM_10);
        interrupts.push_back({interrupt, state});
    }
    AudioPolicyManager::GetInstance().InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        RegisterAudioZoneClientFuzzTest,
        CreateAudioZoneFuzzTest,
        ReleaseAudioZoneFuzzTest,
        UpdateContextForAudioZoneFuzzTest,
        GetAllAudioZoneFuzzTest,
        GetAudioZoneFuzzTest,
        GetAudioZoneByNameFuzzTest,
        BindDeviceToAudioZoneFuzzTest,
        UnBindDeviceToAudioZoneFuzzTest,
        EnableAudioZoneReportFuzzTest,
        EnableAudioZoneChangeReportFuzzTest,
        AddUidToAudioZoneFuzzTest,
        RemoveUidFromAudioZoneFuzzTest,
        AddUidUsagesToAudioZoneFuzzTest,
        RemoveUidUsagesFromAudioZoneFuzzTest,
        AddStreamToAudioZoneFuzzTest,
        AddStreamsToAudioZoneFuzzTest,
        SetZoneDeviceVisibleFuzzTest,
        RemoveStreamFromAudioZoneFuzzTest,
        RemoveStreamsFromAudioZoneFuzzTest,
        EnableSystemVolumeProxyFuzzTest,
        GetAudioInterruptForZoneFuzzTest,
        GetAudioInterruptForZoneWithDeviceTagFuzzTest,
        EnableAudioZoneInterruptReportFuzzTest,
        InjectInterruptToAudioZoneFuzzTest,
        InjectInterruptToAudioZoneWithDeviceTagFuzzTest,
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
} // namespace OHOS

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
