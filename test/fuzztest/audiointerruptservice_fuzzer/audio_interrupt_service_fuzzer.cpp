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

#include "../fuzz_utils.h"
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_interrupt_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const uint32_t TEST_ID_MODULO = 3;
constexpr uint32_t BOOL_MODULO = 2;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)(const uint8_t *, size_t);

class AudioInterruptCallbackFuzzTest : public AudioInterruptCallback {
public:
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override {};
};

class AudioSessionServiceBuilder {
public:
    AudioSessionServiceBuilder(shared_ptr<AudioInterruptService> &interruptService, int32_t id)
    {
        if (interruptService == nullptr) {
            return;
        }
        AudioSessionStrategy strategy;
        interruptService->sessionService_.sessionMap_.insert(
            std::make_pair(id, std::make_shared<AudioSession>(id, strategy, audioSessionService_)));
    }

    ~AudioSessionServiceBuilder()
    {
        audioSessionService_.sessionMap_.clear();
        audioSessionService_.timeOutCallback_.reset();
    }
private:
    AudioSessionService &audioSessionService_ {OHOS::Singleton<AudioSessionService>::GetInstance()};
};

void InitFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioPolicyServer> server = nullptr;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->Init(server);
}

void AddDumpInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::unordered_map<int32_t, std::shared_ptr<AudioInterruptZone>> audioInterruptZonesMapDump;
    interruptService->AddDumpInfo(audioInterruptZonesMapDump);
}

void SetCallbackHandlerFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPolicyServerHandler> handler = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->SetCallbackHandler(handler);
}

void SetAudioManagerInterruptCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->SetAudioManagerInterruptCallback(object);
}

void ActivateAudioInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(RAW_DATA);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);
    interruptService->ActivateAudioInterrupt(zoneId, audioInterrupt);
}

void DeactivateAudioInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(RAW_DATA);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);
    interruptService->DeactivateAudioInterrupt(zoneId, audioInterrupt);
}

void CreateAudioInterruptZoneFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioZoneContext context;
    interruptService->CreateAudioInterruptZone(zoneId, context);
}

void ReleaseAudioInterruptZoneFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };

    interruptService->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
}

void RemoveAudioInterruptZonePidsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };
    interruptService->MigrateAudioInterruptZone(zoneId, getZoneFunc);
}

void GetStreamInFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    interruptService->GetStreamInFocus(zoneId);
}

void GetSessionInfoInFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(RAW_DATA);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);

    interruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
}

void DispatchInterruptEventWithStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = *reinterpret_cast<const InterruptType *>(RAW_DATA);
    interruptEvent.forceType = *reinterpret_cast<const InterruptForceType *>(RAW_DATA);
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    interruptEvent.duckVolume = 0;

    interruptService->DispatchInterruptEventWithStreamId(sessionId, interruptEvent);
}

void RequestAudioFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t clientId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(RAW_DATA);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);

    interruptService->RequestAudioFocus(clientId, audioInterrupt);
}

void AbandonAudioFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t clientId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(RAW_DATA);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);

    interruptService->AbandonAudioFocus(clientId, audioInterrupt);
}

void SetAudioInterruptCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();

    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    uint32_t uid = *reinterpret_cast<const uint32_t *>(RAW_DATA);

    interruptService->SetAudioInterruptCallback(zoneId, sessionId, object, uid);
}

void UnsetAudioInterruptCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    CHECK_AND_RETURN(interruptService != nullptr);
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(RAW_DATA);

    interruptService->UnsetAudioInterruptCallback(zoneId, sessionId);
}

void AddAudioInterruptZonePidsFuzzTest(FuzzedDataProvider& fdp)
{

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };
    interruptService->MigrateAudioInterruptZone(zoneId, getZoneFunc);
}

void UpdateAudioSceneFromInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    AudioScene audioScene = *reinterpret_cast<const AudioScene *>(RAW_DATA);
    AudioInterruptChangeType changeType = *reinterpret_cast<const AudioInterruptChangeType *>(RAW_DATA);
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}

void AudioInterruptServiceActivateAudioSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionStrategy strategy;
    AudioSessionServiceBuilder(interruptService, callerPid);
    interruptService->ActivateAudioSession(zoneId, callerPid, strategy);
}

void AudioInterruptServiceIsSessionNeedToFetchOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }

    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionServiceBuilder(interruptService, callerPid);
    interruptService->sessionService_.IsSessionNeedToFetchOutputDevice(callerPid);
}

void AudioInterruptServiceSetAudioSessionSceneFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionServiceBuilder(interruptService, callerPid);
    AudioSessionScene scene = AudioSessionScene::INVALID;
    interruptService->SetAudioSessionScene(callerPid, scene);
}

void AudioInterruptServiceAddActiveInterruptToSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionServiceBuilder(interruptService, callerPid);
    interruptService->AddActiveInterruptToSession(callerPid);
}

void AudioInterruptServiceDeactivateAudioSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionServiceBuilder(interruptService, callerPid);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, std::make_shared<AudioInterruptZone>()));
    interruptService->DeactivateAudioSession(zoneId, callerPid);
}

void AudioInterruptServiceRemovePlaceholderInterruptForSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionServiceBuilder(interruptService, callerPid);
    bool isSessionTimeout = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    interruptService->RemovePlaceholderInterruptForSession(callerPid, isSessionTimeout);
}

void AudioInterruptServiceIsAudioSessionActivatedFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioSessionServiceBuilder(interruptService, callerPid);
    interruptService->IsAudioSessionActivated(callerPid);
}

void AudioInterruptServiceIsCanMixInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    uint32_t testId = *reinterpret_cast<const uint32_t *>(RAW_DATA) % TEST_ID_MODULO;
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

void AudioInterruptServiceCanMixForSessionFuzzTest(FuzzedDataProvider& fdp)
{
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
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
}

void AudioInterruptServiceCanMixForIncomingSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioFocusEntry focusEntry;
    AudioSessionServiceBuilder(interruptService, incomingInterrupt.pid);
    interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
}

void AudioInterruptServiceIsIncomingStreamLowPriorityFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioFocusEntry focusEntry;
    uint32_t testId = *reinterpret_cast<const uint32_t *>(RAW_DATA) % TEST_ID_MODULO;
    if (testId == 0) {
        focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
        focusEntry.actionOn = INCOMING;
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
    } else if (testId == 1) {
        focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
        focusEntry.actionOn = INCOMING;
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
    } else {
        focusEntry.isReject = false;
        focusEntry.actionOn = BOTH;
    }
    interruptService->IsIncomingStreamLowPriority(focusEntry);
}

void AudioInterruptServiceIsActiveStreamLowPriorityFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioFocusEntry focusEntry;
    bool testFalse = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    if (testFalse) {
        focusEntry.actionOn = BOTH;
    } else {
        focusEntry.actionOn = CURRENT;
        focusEntry.hintType = INTERRUPT_HINT_DUCK;
    }
    interruptService->IsActiveStreamLowPriority(focusEntry);
}

void AudioInterruptServiceUnsetAudioManagerInterruptCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    std::shared_ptr<AudioPolicyServerHandler> handler = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    interruptService->SetCallbackHandler(handler);
    interruptService->UnsetAudioManagerInterruptCallback();
}

void AudioInterruptServiceRequestAudioFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t clientId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    bool isNotEqual = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    if (!isNotEqual) {
        interruptService->clientOnFocus_ = clientId;
    }
    interruptService->focussedAudioInterruptInfo_ = make_unique<AudioInterrupt>();
    AudioInterrupt audioInterrupt;

    interruptService->RequestAudioFocus(clientId, audioInterrupt);
}

void AudioInterruptServiceAbandonAudioFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t clientId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    interruptService->clientOnFocus_ = clientId;
    AudioInterrupt audioInterrupt;
    interruptService->AbandonAudioFocus(clientId, audioInterrupt);
}

void AudioInterruptServiceUnsetAudioInterruptCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    uint32_t streamId = *reinterpret_cast<const uint32_t *>(RAW_DATA) + 1;
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

void AudioInterruptServiceAudioInterruptIsActiveInFocusListFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    uint32_t incomingStreamId = *reinterpret_cast<const uint32_t *>(RAW_DATA) + 1;
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    interruptService->AudioInterruptIsActiveInFocusList(zoneId, incomingStreamId);
}

void AudioInterruptServiceHandleAppStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);

    AudioSessionServiceBuilder(interruptService, audioInterrupt.pid);
    interruptService->HandleAppStreamType(0, audioInterrupt);
}

void AudioInterruptServiceActivateAudioInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    AudioSessionServiceBuilder(interruptService, zoneId);
    bool isUpdatedAudioStrategy = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    interruptService->isPreemptMode_ = !((*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO);
    interruptService->ActivateAudioInterrupt(zoneId, audioInterrupt, isUpdatedAudioStrategy);
}

void AudioInterruptServicePrintLogsOfFocusStrategyBaseMusicFuzzTest(FuzzedDataProvider& fdp)
{
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
    AudioSessionServiceBuilder(interruptService, 0);
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair = std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_STOP;
    focusEntry.actionOn = CURRENT;
    interruptService->focusCfgMap_.insert(std::make_pair(focusPair, focusEntry));
    uint32_t index = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    audioInterrupt.sessionStrategy.concurrencyMode = concurrencyModes[index % concurrencyModes.size()];

    interruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
}

void AudioInterruptServiceClearAudioFocusInfoListFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    interruptService->ClearAudioFocusInfoList();
}

void AudioInterruptServiceActivatePreemptModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    interruptService->ActivatePreemptMode();
    interruptService->DeactivatePreemptMode();
}

void AudioInterruptServiceInjectInterruptToAudioZoneFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    AudioFocusList interrupts;
    interrupts.emplace_back(audioFocusInfo);
    interruptService->InjectInterruptToAudioZone(zoneId, interrupts);
}

void AudioInterruptServiceGetAudioFocusInfoListFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    std::string deviceTag = "testdevice";
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    AudioFocusList interrupts;
    interrupts.emplace_back(audioFocusInfo);
    interruptService->GetAudioFocusInfoList(zoneId, deviceTag, interrupts);
}

void AudioInterruptServiceGetStreamInFocusByUidFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t uid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));
    interruptService->GetStreamInFocusByUid(uid, zoneId);
}

void AudioInterruptServiceGetSessionInfoInFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t uid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));
    AudioInterrupt interrupt;
    interruptService->GetSessionInfoInFocus(interrupt, zoneId);
}

void AudioInterruptServiceIsSameAppInShareModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t testId = *reinterpret_cast<const uint32_t *>(RAW_DATA) % TEST_ID_MODULO;
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

void AudioInterruptServiceUpdateHintTypeForExistingSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t testId = *reinterpret_cast<const uint32_t *>(RAW_DATA) % TEST_ID_MODULO;
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

void AudioInterruptServiceProcessRemoteInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || audioInterruptZone == nullptr) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = *reinterpret_cast<const bool *>(RAW_DATA);
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    interruptService->zonesMap_.insert({pid, audioInterruptZone});
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    std::set<int32_t> sessionIds;
    interruptService->ProcessRemoteInterrupt(sessionIds, interruptEvent);
}

void AudioInterruptServiceProcessActiveInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || audioInterruptZone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = zoneId;
    audioInterrupt.isAudioSessionInterrupt = *reinterpret_cast<const bool *>(RAW_DATA);
    interruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    audioInterruptZone->audioFocusInfoList.push_back(
        {audioInterrupt, *reinterpret_cast<const AudioFocuState *>(RAW_DATA)});
    interruptService->policyServer_ = nullptr;
    interruptService->ProcessActiveInterrupt(zoneId, audioInterrupt);
}

void AudioInterruptServiceHandleLowPriorityEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t streamId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    interruptService->SetAudioSessionScene(pid, *reinterpret_cast<const AudioSessionScene *>(RAW_DATA));
    if (interruptService->sessionService_.sessionMap_[pid] == nullptr) {
        return;
    }
    interruptService->sessionService_.sessionMap_[pid]->audioSessionScene_ =
        *reinterpret_cast<const AudioSessionScene *>(RAW_DATA);
    interruptService->sessionService_.sessionMap_[pid]->state_ = *reinterpret_cast<const AudioSessionState *>(RAW_DATA);
    interruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    interruptService->HandleLowPriorityEvent(pid, streamId);
}

void AudioInterruptServiceSendActiveInterruptEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t streamId = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    interruptService->SendActiveInterruptEvent(streamId, interruptEvent, incomingInterrupt, activeInterrupt);
}

void AudioInterruptServiceAudioFocusInfoListRemovalConditionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = zoneId;
    audioInterrupt.isAudioSessionInterrupt = *reinterpret_cast<const bool *>(RAW_DATA);
    audioInterrupt.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(RAW_DATA);
    AudioFocuState audioFocusState = *reinterpret_cast<const AudioFocuState *>(RAW_DATA);
    std::pair<AudioInterrupt, AudioFocuState> audioInterruptPair = std::make_pair(audioInterrupt, audioFocusState);

    interruptService->AudioFocusInfoListRemovalCondition(audioInterrupt, audioInterruptPair);
}

void AudioInterruptServiceIsMediaStreamFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioStreamType audioStreamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);

    interruptService->IsMediaStream(audioStreamType);
}

void AudioInterruptServiceUpdateAudioFocusStrategyFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt currentInterrupt;
    currentInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.uid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    incomingInterrupt.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(RAW_DATA);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = *reinterpret_cast<const ActionTarget *>(RAW_DATA);
    focusEntry.forceType = *reinterpret_cast<const InterruptForceType *>(RAW_DATA);
    focusEntry.hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    interruptService->policyServer_ = nullptr;

    interruptService->UpdateAudioFocusStrategy(currentInterrupt, incomingInterrupt, focusEntry);
}

void AudioInterruptServiceIsMicSourceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    SourceType sourceType = *reinterpret_cast<const SourceType *>(RAW_DATA);

    interruptService->IsMicSource(sourceType);
}

void AudioInterruptServiceFocusEntryContinueFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    audioInterrupt.uid = AUDIO_ID;
    SourceType sourceType = *reinterpret_cast<const SourceType *>(RAW_DATA);
    audioInterrupt.currencySources.sourcesTypes.push_back(sourceType);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    list.push_back({audioInterrupt, ACTIVE});
    auto iterActive = list.begin();
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = *reinterpret_cast<const ActionTarget *>(RAW_DATA);
    focusEntry.forceType = *reinterpret_cast<const InterruptForceType *>(RAW_DATA);
    focusEntry.hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    focusEntry.isReject = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    interruptService->FocusEntryContinue(iterActive, focusEntry, incomingInterrupt);
}

void AudioInterruptServiceProcessFocusEntryFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioInterrupt audioInterrupt;
    pair<AudioInterrupt, AudioFocuState> audioFocusInfo = std::make_pair(audioInterrupt, AudioFocuState::MUTED);
    zone->audioFocusInfoList.emplace_back(audioFocusInfo);
    interruptService->zonesMap_.insert(std::make_pair(zoneId, zone));

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(RAW_DATA);
    interruptService->ProcessFocusEntry(zoneId, incomingInterrupt);
}

void GetHighestPriorityAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    interruptService->GetHighestPriorityAudioScene(zoneId);
}
 
void GetStreamTypePriorityFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(RAW_DATA);
    interruptService->GetStreamTypePriority(streamType);
}
 
void DeactivatePreemptModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->DeactivatePreemptMode();
}
 
void IsCapturerFocusAvailableFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
 
    uint32_t zoneId = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    AudioCapturerInfo capturerInfo;
    interruptService->IsCapturerFocusAvailable(zoneId, capturerInfo);
}
 
void ClearAudioFocusBySessionIDFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t sessionID = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    interruptService->ClearAudioFocusBySessionID(sessionID);
}
 
void DeactivateAudioSessionInFakeFocusModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    InterruptHint hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    interruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);
}
 
void DeactivateAudioSessionFakeInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t callerPid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    bool isSessionTimeout = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    interruptService->DeactivateAudioSessionFakeInterrupt(zoneId, callerPid);
}
 
void SetSessionMuteStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    bool insert = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    bool muteFlag = (*reinterpret_cast<const uint32_t *>(RAW_DATA)) % BOOL_MODULO;
    if (interruptService == nullptr) {
        return;
    }
    interruptService->SetSessionMuteState(sessionId, insert, muteFlag);
}
 
void SetLatestMuteStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = *reinterpret_cast<const InterruptType *>(RAW_DATA);
    interruptEvent.forceType = *reinterpret_cast<const InterruptForceType *>(RAW_DATA);
    interruptEvent.hintType = *reinterpret_cast<const InterruptHint *>(RAW_DATA);
    interruptEvent.duckVolume = 0;
    uint32_t streamId = *reinterpret_cast<const uint32_t *>(RAW_DATA);
    interruptService->SetLatestMuteState(interruptEvent, streamId);
}
 
void UpdateMuteAudioFocusStrategyFuzzTest(FuzzedDataProvider& fdp)
{
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
 
void ReportRecordGetFocusFailFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt activeInterrupt;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    activeInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    int32_t reason = *reinterpret_cast<const int32_t *>(RAW_DATA);
    interruptService->ReportRecordGetFocusFail(incomingInterrupt, activeInterrupt, reason);
}
 
void ProcessActiveStreamFocusFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (interruptService == nullptr || zone == nullptr) {
        return;
    }
    int32_t zoneId = *reinterpret_cast<const int32_t *>(RAW_DATA);
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

void CanMixForActiveSessionFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    incomingInterrupt.pid = *reinterpret_cast<const int32_t *>(RAW_DATA);
    AudioFocusEntry focusEntry;
    AudioSessionServiceBuilder(interruptService, incomingInterrupt.pid);
    interruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
}
void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
	InitFuzzTest,
	AddDumpInfoFuzzTest,
	SetCallbackHandlerFuzzTest,
	SetAudioManagerInterruptCallbackFuzzTest,
	ActivateAudioInterruptFuzzTest,
	DeactivateAudioInterruptFuzzTest,
	CreateAudioInterruptZoneFuzzTest,
	ReleaseAudioInterruptZoneFuzzTest,
	RemoveAudioInterruptZonePidsFuzzTest,
	GetStreamInFocusFuzzTest,
	GetSessionInfoInFocusFuzzTest,
	DispatchInterruptEventWithStreamIdFuzzTest,
	RequestAudioFocusFuzzTest,
	AbandonAudioFocusFuzzTest,
	SetAudioInterruptCallbackFuzzTest,
	UnsetAudioInterruptCallbackFuzzTest,
	AddAudioInterruptZonePidsFuzzTest,
	UpdateAudioSceneFromInterruptFuzzTest,
	AudioInterruptServiceActivateAudioSessionFuzzTest,
	AudioInterruptServiceIsSessionNeedToFetchOutputDeviceFuzzTest,
	AudioInterruptServiceSetAudioSessionSceneFuzzTest,
	AudioInterruptServiceAddActiveInterruptToSessionFuzzTest,
	AudioInterruptServiceDeactivateAudioSessionFuzzTest,
	AudioInterruptServiceRemovePlaceholderInterruptForSessionFuzzTest,
	AudioInterruptServiceIsAudioSessionActivatedFuzzTest,
	AudioInterruptServiceIsCanMixInterruptFuzzTest,
	AudioInterruptServiceCanMixForSessionFuzzTest,
	AudioInterruptServiceCanMixForIncomingSessionFuzzTest,
	AudioInterruptServiceIsIncomingStreamLowPriorityFuzzTest,
	AudioInterruptServiceIsActiveStreamLowPriorityFuzzTest,
	AudioInterruptServiceUnsetAudioManagerInterruptCallbackFuzzTest,
	AudioInterruptServiceRequestAudioFocusFuzzTest,
	AudioInterruptServiceAbandonAudioFocusFuzzTest,
	AudioInterruptServiceUnsetAudioInterruptCallbackFuzzTest,
	AudioInterruptServiceAudioInterruptIsActiveInFocusListFuzzTest,
	AudioInterruptServiceHandleAppStreamTypeFuzzTest,
	AudioInterruptServiceActivateAudioInterruptFuzzTest,
	AudioInterruptServicePrintLogsOfFocusStrategyBaseMusicFuzzTest,
	AudioInterruptServiceClearAudioFocusInfoListFuzzTest,
	AudioInterruptServiceActivatePreemptModeFuzzTest,
	AudioInterruptServiceInjectInterruptToAudioZoneFuzzTest,
	AudioInterruptServiceGetAudioFocusInfoListFuzzTest,
	AudioInterruptServiceGetStreamInFocusByUidFuzzTest,
	AudioInterruptServiceGetSessionInfoInFocusFuzzTest,
	AudioInterruptServiceIsSameAppInShareModeFuzzTest,
	AudioInterruptServiceUpdateHintTypeForExistingSessionFuzzTest,
	AudioInterruptServiceProcessRemoteInterruptFuzzTest,
	AudioInterruptServiceProcessActiveInterruptFuzzTest,
	AudioInterruptServiceHandleLowPriorityEventFuzzTest,
	AudioInterruptServiceSendActiveInterruptEventFuzzTest,
	AudioInterruptServiceAudioFocusInfoListRemovalConditionFuzzTest,
	AudioInterruptServiceIsMediaStreamFuzzTest,
	AudioInterruptServiceUpdateAudioFocusStrategyFuzzTest,
	AudioInterruptServiceIsMicSourceFuzzTest,
	AudioInterruptServiceFocusEntryContinueFuzzTest,
	AudioInterruptServiceProcessFocusEntryFuzzTest,
	GetHighestPriorityAudioSceneFuzzTest,
	GetStreamTypePriorityFuzzTest,
	DeactivatePreemptModeFuzzTest,
	IsCapturerFocusAvailableFuzzTest,
	ClearAudioFocusBySessionIDFuzzTest,
	DeactivateAudioSessionInFakeFocusModeFuzzTest,
	DeactivateAudioSessionFakeInterruptFuzzTest,
	SetSessionMuteStateFuzzTest,
	SetLatestMuteStateFuzzTest,
	UpdateMuteAudioFocusStrategyFuzzTest,
	ReportRecordGetFocusFailFuzzTest,
	ProcessActiveStreamFocusFuzzTest,
	CanMixForActiveSessionFuzzTest,
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
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    data = data + 1;
    size = size - 1;
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