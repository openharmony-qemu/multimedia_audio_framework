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

#include "audio_interrupt_unit_test.h"
#include "audio_service_log.h"

#include "audio_interrupt_service.h"
#include "audio_policy_server.h"
#include <thread>
#include <memory>
#include <vector>
#include "binder_invoker.h"
#include "invoker_factory.h"
#include "ipc_thread_skeleton.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static AudioSessionStrategy strategyTest;

static std::shared_ptr<AudioInterruptService> audioInterruptService;
static sptr<AudioPolicyServer> serverTest = nullptr;
const int32_t CALLER_PID_TEST = 0;
const int32_t PIT_TEST = 0;
const int32_t SESSION_ID_TEST = 0;

const int32_t CALLER_PID = 0;
const int32_t DEFAULT_ZONE_ID = 0;
const int32_t VALUE_ERROR = -62980098;
const int32_t VALUE_SUCCESS = 1065353216;
const int32_t SYSTEM_ABILITY_ID = 3009;
const int32_t CALLER_UID = 1041;
const int32_t INTERRUPT_HINT_ERROR = -1;
const bool RUN_ON_CREATE = false;
const bool IS_SESSION_TIMEOUT = false;

void SetUid1041()
{
    IRemoteInvoker *remoteInvoker =
        IPCThreadSkeleton::GetRemoteInvoker(IRemoteObject::IF_PROT_BINDER);
    EXPECT_NE(nullptr, remoteInvoker);
    BinderInvoker *ipcInvoker = (BinderInvoker *)remoteInvoker;
    ipcInvoker->status_ = IRemoteInvoker::ACTIVE_INVOKER;
    auto state = remoteInvoker->GetStatus();
    EXPECT_EQ(state, IRemoteInvoker::ACTIVE_INVOKER);
    ipcInvoker->callerUid_ = CALLER_UID;
}

void AudioInterruptUnitTest::SetUpTestCase(void) {}

void AudioInterruptUnitTest::TearDownTestCase(void)
{
    audioInterruptService.reset();
    serverTest = nullptr;
}
void AudioInterruptUnitTest::SetUp(void) {}
void AudioInterruptUnitTest::TearDown(void) {}

std::shared_ptr<AudioInterruptService> GetTnterruptServiceTest()
{
    return std::make_shared<AudioInterruptService>();
}

std::shared_ptr<AudioPolicyServerHandler> GetServerHandlerTest()
{
    return DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
}

bool g_hasServerInit = false;
sptr<AudioPolicyServer> GetPolicyServerTest()
{
    static int32_t systemAbilityId = 3009;
    static bool runOnCreate = false;
    static sptr<AudioPolicyServer> server =
        sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    if (!g_hasServerInit) {
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
}

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_004
* @tc.desc  : Test AddDumpInfo.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_004, TestSize.Level1)
{
    std::string dumpString;
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), true);
    interruptServiceTest->AudioInterruptZoneDump(dumpString);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[0]->interruptCbsMap[0] = nullptr;
    interruptServiceTest->AudioInterruptZoneDump(dumpString);
    EXPECT_NE(dumpString.find("1 AudioInterruptZoneDump (s) available"), std::string::npos);
    EXPECT_NE(dumpString.find("Interrupt callback size: 1"), std::string::npos);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_005
* @tc.desc  : Test AbandonAudioFocus.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_005, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt incomingInterrupt;
    int32_t clientID = interruptServiceTest->clientOnFocus_;
    auto retStatus = interruptServiceTest->AbandonAudioFocus(clientID, incomingInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);
}

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_006
* @tc.desc  : Test SetAudioInterruptCallback and UnsetAudioInterruptCallback.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_006, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    auto retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, nullptr, 0);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr(), 0);
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr(), 0);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr(), 0);
    EXPECT_EQ(retStatus, SUCCESS);
    interruptServiceTest->zonesMap_[0]->interruptCbsMap.clear();
    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr(), 0);
    EXPECT_EQ(retStatus, SUCCESS);
    interruptServiceTest->zonesMap_[0] = nullptr;
    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, SUCCESS);

    retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0, sptr<RemoteObjectTestStub>::MakeSptr(), 0);
    EXPECT_EQ(retStatus, SUCCESS);
    interruptServiceTest->zonesMap_.clear();
    retStatus = interruptServiceTest->UnsetAudioInterruptCallback(0, 0);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_008
* @tc.desc  : Test ResumeAudioFocusList and SimulateFocusEntry.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_008, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.size(), 0);
    auto newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_EQ(newAudioFocuInfoList.size(), 0);

    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt1;
    audioInterrupt1.audioFocusType.streamType = STREAM_DEFAULT;
    audioInterrupt1.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterrupt1.audioFocusType.isPlay = false;
    AudioInterrupt audioInterrupt2;
    audioInterrupt2.audioFocusType.streamType = STREAM_DEFAULT;
    audioInterrupt2.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterrupt2.audioFocusType.isPlay = false;

    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt1, AudioFocuState{PAUSE});
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt2, AudioFocuState{});
    interruptServiceTest->zonesMap_[0] = audioInterruptZone;
    newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    interruptServiceTest->ResumeAudioFocusList(0, true);
    interruptServiceTest->ResumeAudioFocusList(0, false);
    EXPECT_EQ(newAudioFocuInfoList.size(), 2);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_010
* @tc.desc  : Test ResumeAudioFocusList and SimulateFocusEntry.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_010, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.size(), 0);
    auto newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_EQ(newAudioFocuInfoList.size(), 0);

    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt1;
    audioInterrupt1.audioFocusType.streamType = STREAM_DEFAULT;
    audioInterrupt1.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterrupt1.audioFocusType.isPlay = false;
    AudioInterrupt audioInterrupt2;
    audioInterrupt2.audioFocusType.streamType = STREAM_DEFAULT;
    audioInterrupt2.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterrupt2.audioFocusType.isPlay = false;

    audioInterruptZone->audioFocusInfoList.clear();
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt1, AudioFocuState{PAUSE});
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt2, AudioFocuState{DUCK});
    interruptServiceTest->zonesMap_[0] = audioInterruptZone;
    newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    interruptServiceTest->ResumeAudioFocusList(0, true);
    interruptServiceTest->ResumeAudioFocusList(0, false);
    EXPECT_EQ(newAudioFocuInfoList.size(), 2);

    audioInterruptZone->audioFocusInfoList.clear();
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt1, AudioFocuState{PAUSE});
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt2, AudioFocuState{STOP});
    interruptServiceTest->zonesMap_[0] = audioInterruptZone;
    newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    interruptServiceTest->ResumeAudioFocusList(0, true);
    interruptServiceTest->ResumeAudioFocusList(0, false);
    EXPECT_EQ(newAudioFocuInfoList.size(), 2);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_012
* @tc.desc  : Test ResumeAudioFocusList and SimulateFocusEntry.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_012, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.size(), 0);
    auto newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_EQ(newAudioFocuInfoList.size(), 0);
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(AudioInterrupt(), AudioFocuState{PAUSE});
    audioInterruptZone->audioFocusInfoList.emplace_back(AudioInterrupt(), AudioFocuState{});
    interruptServiceTest->zonesMap_[0] = audioInterruptZone;
    newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    interruptServiceTest->ResumeAudioFocusList(0, true);
    interruptServiceTest->ResumeAudioFocusList(0, false);
    EXPECT_EQ(newAudioFocuInfoList.size(), 2);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_013
* @tc.desc  : Test ResumeAudioFocusList and SimulateFocusEntry.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_013, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.size(), 0);
    auto newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_EQ(newAudioFocuInfoList.size(), 0);
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(AudioInterrupt(), AudioFocuState{PAUSE});
    audioInterruptZone->audioFocusInfoList.emplace_back(AudioInterrupt(), AudioFocuState{});
    interruptServiceTest->zonesMap_[0] = audioInterruptZone;
    newAudioFocuInfoList = interruptServiceTest->SimulateFocusEntry(0);
    interruptServiceTest->ResumeAudioFocusList(0, true);
    interruptServiceTest->ResumeAudioFocusList(0, false);
    EXPECT_EQ(newAudioFocuInfoList.size(), 2);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_015
* @tc.desc  : Test SendInterruptEvent.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_015, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> pairList;
    pairList.emplace_back(AudioInterrupt(), AudioFocuState::ACTIVE);
    AudioFocuState oldState{};
    AudioFocuState newState{};
    auto it = pairList.begin();
    bool removeFocusInfo = true;
    interruptServiceTest->SendInterruptEvent(oldState, newState, it, removeFocusInfo);
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
    interruptServiceTest->SendInterruptEvent(oldState, newState, it, removeFocusInfo);

    interruptServiceTest->SendInterruptEvent(PAUSE, ACTIVE, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(DUCK, ACTIVE, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(PAUSE, DUCK, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(DUCK, DUCK, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(DUCK, PAUSE, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(PAUSE, PAUSE, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(DUCK, PLACEHOLDER, it, removeFocusInfo);
    interruptServiceTest->SendInterruptEvent(DUCK, STOP, it, removeFocusInfo);
    EXPECT_NE(interruptServiceTest->handler_, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_017
* @tc.desc  : Test IsActiveStreamLowPriority and IsIncomingStreamLowPriority.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_017, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusEntry entry;
    entry.actionOn = INCOMING;
    bool ret = interruptServiceTest->IsActiveStreamLowPriority(entry);
    EXPECT_EQ(ret, false);

    entry.actionOn = CURRENT;
    entry.hintType = INTERRUPT_HINT_PAUSE;
    ret = interruptServiceTest->IsActiveStreamLowPriority(entry);
    EXPECT_EQ(ret, true);
    entry.hintType = INTERRUPT_HINT_STOP;
    ret = interruptServiceTest->IsActiveStreamLowPriority(entry);
    EXPECT_EQ(ret, true);
    entry.hintType = INTERRUPT_HINT_DUCK;
    ret = interruptServiceTest->IsActiveStreamLowPriority(entry);
    EXPECT_EQ(ret, true);

    entry.hintType = INTERRUPT_HINT_UNDUCK;
    ret = interruptServiceTest->IsActiveStreamLowPriority(entry);
    EXPECT_EQ(ret, false);
    entry.isReject = true;
    ret = interruptServiceTest->IsIncomingStreamLowPriority(entry);
    EXPECT_EQ(ret, true);
    entry.isReject = false;
    entry.actionOn = CURRENT;
    ret = interruptServiceTest->IsIncomingStreamLowPriority(entry);
    EXPECT_EQ(ret, false);

    entry.actionOn = INCOMING;
    entry.hintType = INTERRUPT_HINT_PAUSE;
    ret = interruptServiceTest->IsIncomingStreamLowPriority(entry);
    EXPECT_EQ(ret, true);
    entry.hintType = INTERRUPT_HINT_STOP;
    ret = interruptServiceTest->IsIncomingStreamLowPriority(entry);
    EXPECT_EQ(ret, true);
    entry.hintType = INTERRUPT_HINT_DUCK;
    ret = interruptServiceTest->IsIncomingStreamLowPriority(entry);
    EXPECT_EQ(ret, true);
    entry.hintType = INTERRUPT_HINT_NONE;
    ret = interruptServiceTest->IsIncomingStreamLowPriority(entry);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_018
* @tc.desc  : Test WriteServiceStartupError.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_018, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    EXPECT_NO_THROW(
        interruptServiceTest->WriteServiceStartupError();
    );
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_019
* @tc.desc  : Test SendFocusChangeEvent.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_019, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt audioInterrupt;
    EXPECT_NO_THROW(
        interruptServiceTest->SendFocusChangeEvent(0, 0, audioInterrupt);

        interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
        interruptServiceTest->zonesMap_.clear();
        interruptServiceTest->SendFocusChangeEvent(0, 0, audioInterrupt);

        interruptServiceTest->zonesMap_[0] = nullptr;
        interruptServiceTest->SendFocusChangeEvent(0, 0, audioInterrupt);

        interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();;
        interruptServiceTest->SendFocusChangeEvent(0, 0, audioInterrupt);
    );
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_021
* @tc.desc  : Test ClearAudioFocusInfoListOnAccountsChanged.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_021, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    sptr<AudioPolicyServer> server = nullptr;
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->handler_ = nullptr;

    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0);
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0);
    AudioInterrupt a1, a2, a3;
    a1.streamUsage = StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    a2.streamUsage = StreamUsage::STREAM_USAGE_VOICE_RINGTONE;
    a3.streamUsage = StreamUsage::STREAM_USAGE_UNKNOWN;
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a2, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a3, AudioFocuState::ACTIVE});
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 3);
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0);
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 2);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_022
* @tc.desc  : Test ResetNonInterruptControl.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_022, TestSize.Level1)
{
    uint32_t sessionId = CLIENT_TYPE_OTHERS;
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->Init(GetPolicyServerTest());
    EXPECT_NO_THROW(
        interruptServiceTest->ResetNonInterruptControl(sessionId);
    );

    sessionId = CLIENT_TYPE_GAME;
    EXPECT_NO_THROW(
        interruptServiceTest->ResetNonInterruptControl(sessionId);
    );

    sessionId = 2;
    EXPECT_NO_THROW(
        interruptServiceTest->ResetNonInterruptControl(sessionId);
    );
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_023
* @tc.desc  : Test CreateAudioInterruptZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_023, TestSize.Level1)
{
    MessageParcel data;
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), true);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[0]->interruptCbsMap[0] = nullptr;
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());
    int32_t zoneId = 1;

    int32_t ret = interruptServiceTest->CreateAudioInterruptZone(zoneId);
    EXPECT_EQ(ret, VALUE_ERROR);

    zoneId = 0;
    ret = interruptServiceTest->CreateAudioInterruptZone(zoneId);
    EXPECT_EQ(ret, VALUE_ERROR);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_024
* @tc.desc  : Test ReleaseAudioInterruptZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_024, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 1;

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 0;
    };

    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), true);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[0]->interruptCbsMap[0] = nullptr;

    int32_t ret = interruptServiceTest->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, VALUE_ERROR);

    zoneId = 0;
    ret = interruptServiceTest->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, VALUE_ERROR);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_025
* @tc.desc  : Test ReleaseAudioInterruptZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_025, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 0;

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 0;
    };

    interruptServiceTest->Init(GetPolicyServerTest());
    MessageParcel data;
    std::set<int32_t> pids;
    pids.insert(data.ReadInt32());

    int32_t ret = interruptServiceTest->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, VALUE_ERROR);

    zoneId = 1;
    ret = interruptServiceTest->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, VALUE_ERROR);

    zoneId = 0;
    ret = interruptServiceTest->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, VALUE_ERROR);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_027
* @tc.desc  : Test GetAudioFocusInfoList.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_027, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 0;

    interruptServiceTest->Init(GetPolicyServerTest());
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};

    int32_t ret = interruptServiceTest->GetAudioFocusInfoList(zoneId, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);

    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), true);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[0]->interruptCbsMap[0] = nullptr;

    ret = interruptServiceTest->GetAudioFocusInfoList(zoneId, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_028
* @tc.desc  : Test GetStreamInFocus.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_028, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 0;

    interruptServiceTest->Init(GetPolicyServerTest());
    EXPECT_NO_THROW(
        interruptServiceTest->GetStreamInFocus(zoneId);
    );

    interruptServiceTest->zonesMap_.clear();
    EXPECT_EQ(interruptServiceTest->zonesMap_.empty(), true);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[0]->interruptCbsMap[0] = nullptr;
    EXPECT_NO_THROW(
        interruptServiceTest->GetStreamInFocus(zoneId);
    );
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_029
* @tc.desc  : Test GetAudioServerProxy.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_029, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    const sptr<IStandardAudioService> result = interruptServiceTest->GetAudioServerProxy();
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_030
* @tc.desc  : Test OnSessionTimeout.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_030, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 2;
    interruptServiceTest->OnSessionTimeout(audioInterrupt.pid);
    EXPECT_NE(interruptServiceTest, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_031
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_031, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    AudioSessionStrategy strategy;
    interruptServiceTest->ActivateAudioSession(CALLER_PID, strategy);
    EXPECT_NE(interruptServiceTest, nullptr);

    interruptServiceTest->sessionService_ = nullptr;
    int32_t result = interruptServiceTest->ActivateAudioSession(CALLER_PID, strategy);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_032
* @tc.desc  : Test AddActiveInterruptToSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_032, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    interruptServiceTest->AddActiveInterruptToSession(CALLER_PID);
    EXPECT_NE(interruptServiceTest, nullptr);

    interruptServiceTest->sessionService_ = nullptr;
    interruptServiceTest->AddActiveInterruptToSession(CALLER_PID);
    EXPECT_NE(interruptServiceTest, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_033
* @tc.desc  : Test DeactivateAudioSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_033, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    int32_t result = interruptServiceTest->DeactivateAudioSession(CALLER_PID);
    EXPECT_NE(interruptServiceTest, nullptr);

    interruptServiceTest->sessionService_ = nullptr;
    result = interruptServiceTest->DeactivateAudioSession(CALLER_PID);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_034
* @tc.desc  : Test CanMixForSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_034, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_NE(interruptServiceTest, nullptr);

    focusEntry.isReject = true;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    bool result = interruptServiceTest->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_035
* @tc.desc  : Test CanMixForIncomingSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_035, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    interruptServiceTest->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_NE(interruptServiceTest, nullptr);

    interruptServiceTest->sessionService_ = nullptr;
    bool result = interruptServiceTest->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_036
* @tc.desc  : Test CanMixForActiveSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_036, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    interruptServiceTest->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_NE(interruptServiceTest, nullptr);

    interruptServiceTest->sessionService_ = nullptr;
    bool result = interruptServiceTest->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_037
* @tc.desc  : Test RequestAudioFocus.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_037, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t clientId = interruptServiceTest->clientOnFocus_;
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_VOICE_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_RING;
    int32_t result = interruptServiceTest->RequestAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_038
* @tc.desc  : Test AbandonAudioFocus.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_038, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t clientId = interruptServiceTest->clientOnFocus_;
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_VOICE_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_RING;
    int32_t result = interruptServiceTest->AbandonAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_039
* @tc.desc  : Test AudioInterruptIsActiveInFocusList.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_039, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 0;
    uint32_t incomingSessionId = 0;
    bool result = interruptServiceTest->AudioInterruptIsActiveInFocusList(zoneId, incomingSessionId);
    EXPECT_EQ(result, 0);
    zoneId = 1;
    incomingSessionId = 1;
    result = interruptServiceTest->AudioInterruptIsActiveInFocusList(zoneId, incomingSessionId);
    EXPECT_EQ(result, 0);
    zoneId = 0;
    incomingSessionId = 2;
    result = interruptServiceTest->AudioInterruptIsActiveInFocusList(zoneId, incomingSessionId);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_001
* @tc.desc  : Test CanMixForIncomingSession. sessionService_ is nullptr.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    interruptService->sessionService_ = nullptr;
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    auto ret = interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_002
* @tc.desc  : Test CanMixForIncomingSession. incomingInterrupt.pid is -1
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_002, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    auto ret = interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_003
* @tc.desc  : Test CanMixForIncomingSession. incomingSession is nullptr.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    int32_t ret = interruptService->ActivateAudioSession(incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_FALSE(interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_004
* @tc.desc  : Test CanMixForIncomingSession. IsIncomingStreamLowPriority(focusEntry) is true.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_004, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    int32_t ret = interruptService->ActivateAudioSession(incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    focusEntry.isReject = true;
    EXPECT_FALSE(interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_005
* @tc.desc  : Test CanMixForIncomingSession. IsIncomingStreamLowPriority(focusEntry) is false.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_005, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    int32_t ret = interruptService->ActivateAudioSession(incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    focusEntry.isReject = false;
    focusEntry.actionOn = CURRENT;
    EXPECT_TRUE(interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_001
* @tc.desc  : Test CanMixForSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = true;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;

    auto ret = interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_002
* @tc.desc  : Test CanMixForSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_002, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;
    focusEntry.actionOn = CURRENT;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    int32_t ret = interruptService->ActivateAudioSession(incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    ret = interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_003
* @tc.desc  : Test CanMixForSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    int32_t ret = interruptService->ActivateAudioSession(incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    focusEntry.actionOn = CURRENT;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    ret = interruptService->ActivateAudioSession(activeInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    ret = interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_004
* @tc.desc  : Test CanMixForSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_004, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    interruptService->SetCallbackHandler(GetServerHandlerTest());
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    activeInterrupt.pid = { -1 };
    AudioFocusEntry focusEntry;
    focusEntry.isReject = true;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;

    auto ret = interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceRemovePlaceholderInterruptForSession_001
* @tc.desc  : Test RemovePlaceholderInterruptForSession. sessionService_ is nullptr.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceRemovePlaceholderInterruptForSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    interruptService->sessionService_ = nullptr;
    int32_t pid = CALLER_PID;
    bool timeOut = IS_SESSION_TIMEOUT;
    interruptService->RemovePlaceholderInterruptForSession(pid, timeOut);
    EXPECT_EQ(nullptr, interruptService->sessionService_);

    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t ret = interruptService->ActivateAudioSession(pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    interruptService->RemovePlaceholderInterruptForSession(pid, timeOut);
    EXPECT_TRUE(interruptService->sessionService_->IsAudioSessionActivated(pid));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceRemovePlaceholderInterruptForSession_002
* @tc.desc  : Test RemovePlaceholderInterruptForSession. About itZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceRemovePlaceholderInterruptForSession_002, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t pid = CALLER_PID;
    bool timeOut = IS_SESSION_TIMEOUT;
    interruptService->zonesMap_.find(DEFAULT_ZONE_ID)->second = nullptr;
    interruptService->RemovePlaceholderInterruptForSession(pid, timeOut);
    EXPECT_EQ(nullptr, interruptService->zonesMap_.find(DEFAULT_ZONE_ID)->second);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 2;
    audioInterrupt.pid = 2;
    interruptService->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, ACTIVE));
    interruptService->RemovePlaceholderInterruptForSession(pid, timeOut);
    EXPECT_FALSE(interruptService->zonesMap_.empty());
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceRemovePlaceholderInterruptForSession_003
* @tc.desc  : Test RemovePlaceholderInterruptForSession. About itZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceRemovePlaceholderInterruptForSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);

    bool timeOut = IS_SESSION_TIMEOUT;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 2;
    audioInterrupt.pid = 2;
    int32_t pid = audioInterrupt.pid;
    interruptService->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, PLACEHOLDER));
    interruptService->RemovePlaceholderInterruptForSession(pid, timeOut);
    EXPECT_FALSE(interruptService->zonesMap_.empty());
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceDeactivateAudioSession_001
* @tc.desc  : Test DeactivateAudioSession.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceDeactivateAudioSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    interruptService->sessionService_ = nullptr;
    int32_t pid = CALLER_PID;
    int32_t ret = interruptService->DeactivateAudioSession(pid);
    EXPECT_EQ(ERR_UNKNOWN, ret);

    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    ret = interruptService->DeactivateAudioSession(pid);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    ret = interruptService->ActivateAudioSession(pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    ret = interruptService->DeactivateAudioSession(pid);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_001
* @tc.desc  : Test AddActiveInterruptToSession. sessionService_ is nullptr.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    interruptService->sessionService_ = nullptr;
    int32_t pid = CALLER_PID;
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_EQ(nullptr, interruptService->sessionService_);

    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_FALSE(interruptService->sessionService_->IsAudioSessionActivated(pid));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_002
* @tc.desc  : Test AddActiveInterruptToSession. About itZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_002, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t pid = CALLER_PID;
    int32_t ret = interruptService->ActivateAudioSession(pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    interruptService->zonesMap_.find(DEFAULT_ZONE_ID)->second = nullptr;
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_EQ(nullptr, interruptService->zonesMap_.find(DEFAULT_ZONE_ID)->second);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 2;
    audioInterrupt.pid = 2;
    interruptService->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, ACTIVE));
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_FALSE(interruptService->zonesMap_.empty());
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_003
* @tc.desc  : Test AddActiveInterruptToSession. About itZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t pid = CALLER_PID;
    int32_t ret = interruptService->ActivateAudioSession(pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = CALLER_PID;
    audioInterrupt.pid = CALLER_PID;
    interruptService->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, ACTIVE));
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_FALSE(interruptService->zonesMap_.empty());
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_004
* @tc.desc  : Test AddActiveInterruptToSession. About itZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_004, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    interruptService->zonesMap_.clear();
    int32_t pid = CALLER_PID;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = CALLER_PID;
    audioInterrupt.pid = CALLER_PID;
    interruptService->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, ACTIVE));
    int32_t ret = interruptService->ActivateAudioSession(pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_FALSE(interruptService->zonesMap_.empty());
    interruptService->zonesMap_.clear();
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_005
* @tc.desc  : Test AddActiveInterruptToSession. About itZone. itZone->second != nullptr.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_005, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    interruptService->zonesMap_.clear();
    int32_t pid = CALLER_PID;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = CALLER_PID;
    audioInterrupt.pid = CALLER_PID;
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    interruptService->zonesMap_[0] = audioInterruptZone;
    int32_t ret = interruptService->ActivateAudioSession(pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_FALSE(interruptService->zonesMap_.empty());
    interruptService->zonesMap_.clear();
    audioInterruptZone.reset();
}

/**
 * @tc.name  : Test GetAudioServerProxy API.
 * @tc.number: AudioInterruptServiceGetAudioServerProxy_001
 * @tc.desc  : Test OnSessionTimeout when g_adProxy is nullptr.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceGetAudioServerProxy_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    sptr<IStandardAudioService> ret = audioInterruptService->GetAudioServerProxy();
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name  : Test GetAudioServerProxy API.
 * @tc.number: AudioInterruptServiceGetAudioServerProxy_002
 * @tc.desc  : Test OnSessionTimeout when g_adProxy is not nullptr.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceGetAudioServerProxy_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    sptr<IStandardAudioService> ret = audioInterruptService->GetAudioServerProxy();
    ret = audioInterruptService->GetAudioServerProxy();
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name  : Test OnSessionTimeout API.
 * @tc.number: AudioInterruptServiceOnSessionTimeout_001
 * @tc.desc  : Test normal OnSessionTimeout.
 *             Test normal HandleSessionTimeOutEvent.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceOnSessionTimeout_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    audioInterruptService->OnSessionTimeout(PIT_TEST);
    EXPECT_NE(nullptr, audioInterruptService->handler_);
}

/**
 * @tc.name  : Test OnSessionTimeout API.
 * @tc.number: AudioInterruptServiceOnSessionTimeout_002
 * @tc.desc  : Test normal OnSessionTimeout.
 *             Test normal HandleSessionTimeOutEvent.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceOnSessionTimeout_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    audioInterruptService->handler_ = nullptr;
    audioInterruptService->OnSessionTimeout(PIT_TEST);
    EXPECT_EQ(nullptr, audioInterruptService->handler_);
}

/**
 * @tc.name  : Test ActivateAudioSession API.
 * @tc.number: AudioInterruptServiceActivateAudioSession_001
 * @tc.desc  : Test ActivateAudioSession when sessionService_ is nullptr.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceActivateAudioSession_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    audioInterruptService->sessionService_ = nullptr;
    int32_t ret =  audioInterruptService->ActivateAudioSession(CALLER_PID_TEST, strategyTest);
    EXPECT_EQ(ERR_UNKNOWN, ret);
}

/**
 * @tc.name  : Test ActivateAudioSession API.
 * @tc.number: AudioInterruptServiceActivateAudioSession_002
 * @tc.desc  : Test normal ActivateAudioSession.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceActivateAudioSession_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    int32_t ret =  audioInterruptService->ActivateAudioSession(CALLER_PID_TEST, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceUnsetAudioManagerInterruptCallback_001
* @tc.desc  : Test UnsetAudioManagerInterruptCallback.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceUnsetAudioManagerInterruptCallback_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(server);

    auto retStatus = interruptServiceTest->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(retStatus, SUCCESS);

    interruptServiceTest->handler_ = GetServerHandlerTest();
    retStatus = interruptServiceTest->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(retStatus, -62980100);
}

/**
 * @tc.name  : Test IsAudioSessionActivated API.
 * @tc.number: AudioInterruptServiceIsAudioSessionActivated_001
 * @tc.desc  : Test IsAudioSessionActivated when sessionService_ is not nullptr.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceIsAudioSessionActivated_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    EXPECT_NE(nullptr, audioInterruptService->sessionService_);
    int32_t PIT_TEST { -1 };
    bool ret = audioInterruptService->IsAudioSessionActivated(PIT_TEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test IsAudioSessionActivated API.
 * @tc.number: AudioInterruptServiceIsAudioSessionActivated_002
 * @tc.desc  : Test IsAudioSessionActivated when sessionService_ is nullptr.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceIsAudioSessionActivated_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    audioInterruptService->sessionService_ = nullptr;
    int32_t PIT_TEST { -1 };
    bool ret = audioInterruptService->IsAudioSessionActivated(PIT_TEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_001
 * @tc.desc  : Test CanMixForActiveSession when return true.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioFocusEntry focusEntry;
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, nullptr);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_->sessionMap_.insert({0, audioSession});
    focusEntry.actionOn = INCOMING;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_002
 * @tc.desc  : Test CanMixForActiveSession when sessionService_ is nullptr.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioFocusEntry focusEntry;
    audioInterruptService->sessionService_ = nullptr;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_003
 * @tc.desc  : Test CanMixForActiveSession when IsAudioSessionActivated is true.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_003, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    activeInterrupt.pid = { -1 };
    AudioFocusEntry focusEntry;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_004
 * @tc.desc  : Test CanMixForActiveSession when concurrencyMode is not MIX_WITH_OTHERS.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_004, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioFocusEntry focusEntry;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, nullptr);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_->sessionMap_.insert({0, audioSession});
    std::shared_ptr<AudioSession> activeSession =
        audioInterruptService->sessionService_->sessionMap_[activeInterrupt.pid];
    activeSession->strategy_.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_005
 * @tc.desc  : Test CanMixForActiveSession when IsActiveStreamLowPriority is true and return false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_005, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, nullptr);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_->sessionMap_.insert({0, audioSession});
    incomingInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_006
 * @tc.desc  : Test CanMixForActiveSession when IsActiveStreamLowPriority is true and return true.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_006, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt incomingInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioInterrupt activeInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, SESSION_ID_TEST);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, nullptr);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_->sessionMap_.insert({0, audioSession});
    incomingInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test SendSessionTimeOutStopEvent
* @tc.number: SendSessionTimeOutStopEvent_001
* @tc.desc  : Test SendSessionTimeOutStopEvent
*/
HWTEST(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->zonesMap_.clear();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    interruptServiceTest->SetCallbackHandler(handler);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    interruptServiceTest->SendSessionTimeOutStopEvent(0, audioInterrupt, audioFocusInfoList);
    EXPECT_NE(nullptr, interruptServiceTest->handler_);
}

/**
* @tc.name  : Test SendSessionTimeOutStopEvent
* @tc.number: SendSessionTimeOutStopEvent_002
* @tc.desc  : Test SendSessionTimeOutStopEvent
*/
HWTEST(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->zonesMap_.clear();
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    interruptServiceTest->SendSessionTimeOutStopEvent(0, audioInterrupt, audioFocusInfoList);
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
}

/**
* @tc.name  : Test SendSessionTimeOutStopEvent
* @tc.number: SendSessionTimeOutStopEvent_003
* @tc.desc  : Test SendSessionTimeOutStopEvent
*/
HWTEST(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_003, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    interruptServiceTest->SendSessionTimeOutStopEvent(1, audioInterrupt, audioFocusInfoList);
    auto it = interruptServiceTest->zonesMap_.find(1);
    EXPECT_EQ(nullptr, it->second);
}

/**
* @tc.name  : Test SendSessionTimeOutStopEvent
* @tc.number: SendSessionTimeOutStopEvent_004
* @tc.desc  : Test SendSessionTimeOutStopEvent
*/
HWTEST(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    std::set<int32_t> pids = {100, 200, 300};

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->pids = pids;
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    interruptServiceTest->SendSessionTimeOutStopEvent(1, audioInterrupt, audioFocusInfoList);
    auto it = interruptServiceTest->zonesMap_.find(1);
    EXPECT_EQ(it->second->pids.find(100), it->second->pids.end());
}

/**
* @tc.name  : Test SendSessionTimeOutStopEvent
* @tc.number: SendSessionTimeOutStopEvent_005
* @tc.desc  : Test SendSessionTimeOutStopEvent
*/
HWTEST(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_005, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->zonesMap_.clear();
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->pids = {1, 2, 3};
    interruptServiceTest->zonesMap_[0] = audioInterruptZone;
    interruptServiceTest->zonesMap_[1] = audioInterruptZone;
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    audioInterrupt.pid = 1;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    interruptServiceTest->SendSessionTimeOutStopEvent(1, audioInterrupt, audioFocusInfoList);
    auto it = interruptServiceTest->zonesMap_.find(1);
    EXPECT_EQ(it->second->pids.find(1), it->second->pids.end());
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_DeactivateAudioInterruptInternal_001
 * @tc.desc  : Test DeactivateAudioInterruptInternal.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_DeactivateAudioInterruptInternal_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;
    AudioSessionService audioSessionService;
    AudioInterruptService audioInterruptService;
    AudioSessionStrategy strategy;

    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());

    bool IS_SESSION_TIMEOUT = true;
    interruptServiceTest->sessionService_ = nullptr;
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, IS_SESSION_TIMEOUT);

    interruptServiceTest->Init(GetPolicyServerTest());
    int32_t ret = interruptServiceTest->ActivateAudioSession(0, strategy);
    EXPECT_EQ(SUCCESS, ret);
    audioInterrupt.pid = 3;
    audioInterrupt.streamId = 3;
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, IS_SESSION_TIMEOUT);
    audioInterrupt.pid = 0;
    audioInterrupt.streamId = 0;
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, IS_SESSION_TIMEOUT);

    audioInterrupt.streamId = 0;
    std::pair<AudioInterrupt, AudioFocuState> pairTest = std::make_pair(audioInterrupt, ACTIVE);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.push_back(pairTest);
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_EQ(VALUE_SUCCESS, interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.back().second);

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_DeactivateAudioInterruptInternal_002
 * @tc.desc  : Test DeactivateAudioInterruptInternal.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_DeactivateAudioInterruptInternal_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;
    AudioSessionStrategy strategy;

    audioInterrupt.pid = 0;
    audioInterrupt.streamId = 0;
    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->ActivateAudioSession(0, strategy);
    interruptServiceTest->sessionService_ = nullptr;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    std::pair<AudioInterrupt, AudioFocuState> pairTest = std::make_pair(audioInterrupt, ACTIVE);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.push_back(pairTest);
    interruptServiceTest->zonesMap_.find(0)->second->pids.insert(1);

    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_FALSE(interruptServiceTest->zonesMap_.find(0)->second->pids.find(0) !=
        interruptServiceTest->zonesMap_.find(0)->second->pids.end());
    interruptServiceTest->zonesMap_.find(0)->second->pids.insert(0);
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->pids.find(0) !=
        interruptServiceTest->zonesMap_.find(0)->second->pids.end());

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_UpdateAudioSceneFromInterrupt_002
 * @tc.desc  : Test UpdateAudioSceneFromInterrupt.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_UpdateAudioSceneFromInterrupt_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();

    interruptServiceTest->Init(GetPolicyServerTest());
    AudioInterruptChangeType changeType = DEACTIVATE_AUDIO_INTERRUPT;
    interruptServiceTest->UpdateAudioSceneFromInterrupt(AUDIO_SCENE_INVALID, changeType);
    EXPECT_EQ(DEACTIVATE_AUDIO_INTERRUPT, changeType);

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SendInterruptEvent_001
 * @tc.desc  : Test SendInterruptEvent.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SendInterruptEvent_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> pairList;
    pairList.emplace_back(AudioInterrupt(), AudioFocuState::ACTIVE);
    auto it = pairList.begin();
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());

    bool removeFocusInfo;
    interruptServiceTest->SendInterruptEvent(ACTIVE, DUCK, it, removeFocusInfo);
    EXPECT_NE(interruptServiceTest->handler_, nullptr);

    interruptServiceTest->zonesMap_.clear();
}

/**
* @tc.name  : Test SendFocusChangeEvent
* @tc.number: SendFocusChangeEvent_001
* @tc.desc  : Test SendFocusChangeEvent
*/
HWTEST(AudioInterruptUnitTest, SendFocusChangeEvent_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    int32_t callbackCategory = 0;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->SendFocusChangeEvent(0, callbackCategory, audioInterrupt);
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
}

/**
* @tc.name  : Test SendFocusChangeEvent
* @tc.number: SendFocusChangeEvent_002
* @tc.desc  : Test SendFocusChangeEvent
*/
HWTEST(AudioInterruptUnitTest, SendFocusChangeEvent_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusType audioFocusTypeTest;
    AudioInterrupt audioInterrupt(STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, audioFocusTypeTest, 0);
    int32_t callbackCategory = 0;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->SendFocusChangeEvent(1, callbackCategory, audioInterrupt);
    auto it = interruptServiceTest->zonesMap_.find(1);
    EXPECT_EQ(nullptr, it->second);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCreateAudioInterruptZone_001
* @tc.desc  : Test RCreateAudioInterruptZone.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceCreateAudioInterruptZone_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(server);

    SetUid1041();
    auto retStatus = interruptServiceTest->CreateAudioInterruptZone(-1);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    SetUid1041();
    retStatus = interruptServiceTest->CreateAudioInterruptZone(0);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    SetUid1041();
    retStatus = interruptServiceTest->CreateAudioInterruptZone(2);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_001
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST(AudioInterruptUnitTest, MigrateAudioInterruptZone_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 1;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    int32_t signal = interruptServiceTest->MigrateAudioInterruptZone(0, getZoneFunc);
    EXPECT_EQ(ERR_INVALID_PARAM, signal);
    signal = interruptServiceTest->MigrateAudioInterruptZone(0, nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, signal);
}

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_003
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST(AudioInterruptUnitTest, MigrateAudioInterruptZone_003, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag)->int32_t {
        return 1;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    auto signal = interruptServiceTest->MigrateAudioInterruptZone(1, getZoneFunc);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());
    EXPECT_EQ(ERR_INVALID_PARAM, signal);
}

static void AddMovieInterruptToList(AudioFocusList &list, int32_t streamId, int32_t uid,
    AudioFocuState state, const std::string &deviceTag = "")
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = STREAM_USAGE_MOVIE;
    interrupt.audioFocusType.streamType = STREAM_MOVIE;
    interrupt.streamId = streamId;
    interrupt.uid = uid;
    interrupt.deviceTag = deviceTag;
    list.emplace_back(std::make_pair(interrupt, state));
}

static void AddMusicInterruptToList(AudioFocusList &list, int32_t streamId, int32_t uid,
    AudioFocuState state, const std::string &deviceTag = "")
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = STREAM_USAGE_MUSIC;
    interrupt.audioFocusType.streamType = STREAM_MUSIC;
    interrupt.streamId = streamId;
    interrupt.uid = uid;
    interrupt.deviceTag = deviceTag;
    list.emplace_back(std::make_pair(interrupt, state));
}

static void AddVoipInterruptToList(AudioFocusList &list, int32_t streamId, int32_t uid,
    AudioFocuState state, const std::string &deviceTag = "")
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    interrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    interrupt.streamId = streamId;
    interrupt.uid = uid;
    interrupt.deviceTag = deviceTag;
    list.emplace_back(std::make_pair(interrupt, state));
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_001
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;
    interruptServiceTest->zonesMap_.clear();
    auto ret = interruptServiceTest->InjectInterruptToAudiotZone(0, interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_002
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;

    SetUid1041();
    auto ret = interruptServiceTest->InjectInterruptToAudiotZone(2, interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    SetUid1041();
    ret = interruptServiceTest->InjectInterruptToAudiotZone(1, interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_004
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;
    interruptServiceTest->zonesMap_.clear();
    auto ret = interruptServiceTest->InjectInterruptToAudiotZone(0, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_005
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_005, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;

    SetUid1041();
    auto ret = interruptServiceTest->InjectInterruptToAudiotZone(2, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    SetUid1041();
    ret = interruptServiceTest->InjectInterruptToAudiotZone(1, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_006
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_006, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();

    SetUid1041();
    auto ret = interruptServiceTest->InjectInterruptToAudiotZone(1, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_008
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_008, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;
    AddMovieInterruptToList(focusList, 0, 1, AudioFocuState::PAUSE);
    AddVoipInterruptToList(focusList, 1, 2, AudioFocuState::ACTIVE);
    AddMusicInterruptToList(focusList, 2, 3, AudioFocuState::DUCK);

    AddMovieInterruptToList(interrupts, 0, 1, AudioFocuState::PAUSE);
    AddMusicInterruptToList(interrupts, 2, 3, AudioFocuState::ACTIVE);

    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudiotZone(0, interrupts);
    );
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 2);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_009
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_009, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;
    AddMovieInterruptToList(focusList, 0, 1, AudioFocuState::PAUSE);
    AddVoipInterruptToList(focusList, 1, 2, AudioFocuState::ACTIVE, "test");
    AddMusicInterruptToList(focusList, 2, 3, AudioFocuState::DUCK, "test");

    AddMusicInterruptToList(interrupts, 2, 3, AudioFocuState::ACTIVE, "test");

    SetUid1041();
    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudiotZone(0, "test", interrupts);
    );
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 2);
}

/**
* @tc.name  : Test InjectInterruptToAudiotZone
* @tc.number: InjectInterruptToAudiotZone_010
* @tc.desc  : Test InjectInterruptToAudiotZone
*/
HWTEST(AudioInterruptUnitTest, InjectInterruptToAudiotZone_010, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;
    AddMovieInterruptToList(focusList, 0, 1, AudioFocuState::PAUSE);
    AddVoipInterruptToList(focusList, 1, 2, AudioFocuState::ACTIVE, "test");
    AddMusicInterruptToList(focusList, 2, 3, AudioFocuState::DUCK, "test");

    AddMusicInterruptToList(interrupts, 2, 3, AudioFocuState::PLACEHOLDER, "test");
    AddMovieInterruptToList(interrupts, 3, 4, AudioFocuState::ACTIVE, "test");

    SetUid1041();
    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudiotZone(0, "test", interrupts);
    );
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 1);
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_001
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0) == interruptServiceTest->zonesMap_.end());

    interruptServiceTest->zonesMap_[0] = nullptr;
    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second == nullptr);

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    std::pair<AudioInterrupt, AudioFocuState> pairTest = std::make_pair(audioInterrupt, ACTIVE);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.push_back(pairTest);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->first.mode = SHARE_MODE;
    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.back().first.mode == SHARE_MODE);

    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->first.mode = INDEPENDENT_MODE;
    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_FALSE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.back().first.mode == SHARE_MODE);

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_002
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;
    AudioInterruptService audioInterruptService;

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    std::pair<AudioInterrupt, AudioFocuState> pairTest = std::make_pair(audioInterrupt, PAUSE);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.push_back(pairTest);
    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_FALSE(interruptServiceTest->zonesMap_.find(0)->second == nullptr);

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_003
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_003, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;
    AudioInterruptService audioInterruptService;

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    std::pair<AudioInterrupt, AudioFocuState> pairTest = std::make_pair(audioInterrupt, PLACEHOLDER);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.push_back(pairTest);
    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_FALSE(interruptServiceTest->zonesMap_.find(0)->second == nullptr);

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_004
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    std::pair<AudioInterrupt, AudioFocuState> pairTest = std::make_pair(audioInterrupt, PAUSE);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.push_back(pairTest);

    std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair = std::make_pair(
        interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->first.audioFocusType,
        interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->first.audioFocusType
    );
    interruptServiceTest->focusCfgMap_[audioFocusTypePair] = {};

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_FALSE(interruptServiceTest->zonesMap_.find(0)->second == nullptr);

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->focusCfgMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_005
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_005, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();

    interruptServiceTest->zonesMap_[0] = nullptr;
    interruptServiceTest->SimulateFocusEntry(1);
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second == nullptr);

    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->SimulateFocusEntry(1);
    AudioInterrupt interrupt;
    interruptServiceTest->zonesMap_.find(1)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt, STOP));
    EXPECT_FALSE(interruptServiceTest->zonesMap_.find(1)->second->audioFocusInfoList.empty());

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_006
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_006, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));
    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);

    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_007
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is true.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_007, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();

    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_008
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is true.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_008, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_009
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_009, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_010
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_010, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_011
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_011, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = -1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_012
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_012, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = -1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_013
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_013, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = -1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_014
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_014, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = -1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_015
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_015, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = INDEPENDENT_MODE;
    interrupt_1.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_016
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_016, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = INDEPENDENT_MODE;
    interrupt_1.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_017
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_017, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = INDEPENDENT_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_018
 * @tc.desc  : Test SimulateFocusEntry and IsSameAppInShareMode is false.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_018, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, PLACEHOLDER));

    interrupt_2.mode = INDEPENDENT_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == PLACEHOLDER);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_019
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_019, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    AudioInterrupt interrupt_1;
    AudioInterrupt interrupt_2;

    interrupt_1.mode = SHARE_MODE;
    interrupt_1.pid = 0;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_1, ACTIVE));

    interrupt_2.mode = SHARE_MODE;
    interrupt_2.pid = 1;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(
        std::make_pair(interrupt_2, PLACEHOLDER));

    interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_TRUE(interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.begin()->second == ACTIVE);
    interruptServiceTest->zonesMap_.clear();
}

/**
* @tc.name  : Test SimulateFocusEntry
* @tc.number: AudioInterruptService_SimulateFocusEntry_020
* @tc.desc  : Test SimulateFocusEntry
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_020, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();

    AudioInterrupt interruptTest;
    interruptTest.mode = INDEPENDENT_MODE;
    interruptTest.pid = 0;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = PLACEHOLDER;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    auto ret = interruptServiceTest->SimulateFocusEntry(0);
    EXPECT_EQ(PLACEHOLDER, ret.begin()->second);
}

/**
* @tc.name  : Test SimulateFocusEntry
* @tc.number: AudioInterruptService_SimulateFocusEntry_021
* @tc.desc  : Test SimulateFocusEntry
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_021, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();

    AudioInterrupt interruptTest;
    interruptTest.mode = SHARE_MODE;
    interruptTest.pid = 0;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = ACTIVE;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    auto ret = interruptServiceTest->SimulateFocusEntry(0);
    auto it = ret.begin();
    EXPECT_EQ(SHARE_MODE, it->first.mode);
    EXPECT_NE(-1, it->first.pid);
    std::advance(it, 1);
    EXPECT_EQ(SHARE_MODE, it->first.mode);
    EXPECT_NE(-1, it->first.pid);
    EXPECT_EQ(ret.begin()->first.pid, it->first.pid);
}

/**
* @tc.name  : Test SimulateFocusEntry
* @tc.number: AudioInterruptService_SimulateFocusEntry_022
* @tc.desc  : Test SimulateFocusEntry
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_022, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->focusCfgMap_.clear();


    AudioInterrupt interruptTest;
    interruptTest.mode = SHARE_MODE;
    interruptTest.pid = 0;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = ACTIVE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_VOICE_CALL;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    std::pair<AudioFocusType, AudioFocusType> focusTypePair;
    AudioFocusEntry FocusEntryTest;
    FocusEntryTest.isReject = false;
    FocusEntryTest.hintType = static_cast<InterruptHint>(INTERRUPT_HINT_ERROR);
    focusTypePair.first.streamType = STREAM_VOICE_CALL;
    focusTypePair.second.streamType = STREAM_MUSIC;
    interruptServiceTest->focusCfgMap_[focusTypePair] = FocusEntryTest;
    audioFocusTypePair.first.pid = 1;
    audioFocusTypePair.second = PAUSE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_VOICE_CALL;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    audioFocusTypePair.first.audioFocusType.streamType = STREAM_MUSIC;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    auto ret = interruptServiceTest->SimulateFocusEntry(0);
    auto it = ret.begin();
    std::advance(it, 1);
    EXPECT_EQ(PAUSE, it->second);
}

/**
* @tc.name  : Test SimulateFocusEntry
* @tc.number: AudioInterruptService_SimulateFocusEntry_023
* @tc.desc  : Test SimulateFocusEntry
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_023, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->focusCfgMap_.clear();

    AudioInterrupt interruptTest;
    interruptTest.mode = SHARE_MODE;
    interruptTest.pid = 0;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = ACTIVE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_VOICE_CALL;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    std::pair<AudioFocusType, AudioFocusType> focusTypePair;
    AudioFocusEntry FocusEntryTest;
    FocusEntryTest.isReject = false;
    FocusEntryTest.hintType = static_cast<InterruptHint>(INTERRUPT_HINT_ERROR);
    focusTypePair.first.streamType = STREAM_VOICE_CALL;
    focusTypePair.second.streamType = STREAM_MUSIC;
    interruptServiceTest->focusCfgMap_[focusTypePair] = FocusEntryTest;
    audioFocusTypePair.first.pid = 1;
    audioFocusTypePair.second = ACTIVE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_VOICE_CALL;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    interruptTest.mode = INDEPENDENT_MODE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_MUSIC;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    auto ret = interruptServiceTest->SimulateFocusEntry(0);
    auto it = ret.begin();
    std::advance(it, 1);
    EXPECT_NE(PAUSE, it->second);
    EXPECT_EQ(STREAM_VOICE_CALL, it->first.audioFocusType.streamType);
}

/**
* @tc.name  : Test SimulateFocusEntry
* @tc.number: AudioInterruptService_SimulateFocusEntry_024
* @tc.desc  : Test SimulateFocusEntry
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_024, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->focusCfgMap_.clear();


    AudioInterrupt interruptTest;
    interruptTest.mode = SHARE_MODE;
    interruptTest.pid = 0;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = ACTIVE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_VOICE_CALL;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    std::pair<AudioFocusType, AudioFocusType> focusTypePair;
    AudioFocusEntry FocusEntryTest;
    FocusEntryTest.isReject = false;
    FocusEntryTest.hintType = static_cast<InterruptHint>(INTERRUPT_HINT_ERROR);
    focusTypePair.first.streamType = STREAM_VOICE_CALL;
    focusTypePair.second.streamType = STREAM_MUSIC;
    interruptServiceTest->focusCfgMap_[focusTypePair] = FocusEntryTest;
    audioFocusTypePair.first.pid = 1;
    audioFocusTypePair.second = PLACEHOLDER;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_VOICE_CALL;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    interruptTest.mode = INDEPENDENT_MODE;
    audioFocusTypePair.first.audioFocusType.streamType = STREAM_MUSIC;
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    auto ret = interruptServiceTest->SimulateFocusEntry(0);
    auto it = ret.begin();
    std::advance(it, 1);
    EXPECT_EQ(PLACEHOLDER, it->second);
    std::advance(it, 2);
    EXPECT_NE(SHARE_MODE, it->first.mode);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceRequestAudioFocus_001
* @tc.desc  : Test RequestAudioFocus.
*/
HWTEST(AudioInterruptUnitTest, AudioInterruptServiceRequestAudioFocus_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
    EXPECT_EQ(interruptServiceTest->sessionService_, nullptr);
    interruptServiceTest->Init(server);
    AudioInterrupt incomingInterrupt;

    interruptServiceTest->clientOnFocus_ = 0;
    auto retStatus = interruptServiceTest->RequestAudioFocus(0, incomingInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);

    interruptServiceTest->clientOnFocus_ = 0;
    retStatus = interruptServiceTest->RequestAudioFocus(1, incomingInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);

    interruptServiceTest->focussedAudioInterruptInfo_ = std::make_unique<AudioInterrupt>();
    interruptServiceTest->clientOnFocus_ = 0;
    retStatus = interruptServiceTest->RequestAudioFocus(1, incomingInterrupt);
    EXPECT_EQ(retStatus, SUCCESS);
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_DeactivateAudioInterruptInternal_004
 * @tc.desc  : Test DeactivateAudioInterruptInternal.
 */
HWTEST(AudioInterruptUnitTest, AudioInterruptService_DeactivateAudioInterruptInternal_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt;
    AudioSessionService audioSessionService;
    AudioInterruptService audioInterruptService;

    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
    audioInterrupt.pid = 0;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());

    interruptServiceTest->zonesMap_.clear();
}
} // namespace AudioStandard
} // namespace OHOS