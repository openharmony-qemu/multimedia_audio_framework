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

#include "audio_policy_server_unit_test.h"
#include "audio_policy_server.h"
#include "audio_interrupt_unit_test.h"
#include "audio_info.h"
#include "securec.h"
#include "audio_interrupt_service.h"
#include "audio_device_descriptor.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermission = false;
bool g_hasServerInit = false;
sptr<AudioPolicyServer> GetPolicyServerUnitTest()
{
    static int32_t systemAbilityId = 3009;
    static bool runOnCreate = false;
    static sptr<AudioPolicyServer> server =
        sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    if (!g_hasServerInit) {
        IdHandler::GetInstance();
        HdiAdapterManager::GetInstance();
        HPAE::IHpaeManager::GetHpaeManager().Init();
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

void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void ReleaseServer()
{
    GetPolicyServerUnitTest()->OnStop();
    g_hasServerInit = false;
}

void AudioPolicyUnitTest::SetUpTestCase(void) {}
void AudioPolicyUnitTest::TearDownTestCase(void)
{
    ReleaseServer();
}

void AudioPolicyUnitTest::SetUp(void)
{
    GetPermission();
}

void AudioPolicyUnitTest::TearDown(void) {}

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

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_001
* @tc.desc  : Test CheckAudioSessionStrategy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_001, TestSize.Level1)
{
    AudioSessionStrategy strategy;
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
    strategy.concurrencyMode = AudioConcurrencyMode::DUCK_OTHERS;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
    strategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;
    EXPECT_TRUE(server->CheckAudioSessionStrategy(strategy));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_002
* @tc.desc  : Test CheckAudioSessionStrategy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_002, TestSize.Level1)
{
    AudioSessionStrategy strategy;
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    strategy.concurrencyMode = static_cast<AudioConcurrencyMode>(999); // Invalid mode
    EXPECT_FALSE(server->CheckAudioSessionStrategy(strategy));
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_003
* @tc.desc  : Test SetAudioManagerInterruptCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_003, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    server->interruptService_ = nullptr;
    int32_t result = server->SetAudioManagerInterruptCallback(0, sptr<RemoteObjectTestStub>::MakeSptr());
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_004
* @tc.desc  : Test UnsetAudioManagerInterruptCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_004, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    server->interruptService_ = nullptr;
    int32_t result = server->UnsetAudioManagerInterruptCallback(0);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_005
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_005, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = static_cast<AudioParamKey>(100);
    std::string condition;
    std::string value;
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_006
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_006, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = PARAM_KEY_STATE;
    std::string condition;
    std::string value;
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_008
* @tc.desc  : Test ArgInfoDump.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_008, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"invalidParam");
    server->ArgInfoDump(dumpString, argQue);
    EXPECT_EQ(dumpString.find("Please input correct param:\n"), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_009
* @tc.desc  : Test DeactivateAudioInterrupt.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_009, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    AudioInterrupt audioInterrupt;
    int32_t zoneID = 456;
    int32_t result = server->DeactivateAudioInterrupt(audioInterrupt, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_010
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_010, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = VOLUME;
    std::string condition;
    std::string value;

    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_011
* @tc.desc  : Test OnAudioParameterChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_011, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);

    std::string networkId;
    AudioParamKey key = INTERRUPT;
    std::string condition;
    std::string value;

    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_012
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_012, TestSize.Level1)
{
    int32_t strategy = 0;
    auto policyServerTest = GetPolicyServerUnitTest();
    EXPECT_EQ(policyServerTest->ActivateAudioSession(strategy), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_013
* @tc.desc  : Test GetStreamInFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_013, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t zoneID = 456;
    int32_t result = 0;
    policyServerTest->GetStreamInFocus(zoneID, result);
    EXPECT_EQ(result, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_014
* @tc.desc  : Test OnRemoveSystemAbility.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_014, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = AVSESSION_SERVICE_ID;
    std::string deviceId = "132456";
    policyServerTest->OnRemoveSystemAbility(systemAbilityId, deviceId);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_015
* @tc.desc  : Test InitMicrophoneMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_015, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    policyServerTest->isInitMuteState_ = true;
    policyServerTest->InitMicrophoneMute();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_016
* @tc.desc  : Test ActivateAudioInterrupt.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_016, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();
    AudioInterrupt audioInterrupt;
    int32_t zoneID = 456;
    int32_t result = 0;
    policyServerTest->ActivateAudioInterrupt(audioInterrupt, zoneID, result);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_017
* @tc.desc  : Test SetRingerModeLegacy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_017, TestSize.Level1)
{
    auto policyServerTest = GetPolicyServerUnitTest();

    AudioRingerMode audioRingerMode = AudioRingerMode::RINGER_MODE_NORMAL;
    int32_t result = policyServerTest->SetRingerModeLegacy(audioRingerMode);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_018
* @tc.desc  : Test AudioPolicyServer::LoadSplitModule.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_018, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const std::string splitArgs = "";
    const std::string networkId = "";
    ptrAudioPolicyServer->LoadSplitModule(splitArgs, networkId);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_019
* @tc.desc  : Test AudioPolicyServer::IsAudioSessionActivated
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_019, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool isActive = false;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->IsAudioSessionActivated(isActive);
    EXPECT_EQ(isActive, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_020
* @tc.desc  : Test AudioPolicyServer::IsAudioSessionActivated
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_020, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool isActive = false;
    ptrAudioPolicyServer->interruptService_ = nullptr;
    ptrAudioPolicyServer->IsAudioSessionActivated(isActive);
    EXPECT_EQ(isActive, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_021
* @tc.desc  : Test AudioPolicyServer::DeactivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_021, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    ptrAudioPolicyServer->interruptService_ = nullptr;
    ptrAudioPolicyServer->DeactivateAudioSession();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_022
* @tc.desc  : Test AudioPolicyServer::DeactivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_022, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->DeactivateAudioSession();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_023
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_023, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_024
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_024, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = nullptr;
    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_025
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_025, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();

    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_026
* @tc.desc  : Test AudioPolicyServer::ActivateAudioSession
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_026, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t strategy = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();

    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_027
* @tc.desc  : Test AudioPolicyServer::InjectInterruption
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_027, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const std::string networkId = "";
    InterruptEvent event;

    auto ret = ptrAudioPolicyServer->InjectInterruption(networkId, event);

    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_028
* @tc.desc  : Test AudioPolicyServer::UnsetAudioDeviceAnahsCallback
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_028, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    auto ret = ptrAudioPolicyServer->UnsetAudioDeviceAnahsCallback();

    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_029
* @tc.desc  : Test AudioPolicyServer::TriggerFetchDevice
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_029, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    auto ret = ptrAudioPolicyServer->TriggerFetchDevice(reason);

    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_030
* @tc.desc  : Test AudioPolicyServer::UnsetAudioDeviceRefinerCallback
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_030, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    auto ret = ptrAudioPolicyServer->UnsetAudioDeviceRefinerCallback();

    EXPECT_EQ(ret, ERROR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_031
* @tc.desc  : Test AudioPolicyServer::SetHighResolutionExist
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_031, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool highResExist = true;
    auto ret = ptrAudioPolicyServer->SetHighResolutionExist(highResExist);

    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_032
* @tc.desc  : Test AudioPolicyServer::IsHighResolutionExist
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_032, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    bool ret;
    ptrAudioPolicyServer->IsHighResolutionExist(ret);

    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_033
* @tc.desc  : Test AudioPolicyServer::DisableSafeMediaVolume
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_033, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    auto ret = ptrAudioPolicyServer->DisableSafeMediaVolume();

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_034
* @tc.desc  : Test AudioPolicyServer::SetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_034, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const AudioSpatializationSceneType spatializationSceneType =
        AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT;
    auto ret = ptrAudioPolicyServer->SetSpatializationSceneType(spatializationSceneType);

    EXPECT_EQ(ret, OPEN_PORT_FAILURE);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_035
* @tc.desc  : Test AudioPolicyServer::GetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_035, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t ret = 0;
    ptrAudioPolicyServer->GetSpatializationSceneType(ret);

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_036
* @tc.desc  : Test AudioPolicyServer::GetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_036, TestSize.Level1)
{
    auto ptrAudioPolicyServer = GetPolicyServerUnitTest();

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> descs = nullptr;
    ptrAudioPolicyServer->GetActiveBluetoothDevice(descs);
    EXPECT_EQ(descs, nullptr);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_037
* @tc.desc  : Test AudioPolicyServer::SetCallDeviceActive
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_037, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    InternalDeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    bool active = true;
    std::string address = "";
    int32_t ret = 0;
    ptrAudioPolicyServer->SetCallDeviceActive(deviceType, active, address, ret);

    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_038
* @tc.desc  : Test AudioPolicyServer::SetCallDeviceActive
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_038, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    InternalDeviceType deviceType = DeviceType::DEVICE_TYPE_EARPIECE;
    bool active = true;
    std::string address = "";
    int32_t ret = 0;
    ptrAudioPolicyServer->SetCallDeviceActive(deviceType, active, address, ret);

    EXPECT_NE(ret, ERR_SYSTEM_PERMISSION_DENIED);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_039
* @tc.desc  : Test AudioPolicyServer::ReleaseAudioInterruptZone
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_039, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const int32_t zoneID = 0;
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    auto ret = ptrAudioPolicyServer->ReleaseAudioInterruptZone(zoneID);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_040
* @tc.desc  : Test AudioPolicyServer::ReleaseAudioInterruptZone
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_040, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    const int32_t zoneID = 0;
    ptrAudioPolicyServer->interruptService_ = nullptr;
    auto ret = ptrAudioPolicyServer->ReleaseAudioInterruptZone(zoneID);

    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetSystemVolumeLevelInternal_001
* @tc.desc  : Test AudioPolicyServer::SetSystemVolumeLevelInternal
*/
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevelInternal_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelInternal_001 start");
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_NE(ptrAudioPolicyServer, nullptr);

    int32_t volumeLevel = 5;
    bool isUpdateUi = true;
    auto ret = ptrAudioPolicyServer->SetSystemVolumeLevelInternal(STREAM_VOICE_CALL, volumeLevel, isUpdateUi);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: SetSystemVolumeLevelInternal_002
* @tc.desc  : Test AudioPolicyServer::SetSystemVolumeLevelInternal
*/
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevelInternal_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelInternal_002 start");
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    ASSERT_NE(ptrAudioPolicyServer, nullptr);

    int32_t volumeLevel = 5;
    bool isUpdateUi = true;
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = ptrAudioPolicyServer->SetSystemVolumeLevelInternal(STREAM_VOICE_CALL, volumeLevel, isUpdateUi);
    VolumeUtils::SetPCVolumeEnable(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_041
* @tc.desc  : Test AudioPolicyServer::GetSystemActiveVolumeType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_041, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t streamType;
    int32_t clientUid = 0;
    ptrAudioPolicyServer->GetSystemActiveVolumeType(clientUid, streamType);
    EXPECT_EQ(streamType, STREAM_MUSIC);
    clientUid = 1;
    ptrAudioPolicyServer->GetSystemActiveVolumeType(clientUid, streamType);
    EXPECT_EQ(streamType, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_042
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_042, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioStreamType streamType = STREAM_ALL;
    int res = ptrAudioPolicyServer->GetSystemVolumeLevelNoMuteState(streamType);
    EXPECT_EQ(res, 5);
    streamType = STREAM_MUSIC;
    res = ptrAudioPolicyServer->GetSystemVolumeLevelNoMuteState(streamType);
    EXPECT_EQ(res, 5);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_043
* @tc.desc  : Test AudioPolicyServer::GetStreamMute
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_043, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t streamType = 2;
    bool ret;
    ptrAudioPolicyServer->GetStreamMute(streamType, ret);
    EXPECT_EQ(ret, false);
    streamType = 25;
    ret = ptrAudioPolicyServer->GetStreamMute(streamType, ret);
    EXPECT_EQ(ret, false);
    streamType = 1;
    ret = ptrAudioPolicyServer->GetStreamMute(streamType, ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_044
* @tc.desc  : Test AudioPolicyServer::GetPreferredOutputStreamType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_044, TestSize.Level1)
{
    auto ptrAudioPolicyServer = GetPolicyServerUnitTest();

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioRendererInfo rendererInfo;
    int32_t ret = 0;
    ptrAudioPolicyServer->GetPreferredOutputStreamType(rendererInfo, ret);
    EXPECT_EQ(ret, 0);
    ptrAudioPolicyServer->audioPolicyService_.isFastControlled_ = true;
    ptrAudioPolicyServer->GetPreferredOutputStreamType(rendererInfo, ret);
    EXPECT_EQ(ret, 0);
    rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    ptrAudioPolicyServer->GetPreferredOutputStreamType(rendererInfo, ret);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);
}

/**
* @tc.name  : Test TranslateErrorCodeer.
* @tc.number: TranslateErrorCode_001
* @tc.desc  : Test TranslateErrorCodeer.
*/
HWTEST(AudioPolicyUnitTest, TranslateErrorCode_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t result = ERR_INVALID_PARAM;
    uint32_t resultForMonitor = ERR_SUBSCRIBE_INVALID_PARAM;
    uint32_t actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = ERR_NULL_POINTER;
    resultForMonitor = ERR_SUBSCRIBE_KEY_OPTION_NULL;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = ERR_MMI_CREATION;
    resultForMonitor = ERR_SUBSCRIBE_MMI_NULL;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = ERR_MMI_SUBSCRIBE;
    resultForMonitor = ERR_MODE_SUBSCRIBE;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);

    result = 99999;
    resultForMonitor = 0;
    actual = ptrAudioPolicyServer->TranslateErrorCode(result);
    EXPECT_EQ(resultForMonitor, actual);
}

/**
* @tc.name  : Test IsVolumeTypeValid.
* @tc.number: IsVolumeTypeValid_001
* @tc.desc  : Test AudioPolicyServer::IsVolumeTypeValid
*/
HWTEST(AudioPolicyUnitTest, IsVolumeTypeValid_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    bool result = ptrAudioPolicyServer->IsVolumeTypeValid(static_cast<AudioStreamType>(-1));
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test UpdateMuteStateAccordingToVolLevel.
* @tc.number: UpdateMuteStateAccordingToVolLevel_001
* @tc.desc  : Test AudioPolicyServer::UpdateMuteStateAccordingToVolLevel
*/
HWTEST(AudioPolicyUnitTest, UpdateMuteStateAccordingToVolLevel_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 1;
    bool mute = true;
    ptrAudioPolicyServer->UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute);
}

/**
* @tc.name  : Test UpdateMuteStateAccordingToVolLevel.
* @tc.number: UpdateMuteStateAccordingToVolLevel_002
* @tc.desc  : Test AudioPolicyServer::UpdateMuteStateAccordingToVolLevel
*/
HWTEST(AudioPolicyUnitTest, UpdateMuteStateAccordingToVolLevel_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 0;
    bool mute = false;
    ptrAudioPolicyServer->UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute);
}

/**
* @tc.name  : Test UpdateMuteStateAccordingToVolLevel.
* @tc.number: UpdateMuteStateAccordingToVolLevel_003
* @tc.desc  : Test AudioPolicyServer::UpdateMuteStateAccordingToVolLevel
*/
HWTEST(AudioPolicyUnitTest, UpdateMuteStateAccordingToVolLevel_003, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    AudioStreamType streamType = AudioStreamType::STREAM_SYSTEM;
    int32_t volumeLevel = 1;
    bool mute = false;
    ptrAudioPolicyServer->UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute);
}

/**
* @tc.name  : Test ChangeVolumeOnVoiceAssistant.
* @tc.number: ChangeVolumeOnVoiceAssistant_001
* @tc.desc  : Test AudioPolicyServer::ChangeVolumeOnVoiceAssistant
*/
HWTEST(AudioPolicyUnitTest, ChangeVolumeOnVoiceAssistant_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_VOICE_ASSISTANT;
    ptrAudioPolicyServer->ChangeVolumeOnVoiceAssistant(streamInFocus);
}

/**
* @tc.name  : Test MaxOrMinVolumeOption.
* @tc.number: MaxOrMinVolumeOption_001
* @tc.desc  : Test AudioPolicyServer::MaxOrMinVolumeOption
*/
#ifdef FEATURE_MULTIMODALINPUT_INPUT
HWTEST(AudioPolicyUnitTest, MaxOrMinVolumeOption_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t volLevel = 20;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC;
    bool result = ptrAudioPolicyServer->MaxOrMinVolumeOption(volLevel, keyType, streamInFocus);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test MaxOrMinVolumeOption.
* @tc.number: MaxOrMinVolumeOption_002
* @tc.desc  : Test AudioPolicyServer::MaxOrMinVolumeOption
*/
HWTEST(AudioPolicyUnitTest, MaxOrMinVolumeOption_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    int32_t volLevel = 0;
    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC;
    bool result = ptrAudioPolicyServer->MaxOrMinVolumeOption(volLevel, keyType, streamInFocus);
    EXPECT_FALSE(result);
}
#endif

/**
* @tc.name  : Test IsVolumeLevelValid.
* @tc.number: IsVolumeLevelValid_001
* @tc.desc  : Test AudioPolicyServer::IsVolumeLevelValid
*/
HWTEST(AudioPolicyUnitTest, IsVolumeLevelValid_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 20;
    bool result = ptrAudioPolicyServer->IsVolumeLevelValid(streamType, volumeLevel);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test GetSystemVolumeInDb.
* @tc.number: GetSystemVolumeInDb_001
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeInDb
*/
HWTEST(AudioPolicyUnitTest, GetSystemVolumeInDb_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioVolumeType volumeType = AudioStreamType::STREAM_MUSIC;
    int32_t volumeLevel = 20;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    float actual = 0;
    ptrAudioPolicyServer->GetSystemVolumeInDb(volumeType, volumeLevel, deviceType, actual);
    float result = static_cast<float>(ERR_INVALID_PARAM);
    EXPECT_EQ(actual, result);
}

/**
* @tc.name  : Test IsArmUsbDevice.
* @tc.number: IsArmUsbDevice_001
* @tc.desc  : Test AudioPolicyServer::IsArmUsbDevice
*/
HWTEST(AudioPolicyUnitTest, IsArmUsbDevice_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    bool result = ptrAudioPolicyServer->IsArmUsbDevice(desc);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test IsArmUsbDevice.
* @tc.number: IsArmUsbDevice_002
* @tc.desc  : Test AudioPolicyServer::IsArmUsbDevice
*/
HWTEST(AudioPolicyUnitTest, IsArmUsbDevice_002, TestSize.Level1)
{
    auto ptrAudioPolicyServer = GetPolicyServerUnitTest();
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    bool result = ptrAudioPolicyServer->IsArmUsbDevice(desc);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test MapExternalToInternalDeviceType.
* @tc.number: MapExternalToInternalDeviceType_001
* @tc.desc  : Test AudioPolicyServer::MapExternalToInternalDeviceType
*/
HWTEST(AudioPolicyUnitTest, MapExternalToInternalDeviceType_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc.deviceId_ = 0;
    auto ptrAudioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(desc);
    EXPECT_NE(ptrAudioDeviceDescriptor, nullptr);
    ptrAudioPolicyServer->audioDeviceManager_.connectedDevices_.push_back(ptrAudioDeviceDescriptor);
    ptrAudioPolicyServer->MapExternalToInternalDeviceType(desc);
}

/**
* @tc.name  : Test MapExternalToInternalDeviceType.
* @tc.number: MapExternalToInternalDeviceType_002
* @tc.desc  : Test AudioPolicyServer::MapExternalToInternalDeviceType
*/
HWTEST(AudioPolicyUnitTest, MapExternalToInternalDeviceType_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceRole_ == INPUT_DEVICE;
    auto ptrAudioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(desc);
    EXPECT_NE(ptrAudioDeviceDescriptor, nullptr);
    ptrAudioPolicyServer->audioDeviceManager_.connectedDevices_.push_back(ptrAudioDeviceDescriptor);
    ptrAudioPolicyServer->MapExternalToInternalDeviceType(desc);
}

/**
* @tc.name  : Test MapExternalToInternalDeviceType.
* @tc.number: MapExternalToInternalDeviceType_003
* @tc.desc  : Test MapExternalToInternalDeviceType
*/
HWTEST(AudioPolicyUnitTest, MapExternalToInternalDeviceType_003, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);
    AudioDeviceDescriptor desc;

    desc.deviceType_ == DEVICE_TYPE_NEARLINK;
    desc.deviceRole_ == INPUT_DEVICE;
    server->MapExternalToInternalDeviceType(desc);
    EXPECT_EQ(desc.deviceType_, DEVICE_TYPE_NONE);

    desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceRole_ == INPUT_DEVICE;
    server->MapExternalToInternalDeviceType(desc);
    EXPECT_EQ(desc.deviceType_, DEVICE_TYPE_NONE);
}

/**
* @tc.name  : Test SetNearlinkDeviceVolume.
* @tc.number: SetNearlinkDeviceVolume_001
* @tc.desc  : Test SetNearlinkDeviceVolume
*/
HWTEST(AudioPolicyUnitTest, SetNearlinkDeviceVolume_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);
    
    std::string macAddress = "LocalDevice";
    int32_t streamTypeIn = 1;
    int32_t volume = 0;
    bool updateUi =true;

    int32_t ret = server->SetNearlinkDeviceVolume(macAddress, streamTypeIn, volume, updateUi);


    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
* @tc.name  : Test UpdateDeviceInfo.
* @tc.number: UpdateDeviceInfo_001
* @tc.desc  : Test UpdateDeviceInfo.
*/
HWTEST(AudioPolicyUnitTest, UpdateDeviceInfo_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>();
    int32_t command = 1;

    int32_t ret = server->UpdateDeviceInfo(deviceDesc, command);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}

/**
* @tc.name  : Test SetSleAudioOperationCallback.
* @tc.number: SetSleAudioOperationCallback_001
* @tc.desc  : Test SetSleAudioOperationCallback.
*/
HWTEST(AudioPolicyUnitTest, SetSleAudioOperationCallback_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> objectct = nullptr;

    int32_t ret = server->SetSleAudioOperationCallback(objectct);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();

    ret = server->SetSleAudioOperationCallback(objectct);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test SetCallbackCapturerInfo.
* @tc.number: SetCallbackCapturerInfo_001
* @tc.desc  : Test AudioPolicyServer::SetCallbackCapturerInfo
*/
HWTEST(AudioPolicyUnitTest, SetCallbackCapturerInfo_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t capturerFlags = 0;
    AudioCapturerInfo audioCapturerInfo(sourceType, capturerFlags);
    int32_t result = ptrAudioPolicyServer->SetCallbackCapturerInfo(audioCapturerInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test SetAudioScene.
* @tc.number: SetAudioScene_001
* @tc.desc  : Test AudioPolicyServer::SetAudioScene
*/
HWTEST(AudioPolicyUnitTest, SetAudioScene_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioScene audioScene = AUDIO_SCENE_CALL_START;
    int32_t result = ptrAudioPolicyServer->SetAudioScene(audioScene);
    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test SetAudioInterruptCallback.
* @tc.number: SetAudioInterruptCallback_001
* @tc.desc  : Test AudioPolicyServer::SetAudioInterruptCallback
*/
HWTEST(AudioPolicyUnitTest, SetAudioInterruptCallback_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    uint32_t sessionID = 0;
    sptr<IRemoteObject> object = nullptr;
    uint32_t clientUid = 0;
    int32_t zoneID = 0;
    int32_t result = ptrAudioPolicyServer->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);
}

/**
* @tc.name  : Test GetStreamVolumeInfoMap.
* @tc.number: GetStreamVolumeInfoMap_001
* @tc.desc  : Test AudioPolicyServer::GetStreamVolumeInfoMap
*/
HWTEST(AudioPolicyUnitTest, GetStreamVolumeInfoMap_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    StreamVolumeInfoMap streamVolumeInfos;
    ptrAudioPolicyServer->GetStreamVolumeInfoMap(streamVolumeInfos);
}

/**
* @tc.name  : Test InitPolicyDumpMap.
* @tc.number: InitPolicyDumpMap_001
* @tc.desc  : Test AudioPolicyServer::InitPolicyDumpMap
*/
HWTEST(AudioPolicyUnitTest, InitPolicyDumpMap_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->InitPolicyDumpMap();
}

/**
* @tc.name  : Test AudioDevicesDump.
* @tc.number: AudioDevicesDump_001
* @tc.desc  : Test AudioPolicyServer::AudioDevicesDump
*/
HWTEST(AudioPolicyUnitTest, AudioDevicesDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioDevicesDump(dumpString);
}

/**
* @tc.name  : Test AudioModeDump.
* @tc.number: AudioModeDump_001
* @tc.desc  : Test AudioPolicyServer::AudioModeDump
*/
HWTEST(AudioPolicyUnitTest, AudioModeDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioModeDump(dumpString);
}

/**
* @tc.name  : Test AudioVolumeDump.
* @tc.number: AudioVolumeDump_001
* @tc.desc  : Test AudioPolicyServer::AudioVolumeDump
*/
HWTEST(AudioPolicyUnitTest, AudioVolumeDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioVolumeDump(dumpString);
}

/**
* @tc.name  : Test AudioPolicyParserDump.
* @tc.number: AudioPolicyParserDump_001
* @tc.desc  : Test AudioPolicyServer::AudioPolicyParserDump
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyParserDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioPolicyParserDump(dumpString);
}

/**
* @tc.name  : Test AudioStreamDump.
* @tc.number: AudioStreamDump_001
* @tc.desc  : Test AudioPolicyServer::AudioStreamDump
*/
HWTEST(AudioPolicyUnitTest, AudioStreamDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->AudioStreamDump(dumpString);
}

/**
* @tc.name  : Test XmlParsedDataMapDump.
* @tc.number: XmlParsedDataMapDump_001
* @tc.desc  : Test AudioPolicyServer::XmlParsedDataMapDump
*/
HWTEST(AudioPolicyUnitTest, XmlParsedDataMapDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->XmlParsedDataMapDump(dumpString);
}

/**
* @tc.name  : Test EffectManagerInfoDump.
* @tc.number: EffectManagerInfoDump_001
* @tc.desc  : Test AudioPolicyServer::EffectManagerInfoDump
*/
HWTEST(AudioPolicyUnitTest, EffectManagerInfoDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->EffectManagerInfoDump(dumpString);
}

/**
* @tc.name  : Test MicrophoneMuteInfoDump.
* @tc.number: MicrophoneMuteInfoDump_001
* @tc.desc  : Test AudioPolicyServer::MicrophoneMuteInfoDump
*/
HWTEST(AudioPolicyUnitTest, MicrophoneMuteInfoDump_001, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    std::string dumpString = "";
    ptrAudioPolicyServer->MicrophoneMuteInfoDump(dumpString);
}

/**
* @tc.name  : Test ChangeVolumeOnVoiceAssistant.
* @tc.number: ChangeVolumeOnVoiceAssistant_002
* @tc.desc  : Test AudioPolicyServer::ChangeVolumeOnVoiceAssistant
*/
HWTEST(AudioPolicyUnitTest, ChangeVolumeOnVoiceAssistant_002, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_DEFAULT;
    ptrAudioPolicyServer->ChangeVolumeOnVoiceAssistant(streamInFocus);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_046
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_046, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = true;
    ptrAudioPolicyServer->isScreenOffOrLock_ = true;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, AUDIO_OK);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_047
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_047, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = true;
    ptrAudioPolicyServer->isScreenOffOrLock_ = false;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, ERROR_UNSUPPORTED);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_048
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_048, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = false;
    ptrAudioPolicyServer->isScreenOffOrLock_ = true;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, AUDIO_OK);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_049
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_049, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    ptrAudioPolicyServer->volumeApplyToAll_ = false;
    ptrAudioPolicyServer->isScreenOffOrLock_ = false;

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t ret = ptrAudioPolicyServer->ProcessVolumeKeyEvents(keyType);
    EXPECT_EQ(ret, AUDIO_OK);
}
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_050
* @tc.desc  : Test AudioPolicyServer::GetSystemVolumeLevelNoMuteState
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_050, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);
    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    int32_t ret = ptrAudioPolicyServer->SetStreamMute(AudioStreamType::STREAM_ALL, false, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_051
* @tc.desc  : Test AudioPolicyServer::SetRingerMode
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_051, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);
    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    ptrAudioPolicyServer->coreService_ = std::make_shared<AudioCoreService>();
    auto ret = ptrAudioPolicyServer->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrAudioPolicyServer->SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrAudioPolicyServer->SetRingerMode(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrAudioPolicyServer->SetRingerMode(60);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_052
* @tc.desc  : Test ArgInfoDump.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_052, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string dumpString;
    std::queue<std::u16string> argQue;
    server->ArgInfoDump(dumpString, argQue);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_053
* @tc.desc  : Test IsHeadTrackingEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_053, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsHeadTrackingEnabled(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_054
* @tc.desc  : Test IsHeadTrackingEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_054, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    bool ret = false;
    server->IsHeadTrackingEnabled(address, ret);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_055
* @tc.desc  : Test SetHeadTrackingEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_055, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->SetHeadTrackingEnabled(true);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_056
* @tc.desc  : Test IsSpatializationSupported.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_056, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsSpatializationSupported(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_057
* @tc.desc  : Test IsSpatializationSupportedForDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_057, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    bool ret = false;
    server->IsSpatializationSupportedForDevice(address, ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_058
* @tc.desc  : Test IsHeadTrackingSupported.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_058, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsHeadTrackingSupported(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_059
* @tc.desc  : Test IsHeadTrackingSupportedForDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_059, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    bool ret;
    server->IsHeadTrackingSupportedForDevice(address, ret);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_060
* @tc.desc  : Test UpdateSpatialDeviceState.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_060, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioSpatialDeviceState audioSpatialDeviceState;
    auto ret = server->UpdateSpatialDeviceState(audioSpatialDeviceState);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_061
* @tc.desc  : Test SetAudioDeviceRefinerCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_061, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAudioDeviceRefinerCallback(object);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_062
* @tc.desc  : Test SetPreferredDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_062, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc;
    int32_t ret = 0;
    server->SetPreferredDevice(PreferredType::AUDIO_CALL_CAPTURE, desc, ret);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_063
* @tc.desc  : Test SetPreferredDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_063, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string networkId = "test";
    VolumeBehavior volumeBehavior;
    int32_t result = server->SetDeviceVolumeBehavior(networkId, DeviceType::DEVICE_TYPE_EARPIECE, volumeBehavior);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_064
* @tc.desc  : Test SetAudioDeviceAnahsCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_064, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAudioDeviceAnahsCallback(object);
    EXPECT_EQ(ret, ERROR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_065
* @tc.desc  : Test GetSupportedAudioEffectProperty.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_065, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioEffectPropertyArrayV3 propertyArray;
    auto ret = server->GetSupportedAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_066
* @tc.desc  : Test GetSupportedAudioEffectProperty.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_066, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioEffectPropertyArrayV3 propertyArray;
    auto ret = server->SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, AUDIO_OK);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_067
* @tc.desc  : Test GetAudioEffectProperty.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_067, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioEffectPropertyArrayV3 propertyArray;
    server->GetAudioEffectProperty(propertyArray);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_068
* @tc.desc  : Test SetDeviceAbsVolumeSupported.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_068, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string macAddress = "test";
    bool support = true;
    auto ret = server->SetDeviceAbsVolumeSupported(macAddress, support);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_069
* @tc.desc  : Test SetA2dpDeviceVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_069, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string macAddress = "test";
    int32_t volume = 1;
    bool updateUi = true;
    auto ret = server->SetA2dpDeviceVolume(macAddress, volume, updateUi);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_070
* @tc.desc  : Test GetAvailableDevices.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_070, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = server->coreService_->GetEventEntry();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetAvailableDevices(AudioDeviceUsage::ALL_CALL_DEVICES, ret);
    EXPECT_NE(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_071
* @tc.desc  : Test GetAvailableDevices.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_071, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t deviceUsage = -1;
    AudioDeviceUsage usge = static_cast<AudioDeviceUsage>(deviceUsage);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetAvailableDevices(usge, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_072
* @tc.desc  : Test SetAvailableDeviceChangeCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_072, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAvailableDeviceChangeCallback(clientId, AudioDeviceUsage::ALL_CALL_DEVICES, object);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_073
* @tc.desc  : Test SetAvailableDeviceChangeCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_073, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    int32_t deviceUsage = -1;
    AudioDeviceUsage usge = static_cast<AudioDeviceUsage>(deviceUsage);
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAvailableDeviceChangeCallback(clientId, usge, object);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_074
* @tc.desc  : Test ConfigDistributedRoutingRole.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_074, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    server->coreService_ = AudioCoreService::GetCoreService();
    auto ret = server->ConfigDistributedRoutingRole(descriptor, CastType::CAST_TYPE_ALL);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_075
* @tc.desc  : Test SetDistributedRoutingRoleCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_075, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->audioPolicyServerHandler_ = nullptr;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetDistributedRoutingRoleCallback(object);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->SetDistributedRoutingRoleCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_076
* @tc.desc  : Test UnsetDistributedRoutingRoleCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_076, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->audioPolicyServerHandler_ = nullptr;
    auto ret = server->UnsetDistributedRoutingRoleCallback();
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->UnsetDistributedRoutingRoleCallback();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_077
* @tc.desc  : Test RegisterPowerStateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_077, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->RegisterPowerStateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_078
* @tc.desc  : Test UnRegisterPowerStateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_078, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->powerStateListener_ = nullptr;
    server->UnRegisterPowerStateListener();

    server->powerStateListener_ = new PowerStateListener(server);
    server->UnRegisterPowerStateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_079
* @tc.desc  : Test RegisterAppStateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_079, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->appStateListener_ = nullptr;
    server->RegisterAppStateListener();

    server->appStateListener_ = new AppStateListener();
    server->RegisterAppStateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_080
* @tc.desc  : Test RegisterSyncHibernateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_080, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->syncHibernateListener_ = nullptr;
    server->RegisterSyncHibernateListener();

    server->syncHibernateListener_ = new SyncHibernateListener(server);
    server->RegisterSyncHibernateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_081
* @tc.desc  : Test UnRegisterSyncHibernateListener.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_081, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->syncHibernateListener_ = nullptr;
    server->UnRegisterSyncHibernateListener();

    server->syncHibernateListener_ = new SyncHibernateListener(server);
    server->UnRegisterSyncHibernateListener();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_082
* @tc.desc  : Test IsSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_082, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsSpatializationEnabled(ret);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_083
* @tc.desc  : Test IsSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_083, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string address = "test";
    auto ret = false;
    server->IsSpatializationEnabled(address, ret);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_084
* @tc.desc  : Test SetSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_084, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->SetSpatializationEnabled(true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_085
* @tc.desc  : Test SetSpatializationEnabled.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_085, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    auto ret = server->SetSpatializationEnabled(selectedAudioDevice, true);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_086
* @tc.desc  : Test UpdateTracker.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_086, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioStreamChangeInfo streamChangeInfo;
    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_PAUSED;
    auto ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);

    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;
    ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);

    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_RELEASED;
    ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);

    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_RUNNING;
    ret = server->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_087
* @tc.desc  : Test FetchOutputDeviceForTrack.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_087, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioStreamChangeInfo streamChangeInfo;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    server->FetchOutputDeviceForTrack(streamChangeInfo, reason);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_088
* @tc.desc  : Test FetchInputDeviceForTrack.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_088, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioStreamChangeInfo streamChangeInfo;
    server->FetchInputDeviceForTrack(streamChangeInfo);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_093
* @tc.desc  : Test GetNetworkIdByGroupId.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_093, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t groupId = 0;
    std::string networkId = "test";
    auto ret = server->GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(ret, ERROR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_094
* @tc.desc  : Test SetSystemSoundUri.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_094, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string key = "test";
    std::string uri = "test";
    auto ret = server->SetSystemSoundUri(key, uri);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_095
* @tc.desc  : Test GetSystemSoundUri.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_095, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string key = "test";
    std::string ret;
    server->GetSystemSoundUri(key, ret);
    EXPECT_EQ(ret, "");
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_096
* @tc.desc  : Test GetMaxRendererInstances.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_096, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t ret = 0;
    server->isFirstAudioServiceStart_.store(true);
    server->GetMaxRendererInstances(ret);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_097
* @tc.desc  : Test GetMaxRendererInstances.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_097, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t ret = 0;
    server->isFirstAudioServiceStart_.store(false);
    server->GetMaxRendererInstances(ret);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_098
* @tc.desc  : Test GetPreferredOutputDeviceDescriptors.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_098, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioRendererInfo rendererInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    server->GetPreferredOutputDeviceDescriptors(rendererInfo, true, deviceDescs);
    server->GetPreferredOutputDeviceDescriptors(rendererInfo, false, deviceDescs);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_099
* @tc.desc  : Test SetCallbackRendererInfo.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_099, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioRendererInfo rendererInfo;
    server->audioPolicyServerHandler_ = nullptr;
    auto ret = server->SetCallbackRendererInfo(rendererInfo);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->SetCallbackRendererInfo(rendererInfo);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_100
* @tc.desc  : Test SetInputDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_100, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    uint32_t sessionID = 0;
    auto ret = server->SetInputDevice(DeviceType::DEVICE_TYPE_EARPIECE, sessionID,
        SourceType::SOURCE_TYPE_INVALID, true);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_101
* @tc.desc  : Test GetDmDeviceType.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_101, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    uint16_t deviceType = 0;
    server->GetDmDeviceType(deviceType);
    EXPECT_EQ(deviceType, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_102
* @tc.desc  : Test SetRingerModeInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_102, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    auto ret = server->SetRingerModeInner(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetRingerModeInner(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_103
* @tc.desc  : Test SetRingerModeInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_103, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->SetMicrophoneMuteCommon(true, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_104
* @tc.desc  : Test IsMicrophoneMuteLegacy.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_104, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool ret = false;
    server->IsMicrophoneMuteLegacy(ret);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_105
* @tc.desc  : Test UnsetAudioInterruptCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_105, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    uint32_t sessionID = 0;
    int32_t zoneID = 0;
    server->interruptService_ = nullptr;
    auto ret = server->UnsetAudioInterruptCallback(sessionID, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->UnsetAudioInterruptCallback(sessionID, zoneID);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_106
* @tc.desc  : Test SetQueryClientTypeCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_106, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetQueryClientTypeCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_107
* @tc.desc  : Test SetAudioClientInfoMgrCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_107, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetAudioClientInfoMgrCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_108
* @tc.desc  : Test SetQueryBundleNameListCallback.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_108, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = server->SetQueryBundleNameListCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_109
* @tc.desc  : Test RequestAudioFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_109, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    server->interruptService_ = nullptr;
    auto ret = server->RequestAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->RequestAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_120
* @tc.desc  : Test AbandonAudioFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_120, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    server->interruptService_ = nullptr;
    auto ret = server->AbandonAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->AbandonAudioFocus(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_121
* @tc.desc  : Test ProcessRemoteInterrupt.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_121, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::set<int32_t> sessionIds;
    InterruptEventInternal interruptEvent;
    server->interruptService_ = nullptr;
    server->ProcessRemoteInterrupt(sessionIds, interruptEvent);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    server->ProcessRemoteInterrupt(sessionIds, interruptEvent);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_122
* @tc.desc  : Test GetStreamInFocusByUid.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_122, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t uid = 0;
    int32_t zoneID = 0;
    int32_t ret = 0;
    server->GetStreamInFocusByUid(uid, zoneID, ret);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_123
* @tc.desc  : Test GetSessionInfoInFocus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_123, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioInterrupt audioInterrupt;
    int32_t zoneID = 0;
    server->interruptService_ = nullptr;
    auto ret = server->GetSessionInfoInFocus(audioInterrupt, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->GetSessionInfoInFocus(audioInterrupt, zoneID);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_124
* @tc.desc  : Test GetAudioFocusInfoList.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_124, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList;
    int32_t zoneID = 0;
    server->interruptService_ = nullptr;
    auto ret = server->GetAudioFocusInfoList(focusInfoList, zoneID);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    server->interruptService_ = std::make_shared<AudioInterruptService>();
    ret = server->GetAudioFocusInfoList(focusInfoList, zoneID);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_125
* @tc.desc  : Test AdjustSystemVolumeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_125, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->AdjustSystemVolumeByStep(AudioVolumeType::STREAM_ALL, VolumeAdjustType::VOLUME_UP);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_126
* @tc.desc  : Test SetStreamMuteInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_126, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    auto ret = server->SetStreamMuteInternal(AudioStreamType::STREAM_ALARM, false, true, deviceType);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetStreamMuteInternal(AudioStreamType::STREAM_ALL, false, true, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_127
* @tc.desc  : Test UpdateSystemMuteStateAccordingMusicState.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_127, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->UpdateSystemMuteStateAccordingMusicState(AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, false, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_128
* @tc.desc  : Test SendMuteKeyEventCbWithUpdateUiOrNot.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_128, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool isUpdateUi = true;
    server->audioPolicyServerHandler_ = nullptr;
    server->SendMuteKeyEventCbWithUpdateUiOrNot(AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    server->SendMuteKeyEventCbWithUpdateUiOrNot(AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_129
* @tc.desc  : Test SetSingleStreamMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_129, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    server->SetSingleStreamMute(AudioStreamType::STREAM_MUSIC, false, true, deviceType);
    server->SetSingleStreamMute(AudioStreamType::STREAM_RING, false, true, deviceType);
    server->SetSingleStreamMute(AudioStreamType::STREAM_RING, true, true, deviceType);
    server->SetSingleStreamMute(AudioStreamType::STREAM_RING, true, false, deviceType);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_130
* @tc.desc  : Test ProcUpdateRingerModeForMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_130, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    server->ProcUpdateRingerModeForMute(false, true);
    server->supportVibrator_ = false;
    server->ProcUpdateRingerModeForMute(true, true);

    server->supportVibrator_ = true;
    server->ProcUpdateRingerModeForMute(true, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_131
* @tc.desc  : Test SetSelfAppVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_131, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 0;
    int32_t volumeFlag = 0;
    auto ret = server->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_132
* @tc.desc  : Test IsAppVolumeMute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_132, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    bool owned = true;
    bool isMute = false;
    auto ret = server->IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_133
* @tc.desc  : Test SendVolumeKeyEventCbWithUpdateUiOrNot.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_133, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    bool isUpdateUi = true;
    server->audioPolicyServerHandler_ = nullptr;
    server->SendVolumeKeyEventCbWithUpdateUiOrNot(AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    server->SendVolumeKeyEventCbWithUpdateUiOrNot(AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, isUpdateUi);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_134
* @tc.desc  : Test ProcUpdateRingerMode.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_134, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = std::make_shared<AudioCoreService>();
    server->supportVibrator_ = true;
    server->ProcUpdateRingerMode();

    server->supportVibrator_ = false;
    server->ProcUpdateRingerMode();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_135
* @tc.desc  : Test SetAppSingleStreamVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_135, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeLevel = 1;
    server->audioPolicyServerHandler_ = nullptr;
    auto ret = server->SetAppSingleStreamVolume(appUid, volumeLevel, true);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = server->SetAppSingleStreamVolume(appUid, volumeLevel, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_136
* @tc.desc  : Test SetSingleStreamVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_136, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    auto ret = server->SetSingleStreamVolume(AudioStreamType::STREAM_VOICE_ASSISTANT, volumeLevel, true, false);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetSingleStreamVolume(AudioStreamType::STREAM_RING, volumeLevel, true, false);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetSingleStreamVolume(AudioStreamType::STREAM_VOICE_RING, volumeLevel, true, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_138
* @tc.desc  : Test GetStreamMuteInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_138, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->GetStreamMuteInternal(AudioStreamType::STREAM_ALL);
    EXPECT_EQ(ret, false);

    ret = server->GetStreamMuteInternal(AudioStreamType::STREAM_RING);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_139
* @tc.desc  : Test GetDevices.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_139, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(server->coreService_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetDevices(DeviceFlag::NONE_DEVICES_FLAG, ret);
    EXPECT_EQ(ret.size(), 0);

    DeviceFlag deviceFlag = static_cast<DeviceFlag>(-1);
    server->GetDevices(DeviceFlag::NONE_DEVICES_FLAG, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_140
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_140, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetDevicesInner(DeviceFlag::ALL_DEVICES_FLAG, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_141
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_141, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetOutputDevice(audioRendererFilter, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_142
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_142, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetOutputDevice(audioRendererFilter, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_143
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_143, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    server->GetInputDevice(audioCapturerFilter, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_144
* @tc.desc  : Test SubscribePowerStateChangeEvents.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_144, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->SubscribePowerStateChangeEvents();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_145
* @tc.desc  : Test OnReceiveEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_145, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    EventFwk::CommonEventData eventData;
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    server->OnReceiveEvent(eventData);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_146
* @tc.desc  : Test SubscribeCommonEventExecute.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_146, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->SubscribeCommonEventExecute();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_147
* @tc.desc  : Test CheckSubscribePowerStateChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_147, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->powerStateCallbackRegister_ = true;
    server->CheckSubscribePowerStateChange();
    server->powerStateCallbackRegister_ = false;
    server->CheckSubscribePowerStateChange();
    EXPECT_TRUE(server->powerStateCallbackRegister_);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_148
* @tc.desc  : Test CheckSubscribePowerStateChange.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_148, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    auto ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_TTS, volumeLevel);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = -1;
    ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = server->audioVolumeManager_.GetMinVolumeLevel(AudioStreamType::STREAM_RING);
    ret = server->SetSystemVolumeLevelLegacy(AudioStreamType::STREAM_RING, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_149
* @tc.desc  : Test SetAppVolumeMuted.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_149, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;
    auto ret = server->SetAppVolumeMuted(appUid, true, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_150
* @tc.desc  : Test SetAppVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_150, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeLevel = 1;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;
    auto ret = server->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    volumeLevel = server->audioVolumeManager_.GetMinVolumeLevel(AudioStreamType::STREAM_APP);
    ret = server->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_151
* @tc.desc  : Test SetSystemVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_151, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;

    auto ret = server->SetSystemVolumeLevel(AudioStreamType::STREAM_WAKEUP, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = -1;
    ret = server->SetSystemVolumeLevel(AudioStreamType::STREAM_MUSIC, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = server->audioVolumeManager_.GetMinVolumeLevel(AudioStreamType::STREAM_MUSIC);
    ret = server->SetSystemVolumeLevel(AudioStreamType::STREAM_MUSIC, volumeLevel, volumeFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_152
* @tc.desc  : Test SetSystemVolumeLevelWithDevice.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_152, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    int32_t volumeFlag = VolumeFlag::FLAG_SHOW_SYSTEM_UI;

    auto ret = server->SetSystemVolumeLevelWithDevice(AudioStreamType::STREAM_WAKEUP, volumeLevel,
        DeviceType::DEVICE_TYPE_EARPIECE, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    volumeLevel = -1;
    ret = server->SetSystemVolumeLevelWithDevice(AudioStreamType::STREAM_MUSIC, volumeLevel,
        DeviceType::DEVICE_TYPE_EARPIECE, volumeFlag);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_153
* @tc.desc  : Test GetSystemActiveVolumeTypeInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_153, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t clientUid = 0;
    auto ret = server->GetSystemActiveVolumeTypeInternal(clientUid);
    EXPECT_EQ(ret, AudioStreamType::STREAM_MUSIC);

    clientUid = 1;
    ret = server->GetSystemActiveVolumeTypeInternal(clientUid);
    EXPECT_EQ(ret, AudioStreamType::STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_154
* @tc.desc  : Test GetAppVolumeLevel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_154, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t appUid = 0;
    int32_t volumeLevel = 1;
    auto ret = server->GetAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_155
* @tc.desc  : Test GetSystemVolumeLevelInternal.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_155, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->GetSystemVolumeLevelInternal(AudioStreamType::STREAM_ALL);
    server->GetSystemVolumeLevelInternal(AudioStreamType::STREAM_VOICE_RING);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_156
* @tc.desc  : Test SetLowPowerVolume.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_156, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamId = 0;
    float volume = 0.5f;
    auto ret = server->SetLowPowerVolume(streamId, volume);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_157
* @tc.desc  : Test CheckCanMuteVolumeTypeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_157, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    server->volumeStep_ = 1;
    auto ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_ASSISTANT, volumeLevel);
    EXPECT_EQ(ret, false);

    server->volumeStep_ = 0;
    ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_ASSISTANT, volumeLevel);
    EXPECT_EQ(ret, true);

    ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(ret, true);

    server->volumeStep_ = 1;
    ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_158
* @tc.desc  : Test CheckCanMuteVolumeTypeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_158, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volumeLevel = 1;
    server->volumeStep_ = 1;
    auto ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_CALL, volumeLevel);
    EXPECT_EQ(ret, false);

    ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_ALARM, volumeLevel);
    EXPECT_EQ(ret, false);

    ret = server->CheckCanMuteVolumeTypeByStep(AudioVolumeType::STREAM_VOICE_COMMUNICATION, volumeLevel);
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_159
* @tc.desc  : Test AdjustVolumeByStep.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_159, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    auto ret = server->AdjustVolumeByStep(VolumeAdjustType::VOLUME_UP);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->AdjustVolumeByStep(VolumeAdjustType::VOLUME_DOWN);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_160
* @tc.desc  : Test TranslateKeyEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_160, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    int32_t resultOfVolumeKey = 0;
    server->SendMonitrtEvent(keyType, resultOfVolumeKey);

    keyType = OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN;
    server->SendMonitrtEvent(keyType, resultOfVolumeKey);

    keyType = OHOS::MMI::KeyEvent::KEYCODE_MUTE;
    server->SendMonitrtEvent(keyType, resultOfVolumeKey);

    keyType = -1;
    server->SendMonitrtEvent(keyType, resultOfVolumeKey);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_161
* @tc.desc  : Test OnAddSystemAbilityExtract.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_161, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t systemAbilityId = APP_MGR_SERVICE_ID;
    std::string deviceId = "test";
    server->OnAddSystemAbilityExtract(systemAbilityId, deviceId);

    systemAbilityId = -1;
    server->OnAddSystemAbilityExtract(systemAbilityId, deviceId);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_162
* @tc.desc  : Test HandleKvDataShareEvent.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_162, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->isInitMuteState_.store(false);
    server->HandleKvDataShareEvent();

    server->isInitMuteState_.store(true);
    server->HandleKvDataShareEvent();
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_163
* @tc.desc  : Test SubscribeVolumeKeyEvents.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_163, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->hasSubscribedVolumeKeyEvents_.store(false);
    server->SubscribeVolumeKeyEvents();

    server->hasSubscribedVolumeKeyEvents_.store(true);
    server->SubscribeVolumeKeyEvents();
}
#endif

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_164
* @tc.desc  : Test SubscribeOsAccountChangeEvents.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_164, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->accountObserver_ = nullptr;
    server->SubscribeOsAccountChangeEvents();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_165
* @tc.desc  : Test AddAudioServiceOnStart.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_165, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    server->isFirstAudioServiceStart_.store(true);
    server->AddAudioServiceOnStart();

    server->coreService_ = AudioCoreService::GetCoreService();
    server->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(server->coreService_);
    server->isFirstAudioServiceStart_.store(false);
    server->AddAudioServiceOnStart();
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_0166
* @tc.desc  : Test SetDeviceConnectionStatus.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_166, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->deviceName_ = "Speaker_Out";
    desc->deviceRole_ = OUTPUT_DEVICE;

    bool isConnected = true;
    int32_t ret = server->SetDeviceConnectionStatus(desc, isConnected);
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_167
* @tc.desc  : Test GetDirectPlaybackSupport.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_167, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.encoding = ENCODING_PCM;
    streamInfo.format = SAMPLE_S24LE;
    streamInfo.channels = STEREO;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t result = 0;
    server->GetDirectPlaybackSupport(streamInfo, streamUsage, result);
    EXPECT_EQ(result, DIRECT_PLAYBACK_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test AudioPolicyServer.
 * @tc.number: AudioPolicyServer_168
 * @tc.desc  : Test ActivatePreemptMode.
 */
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_168, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t result = server->ActivatePreemptMode();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: IsStreamActiveByStreamUsage_001
 * @tc.desc  : AudioPolicyServer::IsStreamActiveByStreamUsage
 */
HWTEST(AudioPolicyUnitTest, IsStreamActiveByStreamUsage_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamUsage = static_cast<int32_t>(StreamUsage::STREAM_USAGE_MUSIC);
    bool isStreamActive = true;

    int32_t ret = server->IsStreamActiveByStreamUsage(streamUsage, isStreamActive);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetVolumeInDbByStream_001
 * @tc.desc  : AudioPolicyServer::GetVolumeInDbByStream
 */
HWTEST(AudioPolicyUnitTest, GetVolumeInDbByStream_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamUsage = static_cast<int32_t>(StreamUsage::STREAM_USAGE_MUSIC);
    int32_t volLevel = 5;
    int32_t deviceType = static_cast<int32_t>(DeviceType::DEVICE_TYPE_SPEAKER);
    float volDb = 0;

    int32_t ret = server->GetVolumeInDbByStream(streamUsage, volLevel, deviceType, volDb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetSupportedAudioVolumeTypes_001
 * @tc.desc  : AudioPolicyServer::GetSupportedAudioVolumeTypes
 */
HWTEST(AudioPolicyUnitTest, GetSupportedAudioVolumeTypes_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::vector<int32_t> audioVolumeTypes = {};

    int32_t ret = server->GetSupportedAudioVolumeTypes(audioVolumeTypes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetAudioVolumeTypeByStreamUsage_001
 * @tc.desc  : AudioPolicyServer::GetAudioVolumeTypeByStreamUsage
 */
HWTEST(AudioPolicyUnitTest, GetAudioVolumeTypeByStreamUsage_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t streamUsage = static_cast<int32_t>(StreamUsage::STREAM_USAGE_MUSIC);
    int32_t audioVolumeType = static_cast<int32_t>(AudioVolumeType::STREAM_DEFAULT);

    int32_t ret = server->GetAudioVolumeTypeByStreamUsage(streamUsage, audioVolumeType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: GetStreamUsagesByVolumeType_001
 * @tc.desc  : AudioPolicyServer::GetStreamUsagesByVolumeType
 */
HWTEST(AudioPolicyUnitTest, GetStreamUsagesByVolumeType_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    int32_t volType = static_cast<int32_t>(AudioVolumeType::STREAM_MUSIC);
    std::vector<int32_t> streamUsages = {};

    int32_t ret = server->GetStreamUsagesByVolumeType(volType, streamUsages);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyServer
 * @tc.number: SetQueryDeviceVolumeBehaviorCallback_001
 * @tc.desc  : AudioPolicyServer::SetQueryDeviceVolumeBehaviorCallback
 */
HWTEST(AudioPolicyUnitTest, SetQueryDeviceVolumeBehaviorCallback_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    sptr<IRemoteObject> object = nullptr;

    int32_t ret = server->SetQueryDeviceVolumeBehaviorCallback(object);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    object = new RemoteObjectTestStub();

    ret = server->SetQueryDeviceVolumeBehaviorCallback(object);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: SetDeviceVolumeBehavior_001
* @tc.desc  : Test SetDeviceVolumeBehavior.
*/
HWTEST(AudioPolicyUnitTest, SetDeviceVolumeBehavior_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string networkId = "test";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    VolumeBehavior volumeBehavior;
    int32_t result = server->audioDeviceManager_.SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: GetDeviceVolumeBehavior_001
* @tc.desc  : Test GetDeviceVolumeBehavior.
*/
HWTEST(AudioPolicyUnitTest, GetDeviceVolumeBehavior_001, TestSize.Level1)
{
    sptr<AudioPolicyServer> server = GetPolicyServerUnitTest();
    ASSERT_TRUE(server != nullptr);

    std::string networkId = "test";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    VolumeBehavior volumeBehavior = server->audioDeviceManager_.GetDeviceVolumeBehavior(networkId, deviceType);
    EXPECT_EQ(volumeBehavior.isReady, false);
}
} // AudioStandard
} // OHOS
