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

#include "audio_policy_service_first_unit_test.h"
#include "audio_policy_service_thirdext_unit_test.h"
#include "nativetoken_kit.h"
#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermissioned = false;

const int32_t TEST_SESSIONID = MIN_SESSIONID + 1010;
const int32_t A2DP_STOPPED = 1;
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_INVALID = 3;
const uint32_t ROTATE = 1;
const int32_t SESSION_ID = 1000001;
const int32_t STATE = 1;

enum PerferredType {
    MEDIA_RENDER = 0,
    CALL_RENDER = 1,
    CALL_CAPTURE = 2,
    RING_RENDER = 3,
    RECORD_CAPTURE = 4,
    TONE_RENDER = 5,
};

void AudioPolicyServiceFourthUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceFourthUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceFourthUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUp start-end");
}
void AudioPolicyServiceFourthUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDown start-end");
}

static void GetPermission()
{
    if (!g_hasPermissioned) {
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
            .processName = "audio_policy_service_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermissioned = true;
    }
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, AudioPolicyServiceTest_Prepare001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest AudioPolicyServiceTest_Prepare001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    // Get permission for test
    GetPermission();

    // Call OnServiceConnected for HDI_SERVICE_INDEX
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.OnServiceConnected(HDI_SERVICE_INDEX);
}

/**
* @tc.name  : Test GetSupportedAudioEffectProperty.
* @tc.number: GetSupportedAudioEffectProperty_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetSupportedAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetSupportedAudioEffectProperty_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioEffectPropertyArray propertyArray;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
}

/**
* @tc.name  : Test LoadHdiEffectModel.
* @tc.number: LoadHdiEffectModel_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, LoadHdiEffectModel_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest LoadHdiEffectModel_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.LoadHdiEffectModel();
}

/**
* @tc.name  : Test OnReceiveBluetoothEvent.
* @tc.number: OnReceiveBluetoothEvent_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnReceiveBluetoothEvent_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnReceiveBluetoothEvent_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    const std::string macAddress = "11-11-11-11-11-11";
    const std::string deviceName = "deviceName";
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.OnReceiveBluetoothEvent(macAddress, deviceName);
}

/**
* @tc.name  : Test WaitForConnectionCompleted.
* @tc.number: WaitForConnectionCompleted_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, WaitForConnectionCompleted_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest WaitForConnectionCompleted_001 start");
    EXPECT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.WaitForConnectionCompleted();
    EXPECT_FALSE(!(audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED));
}

/**
* @tc.name  : Test WaitForConnectionCompleted.
* @tc.number: WaitForConnectionCompleted_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, WaitForConnectionCompleted_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest WaitForConnectionCompleted_001 start");
    EXPECT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.WaitForConnectionCompleted();
    EXPECT_FALSE(!(audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED));
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_001 start");
    EXPECT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(0);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_001 start");
    EXPECT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(1);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test IsA2dpOffloadConnecting.
* @tc.number: IsA2dpOffloadConnecting_003
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsA2dpOffloadConnecting_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsA2dpOffloadConnecting_001 start");
    EXPECT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.connectionTriggerSessionIds_ = {0};

    bool ret = audioA2dpOffloadManager_.IsA2dpOffloadConnecting(0);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test GetAudioEffectOffloadFlag.
* @tc.number: GetAudioEffectOffloadFlag_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetAudioEffectOffloadFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetAudioEffectOffloadFlag_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.GetAudioEffectOffloadFlag();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test CheckSpatializationAndEffectState.
* @tc.number: CheckSpatializationAndEffectState_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, CheckSpatializationAndEffectState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest CheckSpatializationAndEffectState_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.audioOffloadStream_.CheckSpatializationAndEffectState();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test UpdateRoute.
* @tc.number: UpdateRoute_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateRoute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateRoute_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    outputDevices.push_back(std::move(audioDeviceDescriptor));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor1->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    outputDevices.push_back(std::move(audioDeviceDescriptor1));
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    outputDevices.push_back(std::move(audioDeviceDescriptor2));
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);
    audioDeviceDescriptor.reset();
    audioDeviceDescriptor1.reset();
    audioDeviceDescriptor2.reset();
    rendererChangeInfo.reset();
}

/**
* @tc.name  : Test UpdateRoute.
* @tc.number: UpdateRoute_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateRoute_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateRoute_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_DP;
    outputDevices.push_back(std::move(audioDeviceDescriptor));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);

    server->audioPolicyService_.audioDeviceCommon_.enableDualHalToneState_ = true;
    server->audioPolicyService_.audioDeviceCommon_.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, server->audioPolicyService_.audioVolumeManager_.ringerModeMute_);
    audioDeviceDescriptor.reset();
    rendererChangeInfo.reset();
}

/**
* @tc.name  : Test LoadSplitModule.
* @tc.number: LoadSplitModule_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, LoadSplitModule_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest LoadSplitModule_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    const std::string networkIdOne = "";
    const std::string splitArgOne = "";
    int32_t result = AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.LoadSplitModule(
        splitArgOne, networkIdOne);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    const std::string networkIdTwo = LOCAL_NETWORK_ID;
    const std::string splitArgTwo = "11.22";
    result = AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.LoadSplitModule(
        splitArgTwo, networkIdTwo);
    EXPECT_EQ(ERR_INVALID_HANDLE, result);
}

/**
* @tc.name  : Test SetDefaultOutputDevice.
* @tc.number: SetDefaultOutputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetDefaultOutputDevice_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    bool isRunning = true;
    const StreamUsage streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    const uint32_t sessionID = 0;

    int32_t result =
        AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.SetDefaultOutputDevice(
            deviceType, sessionID, streamUsage, isRunning);
    EXPECT_EQ(SUCCESS, result);
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.hasEarpiece_ = true;
    result =
        AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.SetDefaultOutputDevice(
            deviceType, sessionID, streamUsage, isRunning);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test UpdateDefaultOutputDeviceWhenStopping.
* @tc.number: UpdateDefaultOutputDeviceWhenStopping_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateDefaultOutputDeviceWhenStopping_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateDefaultOutputDeviceWhenStopping_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    int32_t uid = getuid();
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.UpdateDefaultOutputDeviceWhenStopping(uid);
    EXPECT_EQ(SUCCESS, uid);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_STOPPED;
    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(A2DP_STOPPED, playingState);
    EXPECT_EQ(CONNECTION_STATUS_CONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_002 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_STOPPED;
    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_003 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_004
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_004 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "test";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_005
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_005 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTING;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_CONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_006
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_006, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_006 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_PLAYING;
    audioA2dpOffloadManager_.currentOffloadConnectionState_ = CONNECTION_STATUS_CONNECTED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_CONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_007
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_007 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_STOPPED;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_EQ(CONNECTION_STATUS_DISCONNECTED, audioA2dpOffloadManager_.currentOffloadConnectionState_);
}

/**
* @tc.name  : Test OnA2dpPlayingStateChanged.
* @tc.number: OnA2dpPlayingStateChanged_008
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, OnA2dpPlayingStateChanged_008, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest OnA2dpPlayingStateChanged_008 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    AudioA2dpOffloadManager audioA2dpOffloadManager_(
        &(AudioPolicyServiceUnitTest::GetServerPtr())->audioPolicyService_);

    const std::string deviceAddress = "";
    int32_t playingState = A2DP_INVALID;
    audioA2dpOffloadManager_.OnA2dpPlayingStateChanged(deviceAddress, playingState);
    EXPECT_NE(A2DP_STOPPED, playingState);
    EXPECT_NE(A2DP_PLAYING, playingState);
}

/**
* @tc.name  : Test GetSupportedAudioEnhanceProperty.
* @tc.number: GetSupportedAudioEnhanceProperty_001
* @tc.desc  : Test GetSupportedAudioEnhanceProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetSupportedAudioEnhanceProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetSupportedAudioEnhanceProperty_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioEnhancePropertyArray propertyArrayTest;
    AudioPolicyServiceUnitTest::GetServerPtr()->GetSupportedAudioEnhanceProperty(propertyArrayTest);
}

/**
* @tc.name  : Test SetAudioEffectProperty.
* @tc.number: SetAudioEffectProperty_001
* @tc.desc  : Test SetAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetAudioEffectProperty_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioEffectPropertyArray propertyArrayTest;
    AudioPolicyServiceUnitTest::GetServerPtr()->SetAudioEffectProperty(propertyArrayTest);
}

/**
* @tc.name  : Test GetAudioEffectProperty.
* @tc.number: GetAudioEffectProperty_001
* @tc.desc  : Test GetAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetAudioEffectProperty_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioEffectPropertyArray propertyArrayTest;
    AudioPolicyServiceUnitTest::GetServerPtr()->GetAudioEffectProperty(propertyArrayTest);
}

/**
* @tc.name  : Test SetAudioEnhanceProperty.
* @tc.number: SetAudioEnhanceProperty_001
* @tc.desc  : Test SetAudioEnhanceProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetAudioEnhanceProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetAudioEnhanceProperty_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioEnhancePropertyArray propertyArrayTest;
    AudioPolicyServiceUnitTest::GetServerPtr()->SetAudioEnhanceProperty(propertyArrayTest);
}

/**
* @tc.name  : Test GetAudioEnhanceProperty.
* @tc.number: GetAudioEnhanceProperty_001
* @tc.desc  : Test GetAudioEnhanceProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, GetAudioEnhanceProperty_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetAudioEnhanceProperty_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioEnhancePropertyArray propertyArrayTest;
    AudioPolicyServiceUnitTest::GetServerPtr()->GetAudioEnhanceProperty(propertyArrayTest);
}

/**
* @tc.name  : Test SetAudioConcurrencyCallback.
* @tc.number: SetAudioConcurrencyCallback_001
* @tc.desc  : Test SetAudioConcurrencyCallback interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetAudioConcurrencyCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetAudioConcurrencyCallback_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    sptr<IRemoteObject> objectSptrTest = nullptr;
    AudioPolicyServiceUnitTest::GetServerPtr()->SetAudioConcurrencyCallback(TEST_SESSIONID, objectSptrTest);
}

/**
* @tc.name  : Test UnsetAudioConcurrencyCallback.
* @tc.number: UnsetAudioConcurrencyCallback_001
* @tc.desc  : Test UnsetAudioConcurrencyCallback interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UnsetAudioConcurrencyCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UnsetAudioConcurrencyCallback_001 start");
    AudioPolicyServer* server = AudioPolicyServiceUnitTest::GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<IRemoteObject> objectSptrTest = nullptr;
    server->SetAudioConcurrencyCallback(TEST_SESSIONID, objectSptrTest);
    server->UnsetAudioConcurrencyCallback(TEST_SESSIONID);
}

/**
* @tc.name  : Test ActivateAudioConcurrency.
* @tc.number: ActivateAudioConcurrency_001
* @tc.desc  : Test ActivateAudioConcurrency interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ActivateAudioConcurrency_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ActivateAudioConcurrency_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioPolicyServiceUnitTest::GetServerPtr()->ActivateAudioConcurrency(PIPE_TYPE_UNKNOWN);
}

/**
* @tc.name  : Test ResetRingerModeMute.
* @tc.number: ResetRingerModeMute_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ResetRingerModeMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ResetRingerModeMute_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = true;
    auto ret = AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.ResetRingerModeMute();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test ResetRingerModeMute.
* @tc.number: ResetRingerModeMute_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ResetRingerModeMute_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ResetRingerModeMute_002 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = false;
    auto ret = AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.ResetRingerModeMute();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test ErasePreferredDeviceByType.
* @tc.number: ErasePreferredDeviceByType_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ErasePreferredDeviceByType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ErasePreferredDeviceByType_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    AudioPolicyUtils::GetInstance().isBTReconnecting_ = true;
    const PreferredType preferredType = AUDIO_MEDIA_RENDER;
    auto ret = AudioPolicyUtils::GetInstance().ErasePreferredDeviceByType(preferredType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test ErasePreferredDeviceByType.
* @tc.number: ErasePreferredDeviceByType_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ErasePreferredDeviceByType_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ErasePreferredDeviceByType_002 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    const PreferredType preferredType = AUDIO_MEDIA_RENDER;
    auto ret = AudioPolicyUtils::GetInstance().ErasePreferredDeviceByType(preferredType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test UpdateSessionConnectionState.
* @tc.number: UpdateSessionConnectionState_001
* @tc.desc  : Test UpdateSessionConnectionState interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateSessionConnectionState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateSessionConnectionState_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);
    int32_t sessionID = SESSION_ID;
    int32_t state = STATE;
    server->audioPolicyService_.UpdateSessionConnectionState(sessionID, state);
    EXPECT_NE(nullptr, server->audioPolicyService_.GetAudioServerProxy());
}

/**
* @tc.name  : Test IsRingerOrAlarmerDualDevicesRange.
* @tc.number: IsRingerOrAlarmerDualDevicesRange_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsRingerOrAlarmerDualDevicesRange_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsRingerOrAlarmerDualDevicesRange_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);

    InternalDeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsRingerOrAlarmerDualDevicesRange.
* @tc.number: IsRingerOrAlarmerDualDevicesRange_002
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, IsRingerOrAlarmerDualDevicesRange_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest IsRingerOrAlarmerDualDevicesRange_002 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);

    InternalDeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_DP;
    ret = server->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test UpdateEffectBtOffloadSupported.
* @tc.number: UpdateEffectBtOffloadSupported_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, UpdateEffectBtOffloadSupported_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest UpdateEffectBtOffloadSupported_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);
    bool isSupported = false;
    server->audioPolicyService_.UpdateEffectBtOffloadSupported(isSupported);
    EXPECT_NE(nullptr, server->audioPolicyService_.GetAudioServerProxy());
}

/**
* @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
* @tc.number: ScoInputDeviceFetchedForRecongnition_001
* @tc.desc  : Test ScoInputDeviceFetchedForRecongnition interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, ScoInputDeviceFetchedForRecongnition_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest ScoInputDeviceFetchedForRecongnition_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);
    std::string address = "";

    bool handleFlag = false;
    ConnectState connectState = DEACTIVE_CONNECTED;
    int32_t result = server->audioPolicyService_.audioDeviceCommon_.ScoInputDeviceFetchedForRecongnition(handleFlag,
        address, connectState);
    EXPECT_EQ(SUCCESS, result);

    handleFlag = true;
    connectState = VIRTUAL_CONNECTED;
    result = server->audioPolicyService_.audioDeviceCommon_.ScoInputDeviceFetchedForRecongnition(handleFlag,
        address, connectState);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test SetRotationToEffect.
* @tc.number: SetRotationToEffect_001
* @tc.desc  : Test AudioPolicyServic interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetRotationToEffect_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetRotationToEffect_001 start");
    auto server = AudioPolicyServiceUnitTest::GetServerPtr();
    EXPECT_NE(nullptr, server);
    uint32_t rotate = ROTATE;
    server->audioPolicyService_.SetRotationToEffect(rotate);
    EXPECT_NE(nullptr, server->audioPolicyService_.GetAudioServerProxy());
}

/**
* @tc.name  : Test DealAudioSceneOutputDevices.
* @tc.number: DealAudioSceneOutputDevices_001
* @tc.desc  : Test IsA2dpOffloadConnected interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, DealAudioSceneOutputDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest DealAudioSceneOutputDevices_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    const AudioScene audioScene = AUDIO_SCENE_RINGING;
    std::vector<DeviceType> activeOutputDevices;
    bool haveArmUsbDevice = false;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioSceneManager_.DealAudioSceneOutputDevices(
        audioScene, activeOutputDevices, haveArmUsbDevice);
    EXPECT_EQ(false, haveArmUsbDevice);

    const AudioScene audioScene2 = AUDIO_SCENE_VOICE_RINGING;
    haveArmUsbDevice = false;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioSceneManager_.DealAudioSceneOutputDevices(
        audioScene2, activeOutputDevices, haveArmUsbDevice);
    EXPECT_EQ(false, haveArmUsbDevice);

    const AudioScene audioScene3 = AUDIO_SCENE_DEFAULT;
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs {};
    haveArmUsbDevice = false;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioSceneManager_.DealAudioSceneOutputDevices(
        audioScene3, activeOutputDevices, haveArmUsbDevice);
    EXPECT_TRUE(descs.empty());
    EXPECT_EQ(false, haveArmUsbDevice);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices.
* @tc.number: SelectRingerOrAlarmDevices_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SelectRingerOrAlarmDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SelectRingerOrAlarmDevices_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs1;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    bool result =
        AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(
        descs1, rendererChangeInfo1);
    EXPECT_EQ(false, result);

    vector<std::shared_ptr<AudioDeviceDescriptor>> descs2;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_SPEAKER;
    descs2.push_back(std::move(audioDeviceDescriptor2));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    rendererChangeInfo2->sessionId = TEST_SESSIONID;
    result =
        AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(
        descs2, rendererChangeInfo2);
    EXPECT_EQ(true, result);

    vector<std::shared_ptr<AudioDeviceDescriptor>> descs3;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor3 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descs3.push_back(std::move(audioDeviceDescriptor3));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    rendererChangeInfo3->sessionId = TEST_SESSIONID;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.enableDualHalToneState_ = true;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioPolicyManager_.SetRingerMode(
        RINGER_MODE_VIBRATE);
    result =
        AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(
        descs3, rendererChangeInfo3);
    EXPECT_EQ(true, result);

    vector<std::shared_ptr<AudioDeviceDescriptor>> descs4;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor4 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor4->deviceType_ = DEVICE_TYPE_EXTERN_CABLE;
    descs3.push_back(std::move(audioDeviceDescriptor4));
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    rendererChangeInfo4->sessionId = TEST_SESSIONID;
    AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.enableDualHalToneState_ = true;
    result =
        AudioPolicyServiceUnitTest::GetServerPtr()->audioPolicyService_.audioDeviceCommon_.SelectRingerOrAlarmDevices(
        descs4, rendererChangeInfo4);
    EXPECT_EQ(false, result);

    rendererChangeInfo1.reset();
    rendererChangeInfo2.reset();
    rendererChangeInfo3.reset();
    rendererChangeInfo4.reset();
    audioDeviceDescriptor2.reset();
    audioDeviceDescriptor3.reset();
    audioDeviceDescriptor4.reset();
}

/**
* @tc.name  : Test SetPreferredDevice.
* @tc.number: SetPreferredDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceFourthUnitTest, SetPreferredDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest SetPreferredDevice_001 start");
    ASSERT_NE(nullptr, AudioPolicyServiceUnitTest::GetServerPtr());

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr1 = nullptr;
    int32_t result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_MEDIA_RENDER, audioDeviceDescriptorSptr1);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr2 = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr2->deviceType_ = DEVICE_TYPE_NONE;
    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_CALL_RENDER, audioDeviceDescriptorSptr2);
    EXPECT_EQ(SUCCESS, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_CALL_CAPTURE, audioDeviceDescriptorSptr2);
    EXPECT_EQ(SUCCESS, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_RECORD_CAPTURE, audioDeviceDescriptorSptr2);
    EXPECT_EQ(SUCCESS, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_RING_RENDER, audioDeviceDescriptorSptr2);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        AUDIO_TONE_RENDER, audioDeviceDescriptorSptr2);
    EXPECT_EQ(ERR_INVALID_PARAM, result);

    uint32_t preferredType = 6;
    PreferredType ERR_PFTYPE = static_cast<PreferredType>(preferredType);
    result = AudioPolicyUtils::GetInstance().SetPreferredDevice(
        ERR_PFTYPE, audioDeviceDescriptorSptr2);
    EXPECT_EQ(ERR_INVALID_PARAM, result);
}
} // namespace AudioStandard
} // namespace OHOS
