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

#include "audio_policy_server_unit_test.h"
#include "audio_policy_server.h"
#include "audio_interrupt_unit_test.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "securec.h"
#include "audio_interrupt_service.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyUnitTest::SetUpTestCase(void) {}
void AudioPolicyUnitTest::TearDownTestCase(void) {}
void AudioPolicyUnitTest::SetUp(void) {}

void AudioPolicyUnitTest::TearDown(void) {}

sptr<AudioPolicyServer> GetPolicyServerUnitTest()
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server =
        sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    return server;
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
* @tc.number: AudioPolicyServer_007
* @tc.desc  : Test ReconfigureAudioChannel.
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_007, TestSize.Level1)
{
    #ifdef AUDIO_BUILD_VARIANT_ROOT
    #undef AUDIO_BUILD_VARIANT_ROOT
    #endif
    auto policyServerTest = GetPolicyServerUnitTest();
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(systemAbilityId, runOnCreate);
    uint32_t count = 2;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    int32_t result = server->ReconfigureAudioChannel(count, deviceType);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
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
    AudioSessionStrategy strategy;
    auto policyServerTest = GetPolicyServerUnitTest();
    EXPECT_EQ(policyServerTest->ActivateAudioSession(strategy), ERR_UNKNOWN);
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
    AudioStreamType result = policyServerTest->GetStreamInFocus(zoneID);
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
    int32_t result = policyServerTest->ActivateAudioInterrupt(audioInterrupt, zoneID);
    EXPECT_EQ(result, ERR_UNKNOWN);
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

    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();
    ptrAudioPolicyServer->IsAudioSessionActivated();
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

    ptrAudioPolicyServer->interruptService_ = nullptr;
    ptrAudioPolicyServer->IsAudioSessionActivated();
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

    const AudioSessionStrategy strategy;
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

    const AudioSessionStrategy strategy;
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

    const AudioSessionStrategy strategy = {AudioConcurrencyMode::SLIENT};
    ptrAudioPolicyServer->interruptService_ = std::make_shared<AudioInterruptService>();

    auto ret = ptrAudioPolicyServer->ActivateAudioSession(strategy);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
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

    const AudioSessionStrategy strategy = {AudioConcurrencyMode::DEFAULT};
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

    auto ret = ptrAudioPolicyServer->IsHighResolutionExist();

    EXPECT_EQ(ret, false);
}

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

    EXPECT_EQ(ret, SUCCESS);
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

    auto ret = ptrAudioPolicyServer->GetSpatializationSceneType();

    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_036
* @tc.desc  : Test AudioPolicyServer::GetSpatializationSceneType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_036, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    ptrAudioPolicyServer->GetActiveBluetoothDevice();
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
    auto ret = ptrAudioPolicyServer->SetCallDeviceActive(deviceType, active, address);

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
    auto ret = ptrAudioPolicyServer->SetCallDeviceActive(deviceType, active, address);

    EXPECT_NE(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

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

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
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

    AudioStreamType streamType;
    int32_t clientUid = 0;
    streamType = ptrAudioPolicyServer->GetSystemActiveVolumeType(clientUid);
    EXPECT_EQ(streamType, STREAM_MUSIC);
    clientUid = 1;
    streamType = ptrAudioPolicyServer->GetSystemActiveVolumeType(clientUid);
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
    EXPECT_EQ(res, 0);
    streamType = STREAM_MUSIC;
    res = ptrAudioPolicyServer->GetSystemVolumeLevelNoMuteState(streamType);
    EXPECT_EQ(res, 0);
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

    AudioStreamType streamType = STREAM_RING;
    bool ret = ptrAudioPolicyServer->GetStreamMute(streamType);
    EXPECT_EQ(ret, false);
    streamType = STREAM_VOICE_RING;
    ret = ptrAudioPolicyServer->GetStreamMute(streamType);
    EXPECT_EQ(ret, false);
    streamType = STREAM_MUSIC;
    ret = ptrAudioPolicyServer->GetStreamMute(streamType);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_044
* @tc.desc  : Test AudioPolicyServer::GetPreferredOutputStreamType
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_044, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    AudioRendererInfo rendererInfo;
    int32_t ret = ptrAudioPolicyServer->GetPreferredOutputStreamType(rendererInfo);
    EXPECT_EQ(ret, 0);
    ptrAudioPolicyServer->audioPolicyService_.isFastControlled_ = true;
    ret = ptrAudioPolicyServer->GetPreferredOutputStreamType(rendererInfo);
    EXPECT_EQ(ret, 0);
    rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    ret = ptrAudioPolicyServer->GetPreferredOutputStreamType(rendererInfo);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioPolicyServer.
* @tc.number: AudioPolicyServer_045
* @tc.desc  : Test AudioPolicyServer::IsAllowedPlayback
*/
HWTEST(AudioPolicyUnitTest, AudioPolicyServer_045, TestSize.Level1)
{
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto ptrAudioPolicyServer = std::make_shared<AudioPolicyServer>(systemAbilityId, runOnCreate);

    EXPECT_NE(ptrAudioPolicyServer, nullptr);

    int32_t uid = 0;
    int32_t pid = 0;
    bool ret = ptrAudioPolicyServer->IsAllowedPlayback(uid, pid);
    EXPECT_EQ(ret, true);
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
} // AudioStandard
} // OHOS
