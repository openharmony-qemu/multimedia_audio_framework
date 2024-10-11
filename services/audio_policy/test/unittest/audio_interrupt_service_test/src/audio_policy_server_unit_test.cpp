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
    EXPECT_EQ(result, SUCCESS);
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
    AudioStreamType result = policyServerTest->GetStreamInFocus(zoneID);
    EXPECT_EQ(result, -62980296);
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
}
}
