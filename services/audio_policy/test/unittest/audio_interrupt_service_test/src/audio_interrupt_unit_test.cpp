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
#include <atomic>
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
void AudioInterruptUnitTest::TearDown(void)
{
    auto &audioSessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    audioSessionService.sessionMap_.clear();
    audioSessionService.timeOutCallback_.reset();
}

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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_004, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_005, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_006, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_008, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_010, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_012, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_013, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_015, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> pairList;
    pairList.emplace_back(AudioInterrupt(), AudioFocuState::ACTIVE);
    AudioFocuState oldState{};
    AudioFocuState newState{};
    auto it = pairList.begin();
    bool removeFocusInfo = true;
    interruptServiceTest->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_017, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_018, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    EXPECT_NO_THROW(
        interruptServiceTest->WriteServiceStartupError();
    );
}

/**
* @tc.name  : Test WriteCallSessionEvent.
* @tc.number: WriteCallSessionEventTest
* @tc.desc  : Test WriteCallSessionEvent.
*/
HWTEST_F(AudioInterruptUnitTest, WriteCallSessionEventTest, TestSize.Level1)
{
    int32_t value = 1;
    auto interruptServiceTest = GetTnterruptServiceTest();
    EXPECT_NO_THROW(
        interruptServiceTest->WriteCallSessionEvent(value);
    );
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_019
* @tc.desc  : Test SendFocusChangeEvent.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_019, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_021, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    sptr<AudioPolicyServer> server = nullptr;
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0, 1);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->handler_ = nullptr;

    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0, 1);
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0, 1);
    AudioInterrupt a1, a2, a3, a4, a5, a6;
    a1.streamUsage = StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    a2.streamUsage = StreamUsage::STREAM_USAGE_VOICE_RINGTONE;
    a3.streamUsage = StreamUsage::STREAM_USAGE_UNKNOWN;
    a4.streamUsage = StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION;
    a5.audioFocusType.sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    a6.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a2, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a3, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a4, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a5, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a6, AudioFocuState::ACTIVE});
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 6);
    interruptServiceTest->ClearAudioFocusInfoListOnAccountsChanged(0, 1);
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 2);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_022
* @tc.desc  : Test ResetNonInterruptControl.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_022, TestSize.Level1)
{
    uint32_t sessionId = CLIENT_TYPE_OTHERS;
    auto interruptServiceTest = GetTnterruptServiceTest();

    interruptServiceTest->Init(GetPolicyServerTest());
    AudioInterrupt interrupt;
    EXPECT_NO_THROW(
        interruptServiceTest->ResetNonInterruptControl(interrupt);
    );

    interrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    EXPECT_NO_THROW(
        interruptServiceTest->ResetNonInterruptControl(interrupt);
    );

    interrupt.streamId = 2;
    EXPECT_NO_THROW(
        interruptServiceTest->ResetNonInterruptControl(interrupt);
    );
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_023
* @tc.desc  : Test CreateAudioInterruptZone.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_023, TestSize.Level1)
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
    AudioZoneContext context;

    int32_t ret = interruptServiceTest->CreateAudioInterruptZone(zoneId, context);
    EXPECT_EQ(VALUE_ERROR, ret);

    zoneId = 0;
    ret = interruptServiceTest->CreateAudioInterruptZone(zoneId, context);
    EXPECT_EQ(VALUE_ERROR, ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_024
* @tc.desc  : Test ReleaseAudioInterruptZone.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_024, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 1;

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_025, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t zoneId = 0;

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_027, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_028, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_029, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    const sptr<IStandardAudioService> result = interruptServiceTest->GetAudioServerProxy();
    EXPECT_NE(result, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_030
* @tc.desc  : Test OnSessionTimeout.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_030, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = 2;
    interruptServiceTest->OnSessionTimeout(audioInterrupt.pid);
    EXPECT_NE(interruptServiceTest, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_031
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_031, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    AudioSessionStrategy strategy;
    int32_t result = interruptServiceTest->ActivateAudioSession(0, CALLER_PID, strategy);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_032
* @tc.desc  : Test AddActiveInterruptToSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_032, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    interruptServiceTest->AddActiveInterruptToSession(CALLER_PID);
    EXPECT_NE(interruptServiceTest, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_033
* @tc.desc  : Test DeactivateAudioSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_033, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    int32_t result = interruptServiceTest->DeactivateAudioSession(0, CALLER_PID);
    EXPECT_EQ(ERR_ILLEGAL_STATE, result);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_034
* @tc.desc  : Test CanMixForSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_034, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_035, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    bool result = interruptServiceTest->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_036
* @tc.desc  : Test CanMixForActiveSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_036, TestSize.Level1)
{
    auto interruptServiceTest = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    bool result = interruptServiceTest->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptService_037
* @tc.desc  : Test RequestAudioFocus.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_037, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t clientId = interruptServiceTest->clientOnFocus_;
    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_038, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    int32_t clientId = interruptServiceTest->clientOnFocus_;
    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_039, TestSize.Level1)
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
* @tc.number: AudioInterruptService_040
* @tc.desc  : Test ClearAudioFocusBySessionID.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_040, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    sptr<AudioPolicyServer> server = nullptr;
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->ClearAudioFocusBySessionID(0);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();

    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);
    interruptServiceTest->ClearAudioFocusBySessionID(0);
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);
    interruptServiceTest->ClearAudioFocusBySessionID(0);
    AudioInterrupt a1;
    a1.streamId = 1;
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 1);
    interruptServiceTest->ClearAudioFocusBySessionID(0);
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 1);
    interruptServiceTest->ClearAudioFocusBySessionID(-1);
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 1);
    interruptServiceTest->ClearAudioFocusBySessionID(1);
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_002
* @tc.desc  : Test CanMixForIncomingSession. incomingInterrupt.pid is -1
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_002, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    int32_t ret = interruptService->ActivateAudioSession(0, incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_FALSE(interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_004
* @tc.desc  : Test CanMixForIncomingSession. IsIncomingStreamLowPriority(focusEntry) is true.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_004, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    int32_t ret = interruptService->ActivateAudioSession(0, incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    focusEntry.isReject = true;
    EXPECT_FALSE(interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForIncomingSession_005
* @tc.desc  : Test CanMixForIncomingSession. IsIncomingStreamLowPriority(focusEntry) is false.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForIncomingSession_005, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    int32_t ret = interruptService->ActivateAudioSession(0, incomingInterrupt.pid, strategyTest);
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_001, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_RemoveExistingFocus_001
 * @tc.desc  : Test RemoveExistingFocus.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_RemoveExistingFocus_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    int32_t appUid = 1;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt1;
    audioInterrupt1.streamId = 100;
    audioInterrupt1.uid = 1;
    std::unordered_set<int32_t> uidActivedSessions;
    audioInterruptZone->audioFocusInfoList.clear();
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt1, AudioFocuState{PAUSE});
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = audioInterruptZone;
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->RemoveExistingFocus(appUid, uidActivedSessions);
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_ResumeFocusByStreamId_001
 * @tc.desc  : Test ResumeFocusByStreamId.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_ResumeFocusByStreamId_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    InterruptEventInternal interruptEvent;
    interruptServiceTest->ResumeFocusByStreamId(0, interruptEvent);
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto retStatus = interruptServiceTest->SetAudioInterruptCallback(0, 0,
        sptr<RemoteObjectTestStub>::MakeSptr(), 0);
    EXPECT_EQ(retStatus, SUCCESS);
    interruptServiceTest->ResumeFocusByStreamId(0, interruptEvent);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_002
* @tc.desc  : Test CanMixForSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_002, TestSize.Level1)
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
    int32_t ret = interruptService->ActivateAudioSession(0, incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    ret = interruptService->ActivateAudioSession(0, incomingInterrupt.pid, strategyTest, true);
    EXPECT_EQ(SUCCESS, ret);
    ret = interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_003
* @tc.desc  : Test CanMixForSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    int32_t ret = interruptService->ActivateAudioSession(0, incomingInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    focusEntry.actionOn = CURRENT;
    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    ret = interruptService->ActivateAudioSession(0, activeInterrupt.pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    ret = interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test AudioInterruptUnitTest.
 * @tc.number: SetAppConcurrencyMode_001
 * @tc.desc  : Test SetAppConcurrencyMode.
 */
HWTEST_F(AudioInterruptUnitTest, SetAppConcurrencyMode_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerTest();
    ASSERT_TRUE(server != nullptr);
    int32_t uid = 0;
    int32_t mode = 0;
    auto ret = server->SetAppConcurrencyMode(uid, mode);
    EXPECT_EQ(ret, SUCCESS);
    server->interruptService_ = nullptr;
    ret = server->SetAppConcurrencyMode(uid, mode);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    EXPECT_EQ(ret, ERR_UNKNOWN);
}
 
/**
 * @tc.name  : Test AudioInterruptUnitTest.
 * @tc.number: SetAppSilentOnDisplay_001
 * @tc.desc  : Test SetAppSilentOnDisplay.
 */
HWTEST_F(AudioInterruptUnitTest, SetAppSilentOnDisplay_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerTest();
    ASSERT_TRUE(server != nullptr);
    int32_t displayId = 1;
    auto ret = server->SetAppSilentOnDisplay(displayId);
    EXPECT_EQ(ret, SUCCESS);
    server->interruptService_ = nullptr;
    ret = server->SetAppSilentOnDisplay(displayId);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    EXPECT_EQ(ret, ERR_UNKNOWN);
}
 
/**
* @tc.name  : Test AudioInterruptUnitTest.
* @tc.number: AudioPolicyServer_009
* @tc.desc  : Test DeactivateAudioInterrupt.
*/
HWTEST_F(AudioInterruptUnitTest, AudioPolicyServer_001, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
 
    AudioInterrupt audioInterrupt;
    int32_t zoneID = 456;
    pid_t pid = 1;
    pid_t uid = 1;
    int32_t strategy = 0;
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    int32_t result = server->DeactivateAudioInterrupt(audioInterrupt, zoneID);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(SUCCESS, policyServerTest->ActivateAudioSession(strategy));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCanMixForSession_004
* @tc.desc  : Test CanMixForSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForSession_004, TestSize.Level1)
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
* @tc.number: AudioInterruptServiceRemovePlaceholderInterruptForSession_002
* @tc.desc  : Test RemovePlaceholderInterruptForSession. About itZone.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceRemovePlaceholderInterruptForSession_002, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t pid = CALLER_PID;
    bool timeOut = IS_SESSION_TIMEOUT;
    interruptService->zonesMap_.find(DEFAULT_ZONE_ID)->second = nullptr;
    interruptService->RemovePlaceholderInterruptForSession(pid, timeOut);

    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceRemovePlaceholderInterruptForSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);

    bool timeOut = IS_SESSION_TIMEOUT;
    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceDeactivateAudioSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    int32_t pid = CALLER_PID;
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t ret = interruptService->DeactivateAudioSession(0, pid);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    strategyTest.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    ret = interruptService->ActivateAudioSession(0, pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    ret = interruptService->DeactivateAudioSession(0, pid);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_001
* @tc.desc  : Test AddActiveInterruptToSession.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_001, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    int32_t pid = CALLER_PID;
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    interruptService->AddActiveInterruptToSession(pid);
    EXPECT_FALSE(interruptService->sessionService_.IsAudioSessionActivated(pid));
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceAddActiveInterruptToSession_002
* @tc.desc  : Test AddActiveInterruptToSession. About itZone.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_002, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t pid = CALLER_PID;
    int32_t ret = interruptService->ActivateAudioSession(0, pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
    interruptService->zonesMap_.find(DEFAULT_ZONE_ID)->second = nullptr;
    interruptService->AddActiveInterruptToSession(pid);

    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_003, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    int32_t pid = CALLER_PID;
    int32_t ret = interruptService->ActivateAudioSession(0, pid, strategyTest);
    EXPECT_EQ(SUCCESS, ret);

    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_004, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    interruptService->zonesMap_.clear();
    int32_t pid = CALLER_PID;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = CALLER_PID;
    audioInterrupt.pid = CALLER_PID;
    interruptService->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptService->zonesMap_[0]->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, ACTIVE));
    int32_t ret = interruptService->ActivateAudioSession(0, pid, strategyTest);
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceAddActiveInterruptToSession_005, TestSize.Level1)
{
    auto interruptService = GetTnterruptServiceTest();
    auto server = GetPolicyServerTest();
    interruptService->Init(server);
    interruptService->zonesMap_.clear();
    int32_t pid = CALLER_PID;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = CALLER_PID;
    audioInterrupt.pid = CALLER_PID;
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    interruptService->zonesMap_[0] = audioInterruptZone;
    int32_t ret = interruptService->ActivateAudioSession(0, pid, strategyTest);
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceGetAudioServerProxy_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    sptr<IStandardAudioService> ret = audioInterruptService->GetAudioServerProxy();
    EXPECT_NE(nullptr, ret);
}

/**
 * @tc.name  : Test GetAudioServerProxy API.
 * @tc.number: AudioInterruptServiceGetAudioServerProxy_002
 * @tc.desc  : Test OnSessionTimeout when g_adProxy is not nullptr.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceGetAudioServerProxy_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    sptr<IStandardAudioService> ret = audioInterruptService->GetAudioServerProxy();
    ret = audioInterruptService->GetAudioServerProxy();
    EXPECT_NE(nullptr, ret);
}

/**
 * @tc.name  : Test OnSessionTimeout API.
 * @tc.number: AudioInterruptServiceOnSessionTimeout_001
 * @tc.desc  : Test normal OnSessionTimeout.
 *             Test normal HandleSessionTimeOutEvent.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceOnSessionTimeout_001, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceOnSessionTimeout_002, TestSize.Level1)
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
 * @tc.number: AudioInterruptServiceActivateAudioSession_002
 * @tc.desc  : Test normal ActivateAudioSession.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceActivateAudioSession_002, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    int32_t ret =  audioInterruptService->ActivateAudioSession(0, CALLER_PID_TEST, strategyTest);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceUnsetAudioManagerInterruptCallback_001
* @tc.desc  : Test UnsetAudioManagerInterruptCallback.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceUnsetAudioManagerInterruptCallback_001, TestSize.Level1)
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
 * @tc.desc  : Test IsAudioSessionActivated.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceIsAudioSessionActivated_001, TestSize.Level1)
{
    audioInterruptService = GetTnterruptServiceTest();
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(nullptr, audioInterruptService);

    audioInterruptService->Init(serverTest);
    int32_t PIT_TEST { -1 };
    bool ret = audioInterruptService->IsAudioSessionActivated(PIT_TEST);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_001
 * @tc.desc  : Test CanMixForActiveSession when return true.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_001, TestSize.Level1)
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
    audioInterruptService->sessionService_.ActivateAudioSession(0, strategy);
    activeInterrupt.pid = { 0 };
    focusEntry.actionOn = INCOMING;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_003
 * @tc.desc  : Test CanMixForActiveSession when IsAudioSessionActivated is true.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_003, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_004, TestSize.Level1)
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
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, audioSessionStateMonitor_);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_.sessionMap_.insert({0, audioSession});
    std::shared_ptr<AudioSession> activeSession =
        audioInterruptService->sessionService_.sessionMap_[activeInterrupt.pid];
    activeSession->strategy_.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CanMixForActiveSession API.
 * @tc.number: AudioInterruptServiceCanMixForActiveSession_005
 * @tc.desc  : Test CanMixForActiveSession when IsActiveStreamLowPriority is true and return false.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_005, TestSize.Level1)
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
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, audioSessionStateMonitor_);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_.sessionMap_.insert({0, audioSession});
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCanMixForActiveSession_006, TestSize.Level1)
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
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(0, strategy, audioSessionStateMonitor_);
    ASSERT_NE(nullptr, audioSession);
    activeInterrupt.pid = { 0 };
    audioInterruptService->sessionService_.sessionMap_.insert({0, audioSession});
    incomingInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    bool ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_FALSE(ret);
    audioInterruptService->sessionService_.sessionMap_[0]->isSystemApp_ = true;
    audioInterruptService->sessionService_.sessionMap_[0]->state_ = AudioSessionState::SESSION_ACTIVE;
    ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test SendSessionTimeOutStopEvent
* @tc.number: SendSessionTimeOutStopEvent_001
* @tc.desc  : Test SendSessionTimeOutStopEvent
*/
HWTEST_F(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_001, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_002, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_003, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_004, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, SendSessionTimeOutStopEvent_005, TestSize.Level1)
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
    EXPECT_NE(it->second->pids.find(1), it->second->pids.end());
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_DeactivateAudioInterruptInternal_001
 * @tc.desc  : Test DeactivateAudioInterruptInternal.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_DeactivateAudioInterruptInternal_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};
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
    interruptServiceTest->Init(GetPolicyServerTest());
    int32_t ret = interruptServiceTest->ActivateAudioSession(0, 0, strategy);
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_DeactivateAudioInterruptInternal_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};
    AudioSessionStrategy strategy;

    audioInterrupt.pid = 0;
    audioInterrupt.streamId = 0;
    interruptServiceTest->Init(GetPolicyServerTest());
    interruptServiceTest->ActivateAudioSession(0, 0, strategy);
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_UpdateAudioSceneFromInterrupt_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    ASSERT_TRUE(interruptServiceTest != nullptr);
    interruptServiceTest->zonesMap_.clear();

    interruptServiceTest->Init(GetPolicyServerTest());
    AudioInterruptChangeType changeType = DEACTIVATE_AUDIO_INTERRUPT;
    interruptServiceTest->UpdateAudioSceneFromInterrupt(AUDIO_SCENE_INVALID, changeType);
    interruptServiceTest->zonesMap_.clear();
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_UpdateAudioSceneFromInterrupt_003
 * @tc.desc  : Test UpdateAudioSceneFromInterrupt.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_UpdateAudioSceneFromInterrupt_003, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);
    int32_t ownerUid_ = 20020190;
    int32_t formerUid_ = 20020191;
 
    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();
    audioInterruptService->formerUid_.store(formerUid_);
    audioInterruptService->ownerUid_ = ownerUid_;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    AudioInterruptChangeType changeType = ACTIVATE_AUDIO_INTERRUPT;
 
    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}
 
/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_UpdateAudioSceneFromInterrupt_004
 * @tc.desc  : Test UpdateAudioSceneFromInterrupt.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_UpdateAudioSceneFromInterrupt_004, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);
    int32_t ownerUid_ = 20020190;
    int32_t formerUid_ = 20020190;
 
    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();
    audioInterruptService->formerUid_.store(formerUid_);
    audioInterruptService->ownerUid_ = ownerUid_;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    AudioInterruptChangeType changeType = ACTIVATE_AUDIO_INTERRUPT;
 
    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}
 
/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_UpdateAudioSceneFromInterrupt_005
 * @tc.desc  : Test UpdateAudioSceneFromInterrupt.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_UpdateAudioSceneFromInterrupt_005, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);
    int32_t ownerUid_ = 20020190;
    int32_t formerUid_ = 20020191;
 
    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();
    audioInterruptService->formerUid_.store(formerUid_);
    audioInterruptService->ownerUid_ = ownerUid_;
    AudioScene audioScene = AUDIO_SCENE_CALL_START;
    AudioInterruptChangeType changeType = ACTIVATE_AUDIO_INTERRUPT;
 
    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}
 
/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_UpdateAudioSceneFromInterrupt_006
 * @tc.desc  : Test UpdateAudioSceneFromInterrupt.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_UpdateAudioSceneFromInterrupt_006, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);
    int32_t ownerUid_ = 20020190;
    int32_t formerUid_ = 20020190;
 
    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();
    audioInterruptService->formerUid_.store(formerUid_);
    audioInterruptService->ownerUid_ = ownerUid_;
    AudioScene audioScene = AUDIO_SCENE_CALL_START;
    AudioInterruptChangeType changeType = ACTIVATE_AUDIO_INTERRUPT;
 
    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SendInterruptEvent_001
 * @tc.desc  : Test SendInterruptEvent.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SendInterruptEvent_001, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, SendFocusChangeEvent_001, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, SendFocusChangeEvent_002, TestSize.Level1)
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
* @tc.name  : Test SendActiveVolumeTypeChangeEvent
* @tc.number: SendActiveVolumeTypeChangeEvent_001
* @tc.desc  : Test SendActiveVolumeTypeChangeEvent
*/
HWTEST_F(AudioInterruptUnitTest, SendActiveVolumeTypeChangeEvent_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());
    EXPECT_NE(interruptServiceTest->handler_, nullptr);

    interruptServiceTest->zonesMap_.clear();
    int32_t zoneId = 0;
    interruptServiceTest->SendActiveVolumeTypeChangeEvent(zoneId);
    EXPECT_EQ(STREAM_MUSIC, interruptServiceTest->activeStreamType_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptServiceReleaseAudioInterruptZone_001
* @tc.desc  : Test ReleaseAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceReleaseAudioInterruptZone_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };

    auto retStatus = interruptServiceTest->ReleaseAudioInterruptZone(-1, getZoneFunc);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    SetUid1041();
    retStatus = interruptServiceTest->ReleaseAudioInterruptZone(-1, getZoneFunc);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    SetUid1041();
    retStatus = interruptServiceTest->ReleaseAudioInterruptZone(0, getZoneFunc);
    EXPECT_EQ(ERR_INVALID_PARAM, retStatus);

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();

    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    retStatus = interruptServiceTest->ReleaseAudioInterruptZone(1, getZoneFunc);
    EXPECT_EQ(ERR_INVALID_PARAM, retStatus);
}

/**
* @tc.name  : Test AudioInterruptService.
* @tc.number: AudioInterruptServiceCreateAudioInterruptZone_001
* @tc.desc  : Test RCreateAudioInterruptZone.
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceCreateAudioInterruptZone_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->Init(server);
    AudioZoneContext context;

    SetUid1041();
    auto retStatus = interruptServiceTest->CreateAudioInterruptZone(-1, context);
    EXPECT_EQ(retStatus, ERR_INVALID_PARAM);

    SetUid1041();
    retStatus = interruptServiceTest->CreateAudioInterruptZone(0, context);
    EXPECT_EQ(ERR_INVALID_PARAM, retStatus);

    SetUid1041();
    retStatus = interruptServiceTest->CreateAudioInterruptZone(2, context);
    EXPECT_EQ(ERR_INVALID_PARAM, retStatus);
}

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_001
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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
* @tc.number: MigrateAudioInterruptZone_002
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_002, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 1;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    auto signal = interruptServiceTest->MigrateAudioInterruptZone(0, getZoneFunc);
    auto it = interruptServiceTest->zonesMap_.find(1);
    EXPECT_NE(nullptr, it->second);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());
    EXPECT_EQ(SUCCESS, signal);
}

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_003
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_003, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
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

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_004
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_004, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 1;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    int32_t signal = interruptServiceTest->MigrateAudioInterruptZone(1, getZoneFunc);
    EXPECT_EQ(SUCCESS, signal);
}

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_005
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_005, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 1;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[2] = std::make_shared<AudioInterruptZone>();
    auto ret = interruptServiceTest->MigrateAudioInterruptZone(1, getZoneFunc);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());
    EXPECT_EQ(SUCCESS, ret);
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
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_006
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_006, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->coreService_ = AudioCoreService::GetCoreService();
    server->coreService_->Init();
    server->eventEntry_ = server->coreService_->GetEventEntry();
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        if (uid == 2) {
            return 1;
        }
        return 0;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;
    AddMovieInterruptToList(focusList, 0, 1, AudioFocuState::PAUSE);
    AddVoipInterruptToList(focusList, 1, 2, AudioFocuState::ACTIVE);
    AddMusicInterruptToList(focusList, 2, 3, AudioFocuState::DUCK);

    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    EXPECT_NO_THROW(
        interruptServiceTest->MigrateAudioInterruptZone(0, getZoneFunc);
    );
    EXPECT_NE(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 1);
    EXPECT_EQ(interruptServiceTest->zonesMap_[1]->audioFocusInfoList.size(), 1);
}

/**
* @tc.name  : Test MigrateAudioInterruptZone
* @tc.number: MigrateAudioInterruptZone_007
* @tc.desc  : Test MigrateAudioInterruptZone
*/
HWTEST_F(AudioInterruptUnitTest, MigrateAudioInterruptZone_007, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 1;
    };

    SetUid1041();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;
    AddMusicInterruptToList(focusList, 0, 1, AudioFocuState::DUCK);

    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();
    EXPECT_NO_THROW(
        interruptServiceTest->MigrateAudioInterruptZone(0, getZoneFunc);
    );
    EXPECT_NE(interruptServiceTest->zonesMap_.find(1), interruptServiceTest->zonesMap_.end());
    EXPECT_EQ(interruptServiceTest->zonesMap_[1]->audioFocusInfoList.size(), 1);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_001
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;
    interruptServiceTest->zonesMap_.clear();
    auto ret = interruptServiceTest->InjectInterruptToAudioZone(0, interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_002
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;

    SetUid1041();
    auto ret = interruptServiceTest->InjectInterruptToAudioZone(2, interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    SetUid1041();
    ret = interruptServiceTest->InjectInterruptToAudioZone(1, interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_003
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_003, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();

    SetUid1041();
    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudioZone(1, interrupts);
    );
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_004
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;
    interruptServiceTest->zonesMap_.clear();
    auto ret = interruptServiceTest->InjectInterruptToAudioZone(0, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_005
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_005, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = nullptr;

    SetUid1041();
    auto ret = interruptServiceTest->InjectInterruptToAudioZone(2, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    SetUid1041();
    ret = interruptServiceTest->InjectInterruptToAudioZone(1, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_006
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_006, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    AudioFocusList interrupts;

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();

    SetUid1041();
    auto ret = interruptServiceTest->InjectInterruptToAudioZone(1, "", interrupts);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_007
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_007, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->coreService_ = AudioCoreService::GetCoreService();
    server->coreService_->Init();
    server->eventEntry_ = server->coreService_->GetEventEntry();
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_[1] = std::make_shared<AudioInterruptZone>();

    SetUid1041();
    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudioZone(1, "1", interrupts);
    );
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_008
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_008, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

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
        interruptServiceTest->InjectInterruptToAudioZone(0, interrupts);
    );
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 3);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_009
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_009, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;
    AddMovieInterruptToList(focusList, 0, 1, AudioFocuState::PAUSE);
    AddVoipInterruptToList(focusList, 1, 2, AudioFocuState::ACTIVE, "test");
    AddMusicInterruptToList(focusList, 2, 3, AudioFocuState::DUCK, "test");

    AddMusicInterruptToList(interrupts, 2, 3, AudioFocuState::ACTIVE, "test");

    SetUid1041();
    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudioZone(0, "1", interrupts);
    );
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 3);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_010
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_010, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

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
        interruptServiceTest->InjectInterruptToAudioZone(0, "1", interrupts);
    );
    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), 3);
}

/**
* @tc.name  : Test InjectInterruptToAudioZone
* @tc.number: InjectInterruptToAudioZone_011
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST_F(AudioInterruptUnitTest, InjectInterruptToAudioZone_011, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = new (std::nothrow) AudioPolicyServer(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->interruptService_->Init(server);
    auto interruptServiceTest = server->interruptService_;
    AudioFocusList interrupts;
    auto coreService = std::make_shared<AudioCoreService>();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    server->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    auto &focusList = interruptServiceTest->zonesMap_[0]->audioFocusInfoList;

    AudioInterrupt interrupt;
    interrupt.streamUsage = STREAM_USAGE_MUSIC;
    interrupt.audioFocusType.streamType = STREAM_MUSIC;
    interrupt.streamId = 100100;
    interrupt.uid = 100;
    interrupt.deviceTag = "ABCDEFG";
    focusList.emplace_back(std::make_pair(interrupt, PLACEHOLDER));
    interrupts.emplace_back(std::make_pair(interrupt, ACTIVE));

    SetUid1041();
    EXPECT_NO_THROW(
        interruptServiceTest->InjectInterruptToAudioZone(0, "ABCDEFG", interrupts);
    );
    EXPECT_NE(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.size(), -1);
}

/**
 * @tc.name  : Test AudioInterruptService.
 * @tc.number: AudioInterruptService_SimulateFocusEntry_001
 * @tc.desc  : Test SimulateFocusEntry.
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_001, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};

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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_002, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_003, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};

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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_005, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_006, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_007, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_008, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_009, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_010, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_011, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_012, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_013, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_014, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_015, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_016, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_017, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_018, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_019, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_020, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_021, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_022, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_023, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_SimulateFocusEntry_024, TestSize.Level1)
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptServiceRequestAudioFocus_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = nullptr;
    auto interruptServiceTest = GetTnterruptServiceTest();
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
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_DeactivateAudioInterruptInternal_004, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    AudioInterrupt audioInterrupt = {};
    AudioInterruptService audioInterruptService;

    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_EQ(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
    audioInterrupt.pid = 0;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->DeactivateAudioInterruptInternal(0, audioInterrupt, true);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());

    interruptServiceTest->zonesMap_.clear();
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_GetAppState_001
* @tc.desc  : Test GetAppState
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_GetAppState_001, TestSize.Level1)
{
    auto server = GetPolicyServerTest();
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    int32_t appPid = -1;

    uint8_t ret = interruptServiceTest->GetAppState(appPid);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_WriteStartDfxMsg_001
* @tc.desc  : Test WriteStartDfxMsg
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_WriteStartDfxMsg_001, TestSize.Level1)
{
    auto server = GetPolicyServerTest();
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->Init(server);

    InterruptDfxBuilder dfxBuilder;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.state == State::PREPARED;
    audioInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterrupt.audioFocusType.isPlay = false;

    interruptServiceTest->WriteStartDfxMsg(dfxBuilder, audioInterrupt);
    EXPECT_NE(interruptServiceTest->dfxCollector_, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_WriteSessionTimeoutDfxEvent_001
* @tc.desc  : Test WriteSessionTimeoutDfxEvent
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_WriteSessionTimeoutDfxEvent_001, TestSize.Level1)
{
    auto server = GetPolicyServerTest();
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->Init(server);

    int32_t pid = 1001;
    AudioInterrupt interruptTest;
    interruptTest.mode = SHARE_MODE;
    interruptTest.pid = 0;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = ACTIVE;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    interruptServiceTest->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_WriteStopDfxMsg_001
* @tc.desc  : Test WriteStopDfxMsg
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_WriteStopDfxMsg_001, TestSize.Level1)
{
    auto server = GetPolicyServerTest();
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->Init(server);

    int32_t pid = 1001;
    AudioInterrupt interruptTest;
    interruptTest.mode = SHARE_MODE;
    interruptTest.pid = 0;
    interruptTest.state = State::RELEASED;
    std::pair<AudioInterrupt, AudioFocuState> audioFocusTypePair;
    audioFocusTypePair.first = interruptTest;
    audioFocusTypePair.second = ACTIVE;
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->zonesMap_.find(0)->second->audioFocusInfoList.emplace_back(audioFocusTypePair);

    interruptServiceTest->WriteStopDfxMsg(interruptTest);
    EXPECT_NE(interruptServiceTest->zonesMap_.find(0), interruptServiceTest->zonesMap_.end());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_AudioSessionInfoDump_001
* @tc.desc  : Test AudioSessionInfoDump
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_AudioSessionInfoDump_001, TestSize.Level1)
{
    auto server = GetPolicyServerTest();
    auto interruptServiceTest = GetTnterruptServiceTest();
    interruptServiceTest->zonesMap_.clear();
    std::string dumpString = "test dump string";
    interruptServiceTest->Init(server);
    interruptServiceTest->sessionService_.AudioSessionInfoDump(dumpString);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_101
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_101, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_102
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_102, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_103
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_103, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_104
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_104, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    incomingInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_105
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_105, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    incomingInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_106
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_106, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_107
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_107, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;

    auto ret = audioInterruptService->IsCanMixInterrupt(incomingInterrupt, activeInterrupt);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_108
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_108, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    AudioInterruptChangeType changeType = ACTIVATE_AUDIO_INTERRUPT;

    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_109
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_109, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    audioInterruptService->policyServer_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);

    AudioScene audioScene = AUDIO_SCENE_PHONE_CALL;
    AudioInterruptChangeType changeType = DEACTIVATE_AUDIO_INTERRUPT;

    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_110
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_110, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    audioInterruptService->policyServer_->SetAudioScene(AUDIO_SCENE_PHONE_CALL);
    EXPECT_NE(audioInterruptService->policyServer_, nullptr);

    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    AudioInterruptChangeType changeType = DEACTIVATE_AUDIO_INTERRUPT;

    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_111
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_111, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    int32_t systemAbilityId = 0;
    audioInterruptService->policyServer_ = new AudioPolicyServer(systemAbilityId);
    auto coreService = std::make_shared<AudioCoreService>();
    audioInterruptService->policyServer_->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);
    audioInterruptService->policyServer_->eventEntry_->coreService_ = std::make_shared<AudioCoreService>();

    EXPECT_NE(audioInterruptService->policyServer_, nullptr);

    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    AudioInterruptChangeType changeType = static_cast<AudioInterruptChangeType>(3);

    audioInterruptService->UpdateAudioSceneFromInterrupt(audioScene, changeType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_112
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_112, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incoming;
    AudioInterrupt inprocessing;
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    bool bConcurrency = true;

    auto ret = audioInterruptService->EvaluateWhetherContinue(incoming, inprocessing, focusEntry, bConcurrency);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_113
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_113, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incoming;
    AudioInterrupt inprocessing;
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    bool bConcurrency = false;

    auto ret = audioInterruptService->EvaluateWhetherContinue(incoming, inprocessing, focusEntry, bConcurrency);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_114
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_114, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    AudioInterrupt incoming;
    AudioInterrupt inprocessing;
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_NONE;
    bool bConcurrency = false;

    auto ret = audioInterruptService->EvaluateWhetherContinue(incoming, inprocessing, focusEntry, bConcurrency);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_115
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_115, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    InterruptEventInternal interruptEvent;
    uint32_t streamId = 0;
    AudioInterrupt audioInterrupt = {};
    audioInterruptService->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    EXPECT_NE(audioInterruptService->dfxCollector_, nullptr);

    audioInterruptService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioInterruptService->handler_, nullptr);

    audioInterruptService->SendInterruptEventCallback(interruptEvent, streamId, audioInterrupt);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_116
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_116, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    InterruptEventInternal interruptEvent;
    uint32_t streamId = 0;
    AudioInterrupt audioInterrupt = {};
    audioInterruptService->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    EXPECT_NE(audioInterruptService->dfxCollector_, nullptr);

    audioInterruptService->handler_ = nullptr;

    audioInterruptService->SendInterruptEventCallback(interruptEvent, streamId, audioInterrupt);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_117
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_117, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> myList;
    myList.emplace_back(AudioInterrupt(), AudioFocuState::PAUSEDBYREMOTE);
    auto iterActive = myList.begin();

    AudioFocuState oldState = PAUSEDBYREMOTE;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> myList2;
    myList.emplace_back(AudioInterrupt(), AudioFocuState::PAUSEDBYREMOTE);
    auto iterNew = myList.begin();

    auto ret = audioInterruptService->IsHandleIter(iterActive, oldState, iterNew);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_118
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_118, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    EXPECT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> myList;
    myList.emplace_back(AudioInterrupt(), AudioFocuState::PAUSEDBYREMOTE);
    auto iterActive = myList.begin();

    AudioFocuState oldState = ACTIVE;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> myList2;
    myList.emplace_back(AudioInterrupt(), AudioFocuState::PAUSEDBYREMOTE);
    auto iterNew = myList.begin();

    auto ret = audioInterruptService->IsHandleIter(iterActive, oldState, iterNew);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: AudioInterruptService_119
 * @tc.desc  : Test AudioInterruptService
 */
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_119, TestSize.Level1)
{
    auto interruptServiceTest = GetTnterruptServiceTest();
    ASSERT_NE(interruptServiceTest, nullptr);

    interruptServiceTest->zonesMap_.clear();
    interruptServiceTest->zonesMap_[0] = std::make_shared<AudioInterruptZone>();
    interruptServiceTest->SetCallbackHandler(GetServerHandlerTest());

    AudioInterrupt audioInterrupt = {};
    int32_t ret1 = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(ret1, SUCCESS);

    AudioInterrupt a1, a2, a3;
    a1.streamUsage = StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    a2.streamUsage = StreamUsage::STREAM_USAGE_VOICE_RINGTONE;
    a3.streamUsage = StreamUsage::STREAM_USAGE_UNKNOWN;
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a2, AudioFocuState::ACTIVE});
    interruptServiceTest->zonesMap_[0]->audioFocusInfoList.push_back({a3, AudioFocuState::ACTIVE});

    int32_t ret2 = interruptServiceTest->ActivatePreemptMode();
    EXPECT_EQ(ret2, SUCCESS);

    EXPECT_EQ(interruptServiceTest->zonesMap_[0]->audioFocusInfoList.empty(), true);

    ret1 = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(ret1, ERR_FOCUS_DENIED);

    ret2 = interruptServiceTest->DeactivatePreemptMode();
    EXPECT_EQ(ret2, SUCCESS);
    ret1 = interruptServiceTest->ActivateAudioInterrupt(0, audioInterrupt);
    EXPECT_EQ(ret1, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_120
* @tc.desc  : Test GetAudioInterruptBundleName_01
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_120, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.uid = 1013;
    audioInterrupt.bundleName.clear();
    audioInterruptService->GetAudioInterruptBundleName(audioInterrupt);
    EXPECT_TRUE(audioInterrupt.bundleName.empty());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_121
* @tc.desc  : Test GetAudioInterruptBundleName_02
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_121, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt = {};
    std::string str = "xyz";
    audioInterrupt.bundleName = str;
    audioInterruptService->GetAudioInterruptBundleName(audioInterrupt);
    EXPECT_TRUE(audioInterrupt.bundleName.compare(str)==0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_129
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_129, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_EQ(audioCoreService, nullptr);
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t activePid = 101;
    uint32_t sessionId = 1001;
    AudioInterrupt activeInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    activeInterrupt.streamId = sessionId;
 
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.AddInjectorStreamId(sessionId);
    audioInjectorPolicy.DeleteInjectorStreamId(sessionId);
    audioInjectorPolicy.AddInjectorStreamId(sessionId);
    int32_t ret = audioInterruptService->ActivateAudioInterrupt(0, activeInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, activeInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_130
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_130, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t activePid = 101;
    uint32_t sessionId = 1001;
    AudioInterrupt activeInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION;
    activeInterrupt.streamId = sessionId;

    uint32_t streamSessionId = 1002;
    AudioInterrupt streamActiveInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION;
    activeInterrupt.streamId = streamSessionId;

    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[1003] = streamDesc;
 
    int32_t ret = audioInterruptService->ActivateAudioInterrupt(0, activeInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->ActivateAudioInterrupt(0, streamActiveInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, activeInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, streamActiveInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_131
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_131, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t activePid = 101;
    uint32_t sessionId = 1001;
    AudioInterrupt activeInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION;
    activeInterrupt.streamId = sessionId;

    uint32_t streamSessionId = 1002;
    AudioInterrupt streamActiveInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION;
    activeInterrupt.streamId = streamSessionId;
 
    int32_t ret = audioInterruptService->ActivateAudioInterrupt(0, activeInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->ActivateAudioInterrupt(0, streamActiveInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, activeInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, streamActiveInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_132
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_132, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t activePid = 101;
    uint32_t sessionId = 1001;
    AudioInterrupt activeInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.streamType == STREAM_MUSIC;
    activeInterrupt.streamId = sessionId;
 
    int32_t ret = audioInterruptService->ActivateAudioInterrupt(0, activeInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    audioInterruptService->handler_ = nullptr;
    ret = audioInterruptService->DeactivateAudioInterrupt(0, activeInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_133
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_133, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t activePid = 101;
    uint32_t sessionId = 1001;
    AudioInterrupt activeInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.sourceType == SOURCE_TYPE_MIC;
    activeInterrupt.streamId = sessionId;

    uint32_t streamSessionId = 1002;
    AudioInterrupt streamActiveInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION;
    activeInterrupt.streamId = streamSessionId;
 
    int32_t ret = audioInterruptService->ActivateAudioInterrupt(0, activeInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->ActivateAudioInterrupt(0, streamActiveInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, activeInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, streamActiveInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_134
* @tc.desc  : Test AudioInterruptService
*/
HWTEST_F(AudioInterruptUnitTest, AudioInterruptService_134, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t activePid = 101;
    uint32_t sessionId = 1001;
    AudioInterrupt activeInterrupt;
    activeInterrupt.pid = activePid;
    activeInterrupt.audioFocusType.sourceType == SOURCE_TYPE_MIC;
    activeInterrupt.streamId = sessionId;

    int32_t ret = audioInterruptService->ActivateAudioInterrupt(0, activeInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(0, activeInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test RegisterDefaultVolumeTypeListener
* @tc.number: RegisterDefaultVolumeTypeListenerTest
* @tc.desc  : Test RegisterDefaultVolumeTypeListener
*/
HWTEST_F(AudioInterruptUnitTest, RegisterDefaultVolumeTypeListenerTest, TestSize.Level1)
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    bool isDataShareReady = settingProvider.isDataShareReady_.load();
    settingProvider.SetDataShareReady(true);
    ASSERT_TRUE(settingProvider.isDataShareReady_.load());
    auto interruptServiceTest = GetTnterruptServiceTest();
    ASSERT_TRUE(interruptServiceTest != nullptr);
    // The result can be verified only after the datashare mock framework is completed.
    interruptServiceTest->RegisterDefaultVolumeTypeListener();
    settingProvider.SetDataShareReady(isDataShareReady);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_001
 * @tc.desc  : Test AudioSessionFocusMode
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_001, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    AudioInterrupt movieInterrupt;
    movieInterrupt.pid = CALLER_PID;
    movieInterrupt.streamId = 123; // fake stream id.
    movieInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    movieInterrupt.audioFocusType.isPlay = true;
    movieInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    int32_t ret = audioInterruptService->ActivateAudioInterrupt(DEFAULT_ZONE_ID, movieInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioInterruptService->ActivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);

    movieInterrupt.streamId = 456; // fake stream id.
    ret = audioInterruptService->ActivateAudioInterrupt(DEFAULT_ZONE_ID, movieInterrupt, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_002
 * @tc.desc  : Test ActivateAudioSession interrupt other focus
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_002, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    AudioInterrupt movieInterrupt;
    movieInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    movieInterrupt.audioFocusType.isPlay = true;
    movieInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(movieInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    int32_t ret = audioInterruptService->sessionService_.SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioInterruptService->ActivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);

    auto &newAudioInterruptZone = audioInterruptService->zonesMap_[DEFAULT_ZONE_ID];
    EXPECT_EQ(1, newAudioInterruptZone->audioFocusInfoList.size());

    ret = audioInterruptService->DeactivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_003
 * @tc.desc  : Test AudioSessionAbnormalCase
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_003, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());

    AudioInterrupt movieInterrupt;
    movieInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    movieInterrupt.audioFocusType.isPlay = true;
    movieInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;

    audioInterruptService->TryHandleStreamCallbackInSession(DEFAULT_ZONE_ID, movieInterrupt);

    InterruptEventInternal interruptEventInternal;
    audioInterruptService->DispatchInterruptEventForAudioSession(interruptEventInternal, movieInterrupt);

    audioInterruptService->SendAudioSessionInterruptEventCallback(interruptEventInternal, movieInterrupt);

    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    audioInterruptService->SendAudioSessionInterruptEventCallback(interruptEventInternal, movieInterrupt);

    int32_t zoneId = -1;
    bool result = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, movieInterrupt);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_004
 * @tc.desc  : Test ShouldBypassAudioSessionFocus
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_004, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t ret = audioInterruptService->SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    AudioInterrupt movieInterrupt;
    movieInterrupt.pid = CALLER_PID;
    movieInterrupt.streamId = SESSION_ID_TEST;
    movieInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    movieInterrupt.audioFocusType.isPlay = true;
    movieInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;

    int32_t zoneId = 0;
    bool result = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, movieInterrupt);
    EXPECT_FALSE(result);

    movieInterrupt.isAudioSessionInterrupt = true;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(movieInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    result = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, movieInterrupt);
    EXPECT_FALSE(result);

    movieInterrupt.isAudioSessionInterrupt = false;
    result = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, movieInterrupt);
    EXPECT_FALSE(result);

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioInterruptService->ActivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);

    movieInterrupt.isAudioSessionInterrupt = false;
    result = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, movieInterrupt);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_005
 * @tc.desc  : Test AudioSessionTimeOut
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_005, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    int32_t ret = audioInterruptService->SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioInterruptService->ActivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);

    bool result = audioInterruptService->IsAudioSessionActivated(CALLER_PID);
    EXPECT_TRUE(result);

    ret = audioInterruptService->sessionService_.DeactivateAudioSession(CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);

    audioInterruptService->HandleSessionTimeOutEvent(CALLER_PID);
    result = audioInterruptService->IsAudioSessionActivated(CALLER_PID);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_006
 * @tc.desc  : Test AudioSessionCallbackEvent
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_006, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    AudioInterrupt movieInterrupt;
    movieInterrupt.pid = CALLER_PID;
    movieInterrupt.streamId = SESSION_ID_TEST;
    movieInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    movieInterrupt.audioFocusType.isPlay = true;
    movieInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;

    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    InterruptEventInternal duckInterruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, 1.0f};
    InterruptEventInternal stopInterruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    audioInterruptService->SendAudioSessionInterruptEventCallback(duckInterruptEvent, movieInterrupt);
    audioInterruptService->SendAudioSessionInterruptEventCallback(stopInterruptEvent, movieInterrupt);

    audioInterruptService->Init(GetPolicyServerTest());

    audioInterruptService->TryHandleStreamCallbackInSession(DEFAULT_ZONE_ID, movieInterrupt);
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());
    audioInterruptService->TryHandleStreamCallbackInSession(DEFAULT_ZONE_ID, movieInterrupt);
    audioInterruptService->SendAudioSessionInterruptEventCallback(duckInterruptEvent, movieInterrupt);
    audioInterruptService->SendAudioSessionInterruptEventCallback(stopInterruptEvent, movieInterrupt);

    int32_t ret = audioInterruptService->SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    movieInterrupt.isAudioSessionInterrupt = true;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(movieInterrupt, AudioFocuState{PAUSE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;
    audioInterruptService->TryHandleStreamCallbackInSession(DEFAULT_ZONE_ID, movieInterrupt);
    audioInterruptService->SendAudioSessionInterruptEventCallback(duckInterruptEvent, movieInterrupt);
    audioInterruptService->SendAudioSessionInterruptEventCallback(stopInterruptEvent, movieInterrupt);

    audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(movieInterrupt, AudioFocuState{PAUSE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;
    audioInterruptService->TryHandleStreamCallbackInSession(DEFAULT_ZONE_ID, movieInterrupt);

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioInterruptService->sessionService_.ActivateAudioSession(CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
    ret = audioInterruptService->sessionService_.DeactivateAudioSession(CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_007
 * @tc.desc  : Test ProcessFocusEntryForAudioSession
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_007, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());

    int32_t ret = audioInterruptService->SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    bool updateScene = false;
    ret = audioInterruptService->ProcessFocusEntryForAudioSession(DEFAULT_ZONE_ID, CALLER_PID, updateScene);
    EXPECT_EQ(SUCCESS, ret);

    AudioInterrupt movieInterrupt;
    movieInterrupt.pid = CALLER_PID;
    movieInterrupt.streamId = SESSION_ID_TEST;
    movieInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    movieInterrupt.audioFocusType.isPlay = true;
    movieInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(movieInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    ret = audioInterruptService->ProcessFocusEntryForAudioSession(DEFAULT_ZONE_ID, CALLER_PID, updateScene);
    EXPECT_EQ(SUCCESS, ret);

    audioInterruptService->isPreemptMode_ = true;
    ret = audioInterruptService->ProcessFocusEntryForAudioSession(DEFAULT_ZONE_ID, CALLER_PID, updateScene);
    EXPECT_EQ(ERR_FOCUS_DENIED, ret);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_008
 * @tc.desc  : Test GetHighestPriorityAudioScene
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_008, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt fakeAudioInterrupt;
    fakeAudioInterrupt.pid = CALLER_PID;
    fakeAudioInterrupt.streamId = SESSION_ID_TEST;
    fakeAudioInterrupt.audioFocusType.streamType = STREAM_VOICE_COMMUNICATION;
    fakeAudioInterrupt.audioFocusType.isPlay = true;
    fakeAudioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    fakeAudioInterrupt.isAudioSessionInterrupt = true;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(fakeAudioInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    AudioScene audioScene = audioInterruptService->GetHighestPriorityAudioScene(DEFAULT_ZONE_ID);
    EXPECT_EQ(AUDIO_SCENE_PHONE_CHAT, audioScene);

    audioScene = audioInterruptService->RefreshAudioSceneFromAudioInterrupt(fakeAudioInterrupt, audioScene);
    EXPECT_EQ(AUDIO_SCENE_PHONE_CHAT, audioScene);

    audioScene = audioInterruptService->GetHighestPriorityAudioSceneFromAudioSession(fakeAudioInterrupt, audioScene);
    EXPECT_EQ(AUDIO_SCENE_PHONE_CHAT, audioScene);

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    int32_t ret = audioInterruptService->sessionService_.ActivateAudioSession(CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
    ASSERT_NE(nullptr, audioInterruptService->sessionService_.sessionMap_[CALLER_PID]);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = CALLER_PID;
    audioInterrupt.streamId = SESSION_ID_TEST + 1;
    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    audioInterruptService->sessionService_.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);

    audioScene = audioInterruptService->GetHighestPriorityAudioSceneFromAudioSession(fakeAudioInterrupt, audioScene);
    EXPECT_EQ(AUDIO_SCENE_PHONE_CHAT, audioScene);

    audioInterrupt.streamId++;
    audioInterrupt.audioFocusType.streamType = STREAM_RING;
    audioInterruptService->sessionService_.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);
    audioScene = AUDIO_SCENE_DEFAULT;
    audioScene = audioInterruptService->GetHighestPriorityAudioSceneFromAudioSession(fakeAudioInterrupt, audioScene);
    EXPECT_EQ(AUDIO_SCENE_RINGING, audioScene);

    ret = audioInterruptService->sessionService_.DeactivateAudioSession(CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_009
 * @tc.desc  : Test DeactivatAudioSession v2
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_009, TestSize.Level2)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->Init(GetPolicyServerTest());
    audioInterruptService->SetCallbackHandler(GetServerHandlerTest());

    AudioInterrupt fakeAudioInterrupt;
    fakeAudioInterrupt.pid = CALLER_PID + 1;
    fakeAudioInterrupt.streamId = SESSION_ID_TEST;
    fakeAudioInterrupt.audioFocusType.streamType = STREAM_VOICE_COMMUNICATION;
    fakeAudioInterrupt.audioFocusType.isPlay = true;
    fakeAudioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    fakeAudioInterrupt.isAudioSessionInterrupt = true;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(fakeAudioInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    int32_t ret = audioInterruptService->sessionService_.SetAudioSessionScene(CALLER_PID, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioInterruptService->ActivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioInterruptService->DeactivateAudioInterrupt(DEFAULT_ZONE_ID, fakeAudioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    auto &newAudioInterruptZone = audioInterruptService->zonesMap_[DEFAULT_ZONE_ID];
    EXPECT_EQ(1, newAudioInterruptZone->audioFocusInfoList.size());

    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    ret = audioInterruptService->DeactivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);

    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    audioInterruptService->sessionService_.sessionMap_[CALLER_PID] =
        std::make_shared<AudioSession>(CALLER_PID, strategy, audioSessionStateMonitor_);

    ASSERT_NE(nullptr, audioInterruptService->sessionService_.sessionMap_[CALLER_PID]);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = CALLER_PID;
    audioInterrupt.streamId = SESSION_ID_TEST + 1;
    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    audioInterruptService->sessionService_.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);

    ret = audioInterruptService->DeactivateAudioSession(DEFAULT_ZONE_ID, CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_010
 * @tc.desc  : Test DelayToDeactivateStreamsInAudioSession
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_010, TestSize.Level2)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    const std::vector<AudioInterrupt> streamsInSession;
    audioInterruptService->DelayToDeactivateStreamsInAudioSession(DEFAULT_ZONE_ID, CALLER_PID, streamsInSession);

    auto audioInterruptServiceWithSession = std::make_shared<AudioInterruptService>();
    audioInterruptServiceWithSession->SetCallbackHandler(GetServerHandlerTest());
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    audioInterruptServiceWithSession->sessionService_.sessionMap_[CALLER_PID] =
        std::make_shared<AudioSession>(CALLER_PID, strategy, audioSessionStateMonitor_);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = CALLER_PID;
    audioInterrupt.streamId = SESSION_ID_TEST + 1;
    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    audioInterruptServiceWithSession->sessionService_.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);

    audioInterruptServiceWithSession->sessionService_.sessionMap_[CALLER_PID]->state_ =
        AudioSessionState::SESSION_ACTIVE;
    audioInterruptServiceWithSession->DelayToDeactivateStreamsInAudioSession(
        DEFAULT_ZONE_ID, CALLER_PID, audioInterruptServiceWithSession->sessionService_.GetStreams(CALLER_PID));

    auto audioInterruptServiceWithoutHandler = std::make_shared<AudioInterruptService>();
    audioInterruptServiceWithoutHandler->SetCallbackHandler(nullptr);
    audioInterruptServiceWithoutHandler->sessionService_.sessionMap_[CALLER_PID] =
        std::make_shared<AudioSession>(CALLER_PID, strategy, audioSessionStateMonitor_);
    audioInterruptServiceWithoutHandler->sessionService_.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);

    audioInterruptServiceWithoutHandler->sessionService_.sessionMap_[CALLER_PID]->state_
        = AudioSessionState::SESSION_NEW;
    audioInterruptServiceWithoutHandler->DelayToDeactivateStreamsInAudioSession(
        DEFAULT_ZONE_ID, CALLER_PID, audioInterruptServiceWithoutHandler->sessionService_.GetStreams(CALLER_PID));

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_011
 * @tc.desc  : Test HasStreamForDeviceType
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_011, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto &sessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    EXPECT_FALSE(sessionService.HasStreamForDeviceType(CALLER_PID, DEVICE_TYPE_REMOTE_CAST));

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    int32_t ret = sessionService.ActivateAudioSession(CALLER_PID, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
    ASSERT_NE(nullptr, sessionService.sessionMap_[CALLER_PID]);
    EXPECT_FALSE(sessionService.HasStreamForDeviceType(CALLER_PID, DEVICE_TYPE_REMOTE_CAST));
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = CALLER_PID;
    audioInterrupt.streamId = SESSION_ID_TEST + 1;
    audioInterrupt.streamUsage = STREAM_USAGE_MUSIC;
    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    sessionService.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);

    AudioStreamCollector &audioStreamCollector = AudioStreamCollector::GetAudioStreamCollector();
    AudioDeviceDescriptor outputDeviceInfo(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE, 0, 0, "RemoteDevice");
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = CALLER_UID;
    rendererChangeInfo->createrUID = CALLER_UID + 1;
    rendererChangeInfo->sessionId = audioInterrupt.streamId;
    rendererChangeInfo->outputDeviceInfo = outputDeviceInfo;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    audioStreamCollector.audioRendererChangeInfos_.push_back(rendererChangeInfo);
    EXPECT_TRUE(sessionService.HasStreamForDeviceType(CALLER_PID, DEVICE_TYPE_REMOTE_CAST));

    ret = sessionService.DeactivateAudioSession(CALLER_PID);
    EXPECT_EQ(SUCCESS, ret);
    audioStreamCollector.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_012
 * @tc.desc  : Test GetStreamIdsForAudioSessionByStreamUsage
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_012, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt fakeAudioInterrupt;
    fakeAudioInterrupt.pid = CALLER_PID;
    fakeAudioInterrupt.streamUsage = STREAM_USAGE_MUSIC;
    fakeAudioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    fakeAudioInterrupt.streamId = SESSION_ID_TEST + 1;
    fakeAudioInterrupt.audioFocusType.isPlay = true;
    fakeAudioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(fakeAudioInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    std::set<StreamUsage> streamUsageSet;
    auto streamIds = audioInterruptService->GetStreamIdsForAudioSessionByStreamUsage(DEFAULT_ZONE_ID, streamUsageSet);
    EXPECT_TRUE(streamIds.empty());

    fakeAudioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(fakeAudioInterrupt, AudioFocuState{ACTIVE});
    audioInterruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    streamUsageSet.insert(STREAM_USAGE_ALARM);
    streamIds = audioInterruptService->GetStreamIdsForAudioSessionByStreamUsage(DEFAULT_ZONE_ID, streamUsageSet);
    EXPECT_TRUE(streamIds.empty());

    streamUsageSet.insert(STREAM_USAGE_MUSIC);
    streamIds = audioInterruptService->GetStreamIdsForAudioSessionByStreamUsage(DEFAULT_ZONE_ID, streamUsageSet);
    EXPECT_EQ(1, streamIds.size());
    EXPECT_EQ(fakeAudioInterrupt.streamId, *streamIds.begin());

    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    ASSERT_NE(nullptr, server);
    streamIds = server->GetStreamIdsForAudioSessionByStreamUsage(DEFAULT_ZONE_ID, streamUsageSet);
    EXPECT_TRUE(streamIds.empty());

    const sptr<AudioPolicyServer> &policyServer = GetPolicyServerTest();
    ASSERT_NE(nullptr, policyServer);
    ASSERT_NE(nullptr, policyServer->interruptService_);
    policyServer->interruptService_->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;
    streamIds = policyServer->GetStreamIdsForAudioSessionByStreamUsage(DEFAULT_ZONE_ID, streamUsageSet);
    EXPECT_EQ(1, streamIds.size());
    EXPECT_EQ(fakeAudioInterrupt.streamId, *streamIds.begin());
}

/**
 * @tc.name  : Test AudioSessionFocusMode
 * @tc.number: AudioSessionFocusMode_013
 * @tc.desc  : Test GetStreamIdsForAudioSessionByDeviceType
 */
HWTEST_F(AudioInterruptUnitTest, AudioSessionFocusMode_013, TestSize.Level1)
{
    int32_t CALLER_PID = IPCSkeleton::GetCallingPid();
    auto interruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(nullptr, interruptService);

    AudioInterrupt fakeAudioInterrupt;
    fakeAudioInterrupt.pid = CALLER_PID;
    fakeAudioInterrupt.streamUsage = STREAM_USAGE_MUSIC;
    fakeAudioInterrupt.streamId = SESSION_ID_TEST + 1;
    fakeAudioInterrupt.audioFocusType.isPlay = true;
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.emplace_back(fakeAudioInterrupt, AudioFocuState{ACTIVE});
    interruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    auto streamIds =
        interruptService->GetStreamIdsForAudioSessionByDeviceType(DEFAULT_ZONE_ID, DEVICE_TYPE_REMOTE_CAST);
    EXPECT_TRUE(streamIds.empty());

    fakeAudioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.clear();
    audioInterruptZone->audioFocusInfoList.emplace_back(fakeAudioInterrupt, AudioFocuState{ACTIVE});
    interruptService->zonesMap_[DEFAULT_ZONE_ID] = audioInterruptZone;

    streamIds = interruptService->GetStreamIdsForAudioSessionByDeviceType(DEFAULT_ZONE_ID, DEVICE_TYPE_REMOTE_CAST);
    EXPECT_TRUE(streamIds.empty());

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    interruptService->sessionService_.ActivateAudioSession(CALLER_PID, audioSessionStrategy);
    ASSERT_NE(nullptr, interruptService->sessionService_.sessionMap_[CALLER_PID]);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = CALLER_PID;
    audioInterrupt.streamId = SESSION_ID_TEST + 1;
    audioInterrupt.streamUsage = STREAM_USAGE_MUSIC;
    audioInterrupt.audioFocusType.isPlay = true;
    interruptService->sessionService_.sessionMap_[CALLER_PID]->AddStreamInfo(audioInterrupt);

    streamIds = interruptService->GetStreamIdsForAudioSessionByDeviceType(DEFAULT_ZONE_ID, DEVICE_TYPE_REMOTE_CAST);
    EXPECT_TRUE(streamIds.empty());

    AudioDeviceDescriptor outputDeviceInfo(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE, 0, 0, "RemoteDevice");
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = CALLER_UID;
    rendererChangeInfo->createrUID = CALLER_UID + 1;
    rendererChangeInfo->sessionId = SESSION_ID_TEST + 1;
    rendererChangeInfo->outputDeviceInfo = outputDeviceInfo;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    AudioStreamCollector::GetAudioStreamCollector().audioRendererChangeInfos_.push_back(rendererChangeInfo);

    streamIds = interruptService->GetStreamIdsForAudioSessionByDeviceType(DEFAULT_ZONE_ID, DEVICE_TYPE_REMOTE_CAST);
    EXPECT_EQ(1, streamIds.size());
    EXPECT_EQ(fakeAudioInterrupt.streamId, *streamIds.begin());

    interruptService->sessionService_.DeactivateAudioSession(CALLER_PID);
    AudioStreamCollector::GetAudioStreamCollector().audioRendererChangeInfos_.clear();
}

} // namespace AudioStandard
} // namespace OHOS
