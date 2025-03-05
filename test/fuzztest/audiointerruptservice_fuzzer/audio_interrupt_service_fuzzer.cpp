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
#undef private
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_interrupt_service.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const uint8_t TESTSIZE = 18;
typedef void (*TestPtr)(const uint8_t *, size_t);

void InitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioPolicyServer> server = nullptr;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->Init(server);
}

void AddDumpInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::unordered_map<int32_t, std::shared_ptr<AudioInterruptZone>> audioInterruptZonesMapDump;

    interruptService->AddDumpInfo(audioInterruptZonesMapDump);
}

void SetCallbackHandlerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioPolicyServerHandler> handler = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->SetCallbackHandler(handler);
}

void SetAudioManagerInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    sptr<IRemoteObject> object = data.ReadRemoteObject();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->SetAudioManagerInterruptCallback(object);
}

void ActivateAudioInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);

    interruptService->ActivateAudioInterrupt(zoneId, audioInterrupt);
}

void DeactivateAudioInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);

    interruptService->DeactivateAudioInterrupt(zoneId, audioInterrupt);
}

void CreateAudioInterruptZoneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);

    interruptService->CreateAudioInterruptZone(zoneId);
}

void ReleaseAudioInterruptZoneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 0;
    };
    interruptService->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
}

void RemoveAudioInterruptZonePidsFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 0;
    };
    interruptService->MigrateAudioInterruptZone(zoneId, getZoneFunc);
}

void GetStreamInFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);

    interruptService->GetStreamInFocus(zoneId);
}

void GetSessionInfoInFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);

    interruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
}

void DispatchInterruptEventWithStreamIdFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(rawData);
    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = *reinterpret_cast<const InterruptType *>(rawData);
    interruptEvent.forceType = *reinterpret_cast<const InterruptForceType *>(rawData);
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    interruptEvent.duckVolume = 0;

    interruptService->DispatchInterruptEventWithStreamId(sessionId, interruptEvent);
}

void RequestAudioFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t clientId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);

    interruptService->RequestAudioFocus(clientId, audioInterrupt);
}

void AbandonAudioFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t clientId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);

    interruptService->AbandonAudioFocus(clientId, audioInterrupt);
}

void SetAudioInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(rawData);
    uint32_t uid = *reinterpret_cast<const uint32_t *>(rawData);

    interruptService->SetAudioInterruptCallback(zoneId, sessionId, object, uid);
}

void UnsetAudioInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(rawData);

    interruptService->UnsetAudioInterruptCallback(zoneId, sessionId);
}

void AddAudioInterruptZonePidsFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 0;
    };
    interruptService->MigrateAudioInterruptZone(zoneId, getZoneFunc);
}

void UpdateAudioSceneFromInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    AudioScene audioScene = *reinterpret_cast<const AudioScene *>(rawData);
    AudioInterruptChangeType changeType = *reinterpret_cast<const AudioInterruptChangeType *>(rawData);

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}
} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[OHOS::AudioStandard::TESTSIZE] = {
    OHOS::AudioStandard::InitFuzzTest,
    OHOS::AudioStandard::AddDumpInfoFuzzTest,
    OHOS::AudioStandard::SetCallbackHandlerFuzzTest,
    OHOS::AudioStandard::SetAudioManagerInterruptCallbackFuzzTest,
    OHOS::AudioStandard::ActivateAudioInterruptFuzzTest,
    OHOS::AudioStandard::DeactivateAudioInterruptFuzzTest,
    OHOS::AudioStandard::CreateAudioInterruptZoneFuzzTest,
    OHOS::AudioStandard::ReleaseAudioInterruptZoneFuzzTest,
    OHOS::AudioStandard::RemoveAudioInterruptZonePidsFuzzTest,
    OHOS::AudioStandard::GetStreamInFocusFuzzTest,
    OHOS::AudioStandard::GetSessionInfoInFocusFuzzTest,
    OHOS::AudioStandard::DispatchInterruptEventWithStreamIdFuzzTest,
    OHOS::AudioStandard::RequestAudioFocusFuzzTest,
    OHOS::AudioStandard::AbandonAudioFocusFuzzTest,
    OHOS::AudioStandard::SetAudioInterruptCallbackFuzzTest,
    OHOS::AudioStandard::UnsetAudioInterruptCallbackFuzzTest,
    OHOS::AudioStandard::AddAudioInterruptZonePidsFuzzTest,
    OHOS::AudioStandard::UpdateAudioSceneFromInterruptFuzzTest
};

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint8_t firstByte = *data % OHOS::AudioStandard::TESTSIZE;
    if (firstByte >= OHOS::AudioStandard::TESTSIZE) {
        return 0;
    }
    data = data + 1;
    size = size - 1;
    g_testPtrs[firstByte](data, size);
    return 0;
}