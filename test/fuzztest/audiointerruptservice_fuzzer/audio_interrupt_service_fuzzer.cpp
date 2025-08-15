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
const uint8_t TESTSIZE = 71;
const uint32_t TEST_ID_MODULO = 3;
constexpr uint32_t BOOL_MODULO = 2;
typedef void (*TestPtr)(const uint8_t *, size_t);

class AudioInterruptCallbackFuzzTest : public AudioInterruptCallback {
public:
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override {};
};

static void CreateAudioSessionService(shared_ptr<AudioInterruptService> &interruptService, bool isNull, int32_t id)
{
    if (interruptService == nullptr) {
        return;
    }
    interruptService->sessionService_ = std::make_shared<AudioSessionService>();
    if (isNull) {
        interruptService->sessionService_ = nullptr;
        return;
    }
    if (interruptService->sessionService_ == nullptr) {
        return;
    }
    AudioSessionStrategy strategy;
    interruptService->sessionService_->sessionMap_.insert(
        std::make_pair(id, std::make_shared<AudioSession>(id, strategy, make_shared<AudioSessionService>())));
}

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
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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

void AudioInterruptServiceActivateAudioSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    AudioSessionStrategy strategy;
    CreateAudioSessionService(interruptService, false, callerPid);

    interruptService->ActivateAudioSession(zoneId, callerPid, strategy);
}

void AudioInterruptServiceIsSessionNeedToFetchOutputDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    bool isNullptr = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    CreateAudioSessionService(interruptService, !isNullptr, callerPid);
    interruptService->IsSessionNeedToFetchOutputDevice(callerPid);
}

void AudioInterruptServiceSetAudioSessionSceneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    bool isNullptr = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    CreateAudioSessionService(interruptService, !isNullptr, callerPid);
    AudioSessionScene scene = AudioSessionScene::INVALID;
    interruptService->SetAudioSessionScene(callerPid, scene);
}

void AudioInterruptServiceAddActiveInterruptToSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    bool isNullptr = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    CreateAudioSessionService(interruptService, !isNullptr, callerPid);
    interruptService->AddActiveInterruptToSession(callerPid);
}

void AudioInterruptServiceDeactivateAudioSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    bool isNullptr = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    CreateAudioSessionService(interruptService, !isNullptr, callerPid);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, std::make_shared<AudioInterruptZone>()));
    interruptService->DeactivateAudioSession(zoneId, callerPid);
}

void AudioInterruptServiceRemovePlaceholderInterruptForSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    CreateAudioSessionService(interruptService, false, callerPid);
    bool isSessionTimeout = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    interruptService->RemovePlaceholderInterruptForSession(callerPid, isSessionTimeout);
}

void AudioInterruptServiceIsAudioSessionActivatedFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    bool isNullptr = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;

    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    CreateAudioSessionService(interruptService, !isNullptr, callerPid);
    interruptService->IsAudioSessionActivated(callerPid);
}

void AudioInterruptServiceIsCanMixInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    uint32_t testId = *reinterpret_cast<const uint32_t *>(rawData) % TEST_ID_MODULO;
    if (testId == 0) {
        incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
        activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    } else if (testId == 1) {
        incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
        incomingInterrupt.audioFocusType.streamType = STREAM_VOICE_COMMUNICATION;
        activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    } else {
        incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
        activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
        activeInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
        incomingInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
    }

    interruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
}

void AudioInterruptServiceCanMixForSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    activeInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
    incomingInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
}

void AudioInterruptServiceCanMixForIncomingSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    AudioFocusEntry focusEntry;
    CreateAudioSessionService(interruptService, false, incomingInterrupt.pid);
    interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
}

void AudioInterruptServiceIsIncomingStreamLowPriorityFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioFocusEntry focusEntry;
    uint32_t testId = *reinterpret_cast<const uint32_t *>(rawData) % TEST_ID_MODULO;
    if (testId == 0) {
        focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
        focusEntry.actionOn = INCOMING;
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
    } else if (testId == 1) {
        focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
        focusEntry.actionOn = INCOMING;
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
    } else {
        focusEntry.isReject = false;
        focusEntry.actionOn = BOTH;
    }
    interruptService->IsIncomingStreamLowPriority(focusEntry);
}

void AudioInterruptServiceIsActiveStreamLowPriorityFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioFocusEntry focusEntry;
    bool testFalse = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    if (testFalse) {
        focusEntry.actionOn = BOTH;
    } else {
        focusEntry.actionOn = CURRENT;
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
    }
    interruptService->IsActiveStreamLowPriority(focusEntry);
}

void AudioInterruptServiceUnsetAudioManagerInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    std::shared_ptr<AudioPolicyServerHandler> handler = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    interruptService->SetCallbackHandler(handler);
    interruptService->UnsetAudioManagerInterruptCallback();
}

void AudioInterruptServiceRequestAudioFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t clientId = *reinterpret_cast<const int32_t *>(rawData);
    bool isNotEqual = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    if (!isNotEqual) {
        interruptService->clientOnFocus_ = clientId;
    }
    interruptService->focussedAudioInterruptInfo_ = make_unique<AudioInterrupt>();
    AudioInterrupt audioInterrupt;

    interruptService->RequestAudioFocus(clientId, audioInterrupt);
}

void AudioInterruptServiceAbandonAudioFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t clientId = *reinterpret_cast<const int32_t *>(rawData);
    interruptService->clientOnFocus_ = clientId;
    AudioInterrupt audioInterrupt;
    interruptService->AbandonAudioFocus(clientId, audioInterrupt);
}

void AudioInterruptServiceUnsetAudioInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    uint32_t streamId = *reinterpret_cast<const uint32_t *>(rawData) + 1;
    std::shared_ptr<AudioInterruptService::AudioInterruptClient> interruptClient =
        std::make_shared<AudioInterruptService::AudioInterruptClient>(nullptr, nullptr, nullptr);
    interruptService->interruptClients_.insert({streamId, interruptClient});
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (zone == nullptr) {
        return;
    }
    zone->interruptCbsMap.insert(std::make_pair(streamId, make_shared<AudioInterruptCallbackFuzzTest>()));
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));
    interruptService->UnsetAudioInterruptCallback(zoneId, streamId);
}

void AudioInterruptServiceAudioInterruptIsActiveInFocusListFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    uint32_t incomingStreamId = *reinterpret_cast<const uint32_t *>(rawData) + 1;
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    interruptService->AudioInterruptIsActiveInFocusList(zoneId, incomingStreamId);
}

void AudioInterruptServiceHandleAppStreamTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    bool isNull = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    CreateAudioSessionService(interruptService, !isNull, audioInterrupt.pid);
    interruptService->HandleAppStreamType(0, audioInterrupt);
}

void AudioInterruptServiceActivateAudioInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    CreateAudioSessionService(interruptService, false, zoneId);
    bool isUpdatedAudioStrategy = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    interruptService->isPreemptMode_ = !((*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO);
    interruptService->ActivateAudioInterrupt(zoneId, audioInterrupt, isUpdatedAudioStrategy);
}

void AudioInterruptServicePrintLogsOfFocusStrategyBaseMusicFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    static const vector<AudioConcurrencyMode> concurrencyModes = {
        AudioConcurrencyMode::INVALID,
        AudioConcurrencyMode::DEFAULT,
        AudioConcurrencyMode::MIX_WITH_OTHERS,
        AudioConcurrencyMode::DUCK_OTHERS,
        AudioConcurrencyMode::PAUSE_OTHERS,
        AudioConcurrencyMode::SILENT,
    };

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr || concurrencyModes.empty()) {
        return;
    }

    AudioInterrupt audioInterrupt;
    CreateAudioSessionService(interruptService, true, 0);
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair = std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_STOP;
    focusEntry.actionOn = CURRENT;
    interruptService->focusCfgMap_.insert(std::make_pair(focusPair, focusEntry));
    uint32_t index = *reinterpret_cast<const uint32_t *>(rawData);
    audioInterrupt.sessionStrategy.concurrencyMode = concurrencyModes[index % concurrencyModes.size()];

    interruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
}

void AudioInterruptServiceClearAudioFocusInfoListFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    interruptService->ClearAudioFocusInfoList();
}

void AudioInterruptServiceActivatePreemptModeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    interruptService->ActivatePreemptMode();
    interruptService->DeactivatePreemptMode();
}

void AudioInterruptServiceInjectInterruptToAudioZoneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    AudioFocusList interrupts;
    interrupts.emplace_back(audioFocusInfo);
    interruptService->InjectInterruptToAudioZone(zoneId, interrupts);
}

void AudioInterruptServiceGetAudioFocusInfoListFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    std::string deviceTag = "testdevice";
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    AudioFocusList interrupts;
    interrupts.emplace_back(audioFocusInfo);
    interruptService->GetAudioFocusInfoList(zoneId, deviceTag, interrupts);
}

void AudioInterruptServiceGetStreamInFocusByUidFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t uid = *reinterpret_cast<const int32_t *>(rawData);
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));
    interruptService->GetStreamInFocusByUid(uid, zoneId);
}

void AudioInterruptServiceGetSessionInfoInFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t uid = *reinterpret_cast<const int32_t *>(rawData);
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));
    AudioInterrupt interrupt;
    interruptService->GetSessionInfoInFocus(interrupt, zoneId);
}

void AudioInterruptServiceIsSameAppInShareModeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t testId = *reinterpret_cast<const uint32_t *>(rawData) % TEST_ID_MODULO;
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    if (testId == 0) {
        incomingInterrupt.mode = INDEPENDENT_MODE;
        activeInterrupt.mode = INDEPENDENT_MODE;
    } else if (testId == 1) {
        incomingInterrupt.mode = SHARE_MODE;
        activeInterrupt.mode = SHARE_MODE;
        incomingInterrupt.pid = AudioInterruptService::DEFAULT_APP_PID;
        activeInterrupt.pid = AudioInterruptService::DEFAULT_APP_PID;
    } else {
        incomingInterrupt.mode = SHARE_MODE;
        activeInterrupt.mode = SHARE_MODE;
        incomingInterrupt.pid = AudioInterruptService::STREAM_DEFAULT_PRIORITY;
        activeInterrupt.pid = AudioInterruptService::STREAM_DEFAULT_PRIORITY;
    }

    interruptService->IsSameAppInShareMode(incomingInterrupt, activeInterrupt);
}

void AudioInterruptServiceCheckAudioSessionExistenceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.mode = INDEPENDENT_MODE;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    AudioFocusEntry focusEntry;
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    CreateAudioSessionService(interruptService, !focusEntry.isReject, incomingInterrupt.pid);

    interruptService->CheckAudioSessionExistence(incomingInterrupt, focusEntry);
}

void AudioInterruptServiceUpdateHintTypeForExistingSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t testId = *reinterpret_cast<const uint32_t *>(rawData) % TEST_ID_MODULO;
    AudioInterrupt incomingInterrupt;
    if (testId == 0) {
        incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::DUCK_OTHERS;
    } else if (testId == 1) {
        incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;
    } else {
        incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::INVALID;
    }
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_STOP;

    interruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
}

void AudioInterruptServiceProcessRemoteInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || audioInterruptZone == nullptr) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = *reinterpret_cast<const bool *>(rawData);
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    interruptService->zonesMap_.insert({pid, audioInterruptZone});
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    std::set<int32_t> sessionIds;
    interruptService->ProcessRemoteInterrupt(sessionIds, interruptEvent);
}

void AudioInterruptServiceProcessActiveInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || audioInterruptZone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = zoneId;
    audioInterrupt.isAudioSessionInterrupt = *reinterpret_cast<const bool *>(rawData);
    interruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    audioInterruptZone->audioFocusInfoList.push_back(
        {audioInterrupt, *reinterpret_cast<const AudioFocuState *>(rawData)});
    interruptService->policyServer_ = nullptr;
    interruptService->ProcessActiveInterrupt(zoneId, audioInterrupt);
}

void AudioInterruptServiceHandleLowPriorityEventFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    if (interruptService == nullptr || sessionService == nullptr) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t *>(rawData);
    int32_t streamId = *reinterpret_cast<const int32_t *>(rawData);
    sessionService->SetAudioSessionScene(pid, *reinterpret_cast<const AudioSessionScene *>(rawData));
    if (sessionService->sessionMap_[pid] == nullptr) {
        return;
    }
    sessionService->sessionMap_[pid]->audioSessionScene_ = *reinterpret_cast<const AudioSessionScene *>(rawData);
    sessionService->sessionMap_[pid]->state_ = *reinterpret_cast<const AudioSessionState *>(rawData);
    interruptService->sessionService_ = sessionService;
    interruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    bool isNull = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    if (isNull) {
        interruptService->sessionService_ = nullptr;
    }
    interruptService->HandleLowPriorityEvent(pid, streamId);
}

void AudioInterruptServiceSendActiveInterruptEventFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t streamId = *reinterpret_cast<const uint32_t *>(rawData);
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    interruptService->SendActiveInterruptEvent(streamId, interruptEvent, incomingInterrupt, activeInterrupt);
}

void AudioInterruptServiceAudioFocusInfoListRemovalConditionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = zoneId;
    audioInterrupt.isAudioSessionInterrupt = *reinterpret_cast<const bool *>(rawData);
    audioInterrupt.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(rawData);
    AudioFocuState audioFocusState = *reinterpret_cast<const AudioFocuState *>(rawData);
    std::pair<AudioInterrupt, AudioFocuState> audioInterruptPair = std::make_pair(audioInterrupt, audioFocusState);

    interruptService->AudioFocusInfoListRemovalCondition(audioInterrupt, audioInterruptPair);
}

void AudioInterruptServiceIsMediaStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioStreamType audioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);

    interruptService->IsMediaStream(audioStreamType);
}

void AudioInterruptServiceUpdateAudioFocusStrategyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt currentInterrupt;
    currentInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.uid = *reinterpret_cast<const int32_t *>(rawData);
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    incomingInterrupt.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(rawData);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = *reinterpret_cast<const ActionTarget *>(rawData);
    focusEntry.forceType = *reinterpret_cast<const InterruptForceType *>(rawData);
    focusEntry.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    interruptService->policyServer_ = nullptr;

    interruptService->UpdateAudioFocusStrategy(currentInterrupt, incomingInterrupt, focusEntry);
}

void AudioInterruptServiceIsMicSourceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    SourceType sourceType = *reinterpret_cast<const SourceType *>(rawData);

    interruptService->IsMicSource(sourceType);
}

void AudioInterruptServiceFocusEntryContinueFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.uid = AUDIO_ID;
    SourceType sourceType = *reinterpret_cast<const SourceType *>(rawData);
    audioInterrupt.currencySources.sourcesTypes.push_back(sourceType);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    list.push_back({audioInterrupt, ACTIVE});
    auto iterActive = list.begin();
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = *reinterpret_cast<const ActionTarget *>(rawData);
    focusEntry.forceType = *reinterpret_cast<const InterruptForceType *>(rawData);
    focusEntry.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    interruptService->FocusEntryContinue(iterActive, focusEntry, incomingInterrupt);
}

void AudioInterruptServiceProcessFocusEntryFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    interruptService->ProcessFocusEntry(zoneId, incomingInterrupt);
}

void GetHighestPriorityAudioSceneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    interruptService->GetHighestPriorityAudioScene(zoneId);
}
 
void GetStreamTypePriorityFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    interruptService->GetStreamTypePriority(streamType);
}
 
void DeactivatePreemptModeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->DeactivatePreemptMode();
}
 
void IsCapturerFocusAvailableFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
 
    uint32_t zoneId = *reinterpret_cast<const uint32_t *>(rawData);
    AudioCapturerInfo capturerInfo;
    interruptService->IsCapturerFocusAvailable(zoneId, capturerInfo);
}
 
void ClearAudioFocusBySessionIDFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
    interruptService->ClearAudioFocusBySessionID(sessionID);
}
 
void DeactivateAudioSessionInFakeFocusModeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t pid = *reinterpret_cast<const int32_t *>(rawData);
    InterruptHint hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    interruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);
}
 
void DeactivateAudioSessionFakeInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    int32_t callerPid = *reinterpret_cast<const int32_t *>(rawData);
    bool isSessionTimeout = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    interruptService->DeactivateAudioSessionFakeInterrupt(zoneId, callerPid, isSessionTimeout);
}
 
void AudioSessionInfoDumpFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    std::string dumpString = "";
    if (interruptService == nullptr) {
        return;
    }
    interruptService->AudioSessionInfoDump(dumpString);
}
 
void SetSessionMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(rawData);
    bool insert = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    bool muteFlag = (*reinterpret_cast<const uint32_t *>(rawData)) % BOOL_MODULO;
    if (interruptService == nullptr) {
        return;
    }
    interruptService->SetSessionMuteState(sessionId, insert, muteFlag);
}
 
void SetLatestMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = *reinterpret_cast<const InterruptType *>(rawData);
    interruptEvent.forceType = *reinterpret_cast<const InterruptForceType *>(rawData);
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    interruptEvent.duckVolume = 0;
    uint32_t streamId = *reinterpret_cast<const uint32_t *>(rawData);
    interruptService->SetLatestMuteState(interruptEvent, streamId);
}
 
void UpdateMuteAudioFocusStrategyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt currentInterrupt;
    AudioInterrupt incomingInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    interruptService->UpdateMuteAudioFocusStrategy(currentInterrupt, incomingInterrupt, focusEntry);
}
 
void ReportRecordGetFocusFailFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt activeInterrupt;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    activeInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    int32_t reason = *reinterpret_cast<const int32_t *>(rawData);
    interruptService->ReportRecordGetFocusFail(incomingInterrupt, activeInterrupt, reason);
}
 
void ProcessActiveStreamFocusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    incomingInterrupt.audioFocusType.isPlay = false;
    AudioFocuState incomingState = MUTED;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator activeInterrupt = zone->audioFocusInfoList.end();
    interruptService->ProcessActiveStreamFocus(zone->audioFocusInfoList, incomingInterrupt,
        incomingState, activeInterrupt);
}

void CanMixForActiveSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
 
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(rawData);
    AudioFocusEntry focusEntry;
    CreateAudioSessionService(interruptService, false, incomingInterrupt.pid);
    interruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
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
    OHOS::AudioStandard::UpdateAudioSceneFromInterruptFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceActivateAudioSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsSessionNeedToFetchOutputDeviceFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceSetAudioSessionSceneFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceAddActiveInterruptToSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceDeactivateAudioSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceRemovePlaceholderInterruptForSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsAudioSessionActivatedFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsCanMixInterruptFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceCanMixForSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceCanMixForIncomingSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsIncomingStreamLowPriorityFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsActiveStreamLowPriorityFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceUnsetAudioManagerInterruptCallbackFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceRequestAudioFocusFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceAbandonAudioFocusFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceUnsetAudioInterruptCallbackFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceAudioInterruptIsActiveInFocusListFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceHandleAppStreamTypeFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceActivateAudioInterruptFuzzTest,
    OHOS::AudioStandard::AudioInterruptServicePrintLogsOfFocusStrategyBaseMusicFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceClearAudioFocusInfoListFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceActivatePreemptModeFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceInjectInterruptToAudioZoneFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceGetAudioFocusInfoListFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceGetStreamInFocusByUidFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceGetSessionInfoInFocusFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsSameAppInShareModeFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceCheckAudioSessionExistenceFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceUpdateHintTypeForExistingSessionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceProcessRemoteInterruptFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceProcessActiveInterruptFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceHandleLowPriorityEventFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceSendActiveInterruptEventFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceAudioFocusInfoListRemovalConditionFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsMediaStreamFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceUpdateAudioFocusStrategyFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceIsMicSourceFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceFocusEntryContinueFuzzTest,
    OHOS::AudioStandard::AudioInterruptServiceProcessFocusEntryFuzzTest,
    OHOS::AudioStandard::GetHighestPriorityAudioSceneFuzzTest,
    OHOS::AudioStandard::GetStreamTypePriorityFuzzTest,
    OHOS::AudioStandard::DeactivatePreemptModeFuzzTest,
    OHOS::AudioStandard::IsCapturerFocusAvailableFuzzTest,
    OHOS::AudioStandard::ClearAudioFocusBySessionIDFuzzTest,
    OHOS::AudioStandard::DeactivateAudioSessionInFakeFocusModeFuzzTest,
    OHOS::AudioStandard::DeactivateAudioSessionFakeInterruptFuzzTest,
    OHOS::AudioStandard::AudioSessionInfoDumpFuzzTest,
    OHOS::AudioStandard::SetSessionMuteStateFuzzTest,
    OHOS::AudioStandard::SetLatestMuteStateFuzzTest,
    OHOS::AudioStandard::UpdateMuteAudioFocusStrategyFuzzTest,
    OHOS::AudioStandard::ReportRecordGetFocusFailFuzzTest,
    OHOS::AudioStandard::ProcessActiveStreamFocusFuzzTest,
    OHOS::AudioStandard::CanMixForActiveSessionFuzzTest,
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